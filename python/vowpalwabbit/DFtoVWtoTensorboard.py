import tensorboardX as tx


class VWtoTensorboard:
	"""The object of this class would be passed as a callback to DFtoVWtoTensorboard fit method to ensure writing of logs for Tensorboard"""

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
		self.file_writer = tx.SummaryWriter(logdir)   # creating file writer
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



class DFtoVWtoTensorboard:
	"""This class provides capability for learning possible on DFtoVW object, provides capability to 
	print metrics (average_loss, since_last, label, prediction, num_features) and also provides support 
	to log these metrics for Tensorboard Visualization"""

	def __init__(self, df_to_vw, vw):
		"""Construct a DFtoVWtoTensorboard object

		Parameters
		----------

		df_to_vw : A DFtoVW object
				This would be used to construct examples 
		vw		 : A vowpalwabbit object
 				This would be used for learning using the algorithm specified in the object, and also to compute a few metrics

		Returns
		-------

		self : DFtoVWtoTensorboard
		"""
		self.vw = vw  # As this needs to be remembered
		self.vw_formatted_data = df_to_vw.convert_df()


	#------------------------------------------------------------------------------------------------		
	def fit(self, vw_to_tensorboard=None):
		"""Learns on the relevant examples and can also log metrics for tensorboard visualization

		Parameters
		----------

		vw_to_tensorboard  : VWtoTensorboard object 
					Default value is None, this parameter is used to control the logging of metrics for Tensorboard visualization
					If value is VWtoTensorboard object : metrics are computed and logged for Tensorboard visualization
					If value is None : metrics are not computed and not logged for Tensorboard visualization

		Returns
		-------

		None
		"""
		for iteration, vw_format in enumerate(self.vw_formatted_data):
			self.vw.learn(vw_format, vw_to_tensorboard)         # learn on example

