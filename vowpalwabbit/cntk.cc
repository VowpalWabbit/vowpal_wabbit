/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/
//#include <limits>
#include <CNTKLibrary.h>

#include "cntk.h"
#include "reductions.h"
#include "vw.h"
#include "crossplat_compat.h"

#include <iostream>

namespace VW_CNTK {
	using namespace VW;
	using namespace LEARNER;
	using namespace CNTK;
	using namespace std;

	struct cntk2
	{
		vector<Variable> inputs;
		Variable* namespace_to_inputs[256];
		Variable prediction;
		Variable label;
		TrainerPtr trainer;
		string input_filename;
		string output_filename;

		bool last_prediction_valid = false;
		float last_prediction;

		unordered_map<Variable, ValuePtr> outputs;
		ValuePtr labelValue;
		DeviceDescriptor device;

		cntk2() 
			 : device(DeviceDescriptor::CPUDevice())
			// : device(DeviceDescriptor::GPUDevice(0))
			// : device(DeviceDescriptor::BestDevice())
		{ 
			for (auto& d : DeviceDescriptor::AllDevices())
			{
				cout << "Id: " << d.Id() << "  " << (d.Type() == DeviceKind::GPU ? "GPU" : "CPU") << endl;
			}
				
		}
	};

	struct cntk
	{
		vw* all;
		v_array<int> row_indicies[256];
		// v_array<int> col_indicies[256];
		int col_starts[256][2];
		cntk2* d;
		uint64_t length;
		uint64_t weight_mask;

		float output;
		float label;
	};

	void predict(cntk& dat, base_learner&, example& ex)
	{
		std::cerr << "predict" << endl;
	}

	void learn(cntk& dat, base_learner&, example& ex)
	{
		// TODO: create quadratics and map to -q ab Input(a1_b2), given a...a1, b...b2
		// If input has dynamic axis()  -> 1 feature per row to support "This is a boat. This is a fish" (note that "This" occurs twice)
		// If input !has dynamic aixs() -> all features into a single row
		unordered_map<Variable, ValuePtr> arguments;
		NDShape inputShape({ (size_t)(dat.length) });

		for (size_t j = 0; j < 10; ++j)
		for (auto ns_iter = ex.begin();ns_iter != ex.end();++ns_iter)
		{
			auto size = (*ns_iter).indicies.size();
			auto ns = ns_iter.index();

			Variable* input = dat.d->namespace_to_inputs[ns_iter.index()];
			if (input == nullptr)
			{
				// TODO: output once
				// dat.all->trace_message << "Input for namespace '" << (char)ns_iter.index() << "' (" << ns_iter.index() << ") not found" << endl;
				continue;
			}

			// default namespace... 

			// get column index cache
			v_array<int>& row_indicies = dat.row_indicies[ns];

			// size correctly
			// TODO: row_indicies.erase(); shrink again?
			if ((size_t)(row_indicies.end_array - row_indicies._begin) < size)
				row_indicies.resize(size);
			else
				row_indicies._end = row_indicies._begin + size;

			// TODO: needs sorting
			// This semantically transposes from column sparse to row sparse
			auto rp = row_indicies._begin;
			for (auto& f : (*ns_iter).indicies)
				*(rp++) = (int)(f & dat.weight_mask);

			// in single row mode all rowIndices are 0
			// {0, size}
			dat.col_starts[ns][1] = size;

			// colStarts: 0, size (= # num features for VW examples)
			// rowIndicies: VW 
			// nonZeroValues

			auto nonZeroValues = (*ns_iter).values.begin();
			// CNTK_API NDArrayView(const NDShape& viewShape, const SparseIndexType* colStarts, const SparseIndexType* rowIndices, const ElementType* nonZeroValues, size_t numNonZeroValues, const DeviceDescriptor& device, bool readOnly = false);
			ValuePtr inputValue = MakeSharedObject<Value>(MakeSharedObject<NDArrayView>(
				inputShape, 
				dat.col_starts[ns], // colStarts
				row_indicies.begin(), // rowIndicies
				nonZeroValues, // nonZeroValues,
				size,
				dat.d->device, 
				true)); // readOnly, TODO: not sure if we care if things are modified except for data.row_indicies (do I still own the array?)

			arguments.insert(make_pair(*input, inputValue));
		}

		//NDShape inputShape = { inputDim, 1, numSamples };
		//ValuePtr inputValue = MakeSharedObject<Value>(MakeSharedObject<NDArrayView>(inputShape, inputData.data(), inputData.size(), DeviceDescriptor::CPUDevice(), true));

		//std::vector<float> labelData(numOutputClasses * numSamples, 0);
		//for (size_t i = 0; i < numSamples; ++i)
		//{
		//	labelData[(i*numOutputClasses) + (rand() % numOutputClasses)] = 1;
		//}

		//NDShape labelShape = { numOutputClasses, 1, numSamples };
		//ValuePtr labelValue = MakeSharedObject<Value>(MakeSharedObject<NDArrayView>(labelShape, labelData.data(), labelData.size(), DeviceDescriptor::CPUDevice(), true));

		//ValuePtr outputValue, predictionErrorValue;
		//std::unordered_map<Variable, ValuePtr> outputs = { { classifierOutputFunction->Output(), outputValue },{ predictionFunction->Output(), predictionErrorValue } };
		//ffNet->Forward({ { inputVar, inputValue },{ labelsVar, labelValue } }, outputs, computeDevice);

		dat.label = ex.l.simple.label;
		arguments.insert(make_pair(dat.d->label, dat.d->labelValue));

		dat.output = 0;
		dat.d->trainer->TrainMinibatch(arguments, dat.d->outputs, dat.d->device);
		// keep around for next time
		// dat.d->last_prediction = dat.output;
		ex.pred.scalar = dat.output;
	}

