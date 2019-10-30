# Running VW on a cluster

The implementation of Non-linear (Preconditioned) Conjugate Gradient,
LBFGS, online learning, and adaptive online learning works on clusters
(both Hadoop and otherwise) now.

To build the code, run compile the `spanning_tree` target.

At a high level, the code operates by repeatedly executing something
equivalent to the MPI AllReduce function---adding up floats from all
nodes then broadcasting them back to each individual node. In order
to do this, a spanning tree over the nodes must be created. This is
done using the helper daemon 'allreduce_master'.

***********************************************************************

To run the code on non-Hadoop clusters, the script [single_machine](https://github.com/VowpalWabbit/vowpal_wabbit/blob/master/cluster/single_machine)
has the simplest possible invocation.

In general: start the span server on one of the cluster nodes:
```sh
./spanning_tree
```

Or to start in the foreground:
```sh
./spanning_tree --nondaemon
```

Launch `vw` on each of the worker nodes:

```sh
./vw --span_server <location> --total <t> --node <n> --unique_id <u> -d <file>
```

Where:
- `<location>` is the host running spanning_tree
- `<t>` is the total number of nodes
- `<n>` is the node id number
- `<u>` is a number shared by all nodes in the process
- `<file>` is the input source file for that node

---

To run the code on Hadoop clusters:

Decide if you are going to control the number of tasks by:
- (a) using gzip compressed files which cannot be broken up by Hadoop
- (b) controlling the number of reducers.

We'll assume (a) below.

Start the span server for the Hadoop cluster:

```sh
./spanning_tree
```

Start the map-reduce job using Hadoop streaming:

```
hadoop jar $HADOOP_HOME/hadoop-streaming.jar \
  -files vw,runvw.sh \
  -Dmapred.job.map.memory.mb=2500 -input <input> -output <output> \
  -mapper runvw.sh -reducer NONE
```

where `<output>` is the directory on HDFS where you want the trained
model to be saved. The trained model is saved to the file
`<output>/model` on HDFS and can be retreived by `hadoop -get`.

To modify the arguments to VW, edit the script `runvw.sh`. Arguments to
hadoop can be directly added in the hadoop streaming command.

See the `mapscript.sh` which uses `runvw.sh` for an advanced example
of running VW in a Hadoop enviornmnent.

---

The files you need to know about:

`runvw.sh`: This is the mapper code.

It takes as arguments:
- The output directory. The trained model from the first mapper is stored as the file "model" in the output directory.
- The hostname of the cluster gateway, so that the mappers can connect to the gateway
- All the other standard VW options are currently hardcoded in the script, feel free to mess around with them.

---

`spanning_tree.cc`: This is the span server code which runs on the
gateway. You start it before the call to hadoop.  The span server
backgrounds itself after starting and listens for incoming
connections. It sets up the topology on the mappers and then let them
communicate amongst themselves.

---

`allreduce.h`: This is the header file for the nodes.

---

`allreduce.cc`: This is the code for doing allreduce. It implement the
routine described above. all_reduce is implemented as a combination of
reduce and broadcast routines. reduce reads data from children, adds it
with local data and passes it up to the parent with a call to pass_up.
broadcast receives data from parent, and passes it down to children with
a call to pass_down.

---

`cg.cc, gd.cc, bfgs.cc`: learning algorithms which use all_reduce
whenever communication is needed. Uses routines accumulate and
accumulate_scalar to reduce vectors and scalars resp.
