import java.net.InetAddress
import org.apache.spark.Logging

/**
 * The goal of this class is to provide as easy method to pipe data through an external command.  It is done by combining
 * a {@link PipedOutputStream} with a {@link PipedInputStream} to create a single pipe to feed data through.  This is
 * done asynchronously so data can be read and written to at the same time.
 * Created by jmorra on 1/22/15.
 */
class PipeUtils(bufferSize: Int = 1 << 20) {
  import java.io._

  import scala.concurrent.ExecutionContext.Implicits.global
  import scala.concurrent.Future
  import scala.language.postfixOps
  import scala.sys.process._

  /**
   * This implicit class will allow easy access to streaming through external processes.  This
   * should work on a line by line basis just like Spark's pipe command.
   * http://stackoverflow.com/questions/28095469/stream-input-to-external-process-in-scala
   * @param s: The input stream
   */
  implicit class IteratorStream(s: TraversableOnce[String]) {
    def pipe(cmd: String): Stream[String] = cmd #< iter2is(s) lines
    def pipe(cmd: Seq[String]): Stream[String] = cmd #< iter2is(s) lines
    def run(cmd: String): String = cmd #< iter2is(s) !!

    private[this] def iter2is[A](it: TraversableOnce[A]): InputStream = {
      // What is written to the output stream will appear in the input stream.
      val pos = new PipedOutputStream

      val pis = new PipedInputStream(pos, bufferSize)
      val w = new PrintWriter(new BufferedOutputStream(pos, bufferSize), false)

      // Scala 2.11 (scala 2.10, use 'future').  Executes asynchronously.
      // Fill the stream, then close.
      Future {
        try it.foreach(w.println)
        finally w.close
      }

      // Return possibly before pis is fully written to.
      pis
    }
  }
}

/**
 * A framework for running VW in a cluster environment using <a href="http://spark.apache.org/">Apache Spark</a>.  This
 * is meant only as a framework and may require some modification to work under your specific case.
 * Created by jmorra on 8/19/15.
 */