	void update(cntk& dat, base_learner&, example& ex)
	{
		std::cerr << "update" << endl;

		// TODO: what should this do?
	}

	float sensitivity(cntk& dat, base_learner&, example& ex)
	{
		std::cerr << "sensitivity" << endl;

		// TODO: what should this do?
		return 0.;
	}

	void save_load(cntk& dat, io_buf& model_file, bool read, bool text)
	{
		vw& all = *dat.all;

		// TODO: DeviceDescriptor::GPUDevice(0));

		if (read)
		{
			// TODO: check if input_filename is set.
			wstring wfilename;
			std::copy(dat.d->input_filename.begin(), dat.d->input_filename.end(), std::back_inserter(wfilename));

			// load model
			auto model = Function::LoadModel(wfilename, dat.d->device);

			// get inputs
			auto inputs = model->Inputs();
			copy_if(inputs.begin(), inputs.end(), back_inserter(dat.d->inputs), [](auto& var) { return var.IsInput(); });

			// establish mapping from namespaces to inputs
			memset(dat.d->namespace_to_inputs, 0, sizeof(dat.d->namespace_to_inputs));
			for (auto& var : dat.d->inputs)
			{
				wchar_t ns = var.Name()[0];
				assert(ns >= 0 && ns < 256);
				dat.d->namespace_to_inputs[static_cast<char>(ns)] = &var;
			}

			// get output
			auto outputs = model->Outputs();
			if (outputs.size() == 0)
				THROW("Need at least 1 output");

			cout << "num outputs: " << outputs.size() << endl;
			dat.d->prediction = outputs.front();

			NDShape labelShape = { 1, 1, 1 };

			ValuePtr outputValue = MakeSharedObject<Value>(MakeSharedObject<NDArrayView>(labelShape, &dat.output, 1, dat.d->device, false /* readOnly */));
			dat.d->outputs = { { dat.d->prediction, outputValue } };

			// TODO: is numOutputClasses == outputs.size()
			// isSparse?
			dat.d->label = InputVariable({ outputs.size() }, DataType::Float, L"Label");
			dat.d->labelValue = MakeSharedObject<Value>(MakeSharedObject<NDArrayView>(labelShape, &dat.label, 1, dat.d->device, true /* readOnly */));

			// TODO: support different losses
			auto trainingLoss = SquaredError(dat.d->prediction, dat.d->label, L"squaredError");

			// TODO: -1,1 vs 0,1
			// logistic -> CNTK::BinaryCrossEntropy()

			// multi-class  ->  CrossEntropyWithSoftmax
			// ranking  ->  CNTK::LambdaRank
			// cosine distance -> CNTK::CosineDistance

			//auto trainingLoss = CNTK::CrossEntropyWithSoftmax(classifierOutput, labels, L"lossFunction");
			//auto prediction = CNTK::ClassificationError(classifierOutput, labels, L"classificationError");

			// TODO: support more learners
			AdditionalLearningOptions opts;
			opts.l1RegularizationWeight = dat.all->l1_lambda;
			opts.l2RegularizationWeight = dat.all->l2_lambda;

			dat.d->trainer = CreateTrainer(model, trainingLoss, dat.d->prediction,
			{ SGDLearner(model->Parameters(), LearningRateSchedule(dat.all->eta, LearningRateSchedule::UnitType::Minibatch), opts) });
		}
		else
		{
			wstring wfilename;
			std::copy(dat.d->output_filename.begin(), dat.d->output_filename.end(), std::back_inserter(wfilename));

			// load model
			dat.d->trainer->Model()->SaveModel(wfilename);
		}
	}

	void finish(cntk& o)
	{
		//o.row_indicies.delete_v();
		for (int i=0;i<256;i++)
			o.row_indicies[i].delete_v();
		delete o.d;
	}

	base_learner* setup(vw& all)
	{
		if (missing_option(all, true, "cntk", "Use CNTK as base learner."))
			return nullptr;

		cntk& g = calloc_or_throw<cntk>();
		g.all = &all;
		g.d = new cntk2();		

		SetMaxNumCPUThreads(1);

		g.length = UINT64_ONE << all.num_bits;
		g.weight_mask = g.length - 1; // TODO: is this right?

		new_options(all, "CNTK options")
			// TODO make it optional and load from embedded model
			("input", po::value<string>(&g.d->input_filename), "Input model filename.")
			("output", po::value<string>(&g.d->output_filename), "Output model filename.");

		//("sparse_l2", po::value<float>()->default_value(0.f), "use per feature normalized updates");
		add_options(all);
		po::variables_map& vm = all.vm;

		learner<cntk>& ret = init_learner(&g, learn, 1 /* TODO: can I do 0*/);
		ret.set_predict(predict);
		ret.set_sensitivity(sensitivity);
		ret.set_update(update);
		ret.set_save_load(save_load);

		//ret.set_multipredict(multipredict);
		//ret.set_end_pass(end_pass);
		ret.set_finish(finish);

		return make_base(ret);
	}
}