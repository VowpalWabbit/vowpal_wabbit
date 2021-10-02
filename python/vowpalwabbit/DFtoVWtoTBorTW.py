import tensorboardX as tx
import tensorwatch as tw
from tensorboardX.proto.graph_pb2 import GraphDef
from tensorboardX.proto.node_def_pb2 import NodeDef
from tensorboardX.proto import event_pb2


class VWtoTensorboard:
	"""The object of this class can be passed as a callback or used to ensure writing of logs for Tensorboard"""

	def __init__(self, logdir):
		"""Construct a VWtoTensorboard object

		Parameters
		----------

		logdir : str
			A string specifying the log directory for Tensorboard logs

		Returns
		-------

		self : VWtoTensorboard
		"""
		self.file_writer = tx.SummaryWriter(logdir, flush_secs=30)   # creating file writer
		self.iteration = 0   # This would keep value of current iteration 


	def emit_learning_metrics(self, average_loss, since_last):
		"""This method for now logs the metrics given as arguments for Tensorboard visualization

		Parameters
		----------

		average_loss : float
				This is the average_loss value to be logged for Tensorboard
		since_last   : float
				This is the since_last value to be logged for Tensorboard

		Returns
		-------

		None
		"""
		self.file_writer.add_scalar('average_loss', average_loss, self.iteration)  # logging average_loss on each iteration
		self.file_writer.add_scalar('since_last', since_last, self.iteration)   # logging since_last on each iteration
		self.iteration += 1  # Now increment this as the incremented value is for next iteration


	def draw_reductions_graph(self, vw):
		"""This method draws vw model's reductions as graph for Tensorboard visualization

		Parameters
		----------

		vw    : A vowpalwabbit object
				This would be used to get the enabled reductions for making a graph

		Returns
		-------

		None
		"""
		nodes = []
		reductions = vw.get_enabled_reductions()

		for ind, layer in enumerate(reductions):
			if ind == 0:
				nodes.append(NodeDef(name=layer, op='op'))   # First node does not have any input edge
			else:
				nodes.append(NodeDef(name=layer, op='op', input=[reductions[ind-1]]))

		g = GraphDef(node=nodes)
		event = event_pb2.Event(graph_def=g.SerializeToString())
		self.file_writer._get_file_writer().add_event(event)


	def show_args_as_text(self, vw):
		"""This method adds vw arguments when model is initialized as text in tensorboard

		Parameters
		----------

		vw    : A vowpalwabbit object
				This would be used to get the arguments

		Returns
		-------

		None
		"""
		self.file_writer.add_text("command line arguments", vw.get_arguments())



class VWtoTensorwatchStreamer:
	"""The object of this class can be passed as a callback or used to visualize in Tensorwatch"""

	def __init__(self, logfile, port=4500):
		"""Construct a VWtoTensorwatchStreamer object

		Parameters
		----------

		logfile : str
			This is the logfile where tensorwatch will log the metrics to, so if no client is listening to the socket then client can pick values from the log file, file should have '.log' extension
		port : int
			Port on which Watcher of Tensorwatch should stream
			Default value is 4500

		Returns
		-------

		self : VWtoTensorwatchStreamer
		"""
		self.watcher = tw.Watcher(logfile, port=port)
		self.iteration = 0
		self.avg_loss_tw = self.watcher.create_stream('average_loss')
		self.since_last_tw = self.watcher.create_stream('since_last')
		self.label_tw = self.watcher.create_stream('label')
		self.prediction_tw = self.watcher.create_stream('prediction')


	def emit_learning_metrics(self, average_loss, since_last, label, prediction):
		"""This method for now logs the metrics given as arguments for Tensorwatch visualization

		Parameters
		----------

		average_loss : float
				This is the average_loss value to be logged for Tensorwatch
		since_last   : float
				This is the since_last value to be logged for Tensorwatch
		label        : int
				This is the label value to be logged for Tensorwatch
		prediction   : int
				Thhis is the prediction value to be logged for Tensorwatch

		Returns
		-------

		None
		"""
		self.avg_loss_tw.write( (self.iteration, average_loss) )
		self.since_last_tw.write( (self.iteration, since_last) )
		self.label_tw.write( (self.iteration, label) )
		self.prediction_tw.write( (self.iteration, prediction) )
		self.iteration += 1


	def __del__(self):
		"""When the object is being destroyed, this would be called to close the watcher, this helps in freeing port

		Parameters
		----------

		None

		Returns
		-------

		None
		"""
		self.watcher.close()



