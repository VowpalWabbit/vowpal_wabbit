from vowpalwabbit import pyvw
from datetime import datetime
import tensorboardX as tx


class DFtoVWtoTensorboard:
	"""This class provides capability for learning possible on DFtoVW object, provides capability to 
	print metrics (average_loss, since_last, label, prediction, num_features) and also provides support 
	to log these metrics for Tensorboard Visualization"""

	def __init__(self, df_to_vw, vw):


		self.df_to_vw = df_to_vw
		self.vw = vw
		self.vw_formatted_data = df_to_vw.convert_df()


	#------------------------------------------------------------------------------------------------		
	def _calculate_average_loss(self, sum_loss, weighted_examples):
	
	    try:
	        return sum_loss / weighted_examples
	    
	    except ZeroDivisionError:
	        return 0.


	#------------------------------------------------------------------------------------------------		
	def _calculate_since_last(self, sum_loss_since_last, weighted_examples_since_last):
			
	    try:
	        return sum_loss_since_last / weighted_examples_since_last
	    
	    except ZeroDivisionError:
	        return 0.	    


	#------------------------------------------------------------------------------------------------		
	def _print_metrics(self, average_loss, since_last, label, prediction, num_features):

		print( 'average_loss:{:.6f}'.format(average_loss) , end='\t')
		print('since_last:{:.6f}'.format(since_last), end='\t')
		print('label:', label, end='\t')    
		print('prediction:', prediction, end='\t')    
		print('num_features:', num_features) 


	#------------------------------------------------------------------------------------------------		
	def get_label(self, example, label_type):

		switch_label_type = {
	        pyvw.vw.lDefault: None,
	        pyvw.vw.lBinary: example.get_simplelabel_label,
	        pyvw.vw.lMulticlass: example.get_multiclass_label,
	        pyvw.vw.lCostSensitive: example.get_costsensitive_class,
	        pyvw.vw.lContextualBandit: example.get_cbandits_class,
	#         pyvw.vw.lConditionalContextualBandit: None,
	#         pyvw.vw.lSlates: None,
	#         pyvw.vw.lContinuous: None
	    }
		return switch_label_type[label_type]()	


	#------------------------------------------------------------------------------------------------		
	def fit(self, df, tensorboard=True):

		if tensorboard:
			logdir = "logs/scalars/" + datetime.now().strftime("%Y%m%d-%H%M%S")  # logs directory
			file_writer = tx.SummaryWriter(logdir + "/iris")   # creating file writer
		
		sum_loss = 0.
		weighted_examples = 0.

		for ind, iteration in zip(df.index, range(len(df))):
		    vw_format = self.vw_formatted_data[ind]   # get the string format of specific example
		    example = self.vw.parse(vw_format)       # parse the string format, it returns an example object
		    self.vw.learn(example)                  # learn on example

		    label = self.get_label(example, self.vw.get_label_type())    
		    prediction = pyvw.get_prediction(example, self.vw.get_prediction_type())
		    num_features = example.get_feature_number()    

		    self.vw.finish_example(example)  # Any use of vw object should be done after this and use of example before this 

		    sum_loss_since_last = self.vw.get_sum_loss() - sum_loss  # vw.get_sum_loss() return current sum loss, sum_loss variable right now holds sum loss of previous iteration
		    weighted_examples_since_last = self.vw.get_weighted_examples() - weighted_examples  # vw.get_weighted_examples() return current weighted examples(sum),  weighted_examples variable right now holds weighted examples of previous iteration		        
		    
		    sum_loss= self.vw.get_sum_loss()  # Now sum_loss no longer hold previous iteration's sum_loss
		    weighted_examples = self.vw.get_weighted_examples()  # Now weighted_examples no longer hold previous iteration's weighted examples
		    
		    average_loss = self._calculate_average_loss(sum_loss, weighted_examples)
		    since_last = self._calculate_since_last(sum_loss_since_last, weighted_examples_since_last)
		     
		    self._print_metrics(average_loss, since_last, label, prediction, num_features)

		    if tensorboard:
			    file_writer.add_scalar('average_loss', average_loss, iteration)  # logging average_loss on each iteration
			    file_writer.add_scalar('since_last', since_last, iteration)   # logging since_last on each iteration
			#     file_writer.add_scalar('label' , label, iteration)
			#     file_writer.add_scalar('prediction', prediction, iteration)
			#     file_writer.add_histogram('label-prediction', [label, prediction], iteration)
			#     file_writer.add_histogram('label', label, iteration)
			#     file_writer.add_histogram('prediction', prediction, iteration)
			    file_writer.add_scalar('num_features', num_features, iteration)