case class VwSparkCluster(
  pipeUtils: PipeUtils = new PipeUtils,
  ipAddress: String = InetAddress.getLocalHost.getHostAddress,
  defaultParallelism: Int = 2) extends Logging {

  import java.io._
  import org.apache.commons.io.IOUtils
  import org.apache.spark.rdd.RDD
  import org.apache.spark.SparkContext
  import scala.sys.process._
  import pipeUtils._

  /**
   * This will learn a VW model in cluster mode.  If you notice that this command never starts and just stalls then the parallelism
   * is probably too high.  Refer to <a href="https://github.com/JohnLangford/vowpal_wabbit/wiki/Cluster_parallel.pdf">this</a>
   * for more information.
   * @param data an RDD of Strings that are in VW input format.
   * @param vwCmd the VW command to run.  Note that this command must NOT contain --cache_file and -f.  Those will automatically
   *              be appended if necessary.
   * @param parallelism the amount of parallelism to use.  This is calculated using a formula defined in getParallelism
   *                    if it is not supplied.  It is recommended to only supply this if getParallelism is not working
   *                    in you case.
   * @return a byte array containing the final VW model.
   */
  def train(data: RDD[String], vwCmd: String, parallelism: Option[Int] = None): Array[Byte] = {
    if (numberOfRunningProcesses("spanning_tree") != 1) {
      throw new IllegalStateException("spanning_tree is not running on the driver, cannot proceed.  Please start spanning_tree and try again.")
    }

    val sc = data.context
    val conf = sc.getConf

    // By using the job id and the RDD id we should get a globally unique ID.
    val jobId = (conf.get("spark.app.id").replaceAll("[^\\d]", "") + data.id).toLong
    logInfo(s"VW cluster job ID: $jobId")

    val partitions = parallelism.getOrElse(getParallelism(sc).getOrElse(defaultParallelism))
    logInfo(s"VW cluster parallelism: ${partitions}")

    val repartitionedData = if (data.partitions.size == partitions) data else data.repartition(partitions)

    val vwBaseCmd = s"$vwCmd --total $partitions --span_server $ipAddress --unique_id $jobId"
    logInfo(s"VW cluster baseCmd: $vwBaseCmd")

    val vwModels = repartitionedData.mapPartitionsWithIndex{case (partition, x) =>
      Iterator(runVWOnPartition(vwBaseCmd, x, partition))
    }

    vwModels.collect.flatten.flatten
  }

  def numberOfRunningProcesses(process: String): Int = "ps aux".#|(s"grep $process").!!.split("\n").size - 1

  /**
   * Gets the executor storage status excluding the driver node.
   * @param sc the SparkContext
   * @return an Array of Strings that are the names of all the storage statuses.
   */
  def executors(sc: SparkContext): Array[String] = {
    sc.getExecutorStorageStatus.collect{
      case x if x.blockManagerId.executorId != "<driver>" =>
        x.blockManagerId.executorId
    }
  }

  /**
   * Gets the parallelism of the cluster.  This is very much so a work in progress that seems to work now.  This took
   * a lot of experimentation on Spark 1.2.0 to get to work.  I make no guarantees that it will work on other Spark versions
   * especially if <a href="https://spark.apache.org/docs/1.2.0/job-scheduling.html#dynamic-resource-allocation">dynamic
   * allocation</a> is enabled.  I also only tested this with a master of yarn-client and local so I'm not sure how
   * well it'll behave in other resource management environments (Spark Standalone, Mesos, etc.).
   * @param sc the SparkContext
   * @return if the parallelism can be found then the expected amount of parallelism.
   */
  def getParallelism(sc: SparkContext): Option[Int] = {
    sc.master match {
      case x if (x.contains("yarn")) => sc.getConf.getOption("spark.executor.cores").map(x => x.toInt * executors(sc).size)
      case _ => Some(sc.defaultParallelism)
    }
  }

  /**
   * This will accept a base VW command, and append a cache file if necessary.  It will also create a temp file
   * to store the VW model.  It will then run VW on the supplied data.  Finally it will return the bytes of the
   * model ONLY if the partition is 0.
   *
   * This function was tricky to write because the end result of each calculation is a file on the local disk.
   * According to John all the models should be in the same state after learning so we can choose to save
   * anyone we want, therefore, transferring the contents of each file to the driver would be wasteful.
   * In order to avoid this unnecessary transfer we're just going to get the first file.  Now you might
   * ask yourself why not just call .first on the RDD.  We cannot do that because in that case Spark would
   * only evaluate the first mapper and we need all of them to be evaluated, hence the need for .collect to
   * be called.  Note that you may have to increase spark.driver.maxResultSize if the size of the VW model
   * is too large.
   * @param vwBaseCmd the base VW command without a cache file or an output specified.  A cache file will automatically
   *                  be used if --passes is specified.
   * @param data a String a data in VW format to be passed to VW
   * @param partition the partition number of this chunk of data
   * @return an Array of the bytes of the VW model ONLY if this is the 0th partition, else None.
   */
  def runVWOnPartition(vwBaseCmd: String, data: Iterator[String], partition: Int): Option[Array[Byte]] = {
    val cacheFile = if (vwBaseCmd.contains("--passes ")) {
      val c = File.createTempFile("vw-cache", ".cache")
      c.deleteOnExit
      Option(c)
    } else None
    val vwBaseCmdWithCache = cacheFile.map(x => s"$vwBaseCmd -k --cache_file ${x.getCanonicalPath}").getOrElse(vwBaseCmd)

    val output = File.createTempFile("vw-model", ".model")
    output.deleteOnExit
    val vwCmd = s"$vwBaseCmdWithCache --node $partition -f ${output.getCanonicalPath}"
    data.pipe(vwCmd)
    cacheFile.foreach(_.delete)

    val vwModel = if (partition == 0) {
      val inputStream = new BufferedInputStream(new FileInputStream(output))
      val byteArray = IOUtils.toByteArray(inputStream)
      inputStream.close
      Option(byteArray)
    }
    else None

    output.delete()
    vwModel
  }
}