class VWtoTensorwatchClient:
	"""The object of this class can be used to visualize results in Tensorwatch"""

	def __init__(self, logfile, port=4500):
		"""Construct a VWtoTensorwatchClient object

		Parameters
		----------

		logfile : str
			This is the logfile where tensorwatch will log the metrics to, so if no client is listening to the socket then client can pick values from the log file, file should have '.log' extension
		port : int
			Port on which WatcherClient should work
			Default value is 4500
			
		Returns
		-------

		self : VWtoTensorwatchClient
		"""
		self.client = tw.WatcherClient(logfile, port=port)


	def plot_metrics(self):
		"""This displays learning metrics plots in Tensorwatch

		Parameters
		----------

		None

		Returns
		-------

		None
		"""
		avg_loss_tw = self.client.open_stream('average_loss')
		since_last_tw = self.client.open_stream('since_last')
		label_tw = self.client.open_stream('label')
		prediction_tw = self.client.open_stream('prediction')

		avg_plot = tw.Visualizer(avg_loss_tw, vis_type='line', xtitle='iterations', ytitle='average_loss')
		avg_plot.show()

		since_last_plot = tw.Visualizer(since_last_tw, vis_type='line', xtitle='iterations', ytitle='since_last')
		since_last_plot.show()

		label_plot = tw.Visualizer(label_tw, vis_type='bar', xtitle='label_per_iteration', color='red')  # color='red' becuase default color='None' which gives error
		label_plot.show()

		prediction_plot = tw.Visualizer(prediction_tw, vis_type='bar', xtitle='prediction_per_iteration', color='red')   # color='red' becuase default color='None' which gives error
		prediction_plot.show()

	def __del__(self):
		"""When the object is being destroyed, this would be called to close the WatcherClient

		Parameters
		----------

		None

		Returns
		-------

		None
		"""
		self.client.close()



class DFtoVWtoTBorTW:
	"""This class provides capability for learning possible on DFtoVW object and also provides support 
	to log these metrics for Tensorboard and Tensorwatch Visualization"""

	def __init__(self, vw_formatted_data, vw):
		"""Construct a DFtoVWtoTBorTW object

		Parameters
		----------

		vw_formatted_data : A DFtoVW object
				This would be the list of examples provided either by DFtoVW.convert_df or a user making custom examples
		vw		 : A vowpalwabbit object
 				This would be used for learning using the algorithm specified in the object, and also to compute a few metrics

		Returns
		-------

		self : DFtoVWtoTBorTW
		"""
		self.vw = vw  # As this needs to be remembered
		self.vw_formatted_data = vw_formatted_data


	def fit(self, vw_to_tensorboard=None, vw_to_tensorwatch=None):
		"""Learns on the relevant examples and can also log metrics for tensorboard visualization

		Parameters
		----------

		vw_to_tensorboard  : VWtoTensorboard object 
					Default value is None, this parameter is used to control the logging of metrics for Tensorboard visualization
					If value is VWtoTensorboard object : metrics are computed and logged for Tensorboard visualization
					If value is None : metrics are not computed and not logged for Tensorboard visualization

		vw_to_tensorwatch  : VWtoTensorwatch object 
					Default value is None, this parameter is used to control the streaming of metrics for Tensorwatch
					If value is VWtoTensorwatchStreamer object : metrics are computed for Tensorwatch visualization
					If value is None : metrics are not computed and not visualized using Tensorwatch

		Returns
		-------

		None
		"""
		for iteration, vw_format in enumerate(self.vw_formatted_data):
			self.vw.learn(vw_format, vw_to_tensorboard, vw_to_tensorwatch)         # learn on example

