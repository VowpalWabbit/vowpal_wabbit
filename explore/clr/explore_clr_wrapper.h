#pragma once
#include "explore_interop.h"

namespace MultiWorldTesting {

	generic <class Ctx>
	public ref class EpsilonGreedyExplorer : public IExplorer<Ctx>, public PolicyCallback<Ctx>
	{
	public:
		EpsilonGreedyExplorer(IPolicy<Ctx>^ defaultPolicy, float epsilon, UInt32 numActions)
		{
			this->defaultPolicy = defaultPolicy;
			m_explorer = new NativeMultiWorldTesting::EpsilonGreedyExplorer<NativeContext>(*GetNativePolicy(), epsilon, (u32)numActions);
		}

		~EpsilonGreedyExplorer()
		{
			delete m_explorer;
		}

	internal:
		virtual UInt32 InvokePolicyCallback(Ctx context, int index) override
		{
			return defaultPolicy->ChooseAction(context);
		}

		NativeMultiWorldTesting::EpsilonGreedyExplorer<NativeContext>* Get()
		{
			return m_explorer;
		}

	private:
		IPolicy<Ctx>^ defaultPolicy;
		NativeMultiWorldTesting::EpsilonGreedyExplorer<NativeContext>* m_explorer;
	};

	generic <class Ctx>
	public ref class TauFirstExplorer : public IExplorer<Ctx>, public PolicyCallback<Ctx>
	{
	public:
		TauFirstExplorer(IPolicy<Ctx>^ defaultPolicy, UInt32 tau, UInt32 numActions)
		{
			this->defaultPolicy = defaultPolicy;
			m_explorer = new NativeMultiWorldTesting::TauFirstExplorer<NativeContext>(*GetNativePolicy(), tau, (u32)numActions);
		}

		~TauFirstExplorer()
		{
			delete m_explorer;
		}

	internal:
		virtual UInt32 InvokePolicyCallback(Ctx context, int index) override
		{
			return defaultPolicy->ChooseAction(context);
		}

		NativeMultiWorldTesting::TauFirstExplorer<NativeContext>* Get()
		{
			return m_explorer;
		}

	private:
		IPolicy<Ctx>^ defaultPolicy;
		NativeMultiWorldTesting::TauFirstExplorer<NativeContext>* m_explorer;
	};

	generic <class Ctx>
	public ref class SoftmaxExplorer : public IExplorer<Ctx>, public ScorerCallback<Ctx>
	{
	public:
		SoftmaxExplorer(IScorer<Ctx>^ defaultScorer, float lambda, UInt32 numActions)
		{
			this->defaultScorer = defaultScorer;
			m_explorer = new NativeMultiWorldTesting::SoftmaxExplorer<NativeContext>(*GetNativeScorer(), lambda, (u32)numActions);
		}

		~SoftmaxExplorer()
		{
			delete m_explorer;
		}

	internal:
		virtual List<float>^ InvokeScorerCallback(Ctx context) override
		{
			return defaultScorer->ScoreActions(context);
		}

		NativeMultiWorldTesting::SoftmaxExplorer<NativeContext>* Get()
		{
			return m_explorer;
		}

	private:
		IScorer<Ctx>^ defaultScorer;
		NativeMultiWorldTesting::SoftmaxExplorer<NativeContext>* m_explorer;
	};

	generic <class Ctx>
	public ref class GenericExplorer : public IExplorer<Ctx>, public ScorerCallback<Ctx>
	{
	public:
		GenericExplorer(IScorer<Ctx>^ defaultScorer, UInt32 numActions)
		{
			this->defaultScorer = defaultScorer;
			m_explorer = new NativeMultiWorldTesting::GenericExplorer<NativeContext>(*GetNativeScorer(), (u32)numActions);
		}

		~GenericExplorer()
		{
			delete m_explorer;
		}

	internal:
		virtual List<float>^ InvokeScorerCallback(Ctx context) override
		{
			return defaultScorer->ScoreActions(context);
		}

		NativeMultiWorldTesting::GenericExplorer<NativeContext>* Get()
		{
			return m_explorer;
		}

	private:
		IScorer<Ctx>^ defaultScorer;
		NativeMultiWorldTesting::GenericExplorer<NativeContext>* m_explorer;
	};

	generic <class Ctx>
	public ref class BootstrapExplorer : public IExplorer<Ctx>, public PolicyCallback<Ctx>
	{
	public:
		BootstrapExplorer(cli::array<IPolicy<Ctx>^>^ defaultPolicies, UInt32 numActions)
		{
			this->defaultPolicies = defaultPolicies;
			if (this->defaultPolicies == nullptr)
			{
				throw gcnew ArgumentNullException("The specified array of default policy functions cannot be null.");
			}

			m_explorer = new NativeMultiWorldTesting::BootstrapExplorer<NativeContext>(*GetNativePolicies((u32)defaultPolicies->Length), (u32)numActions);
		}

		~BootstrapExplorer()
		{
			delete m_explorer;
		}

	internal:
		virtual UInt32 InvokePolicyCallback(Ctx context, int index) override
		{
			if (index < 0 || index >= defaultPolicies->Length)
			{
				throw gcnew InvalidDataException("Internal error: Index of interop bag is out of range.");
			}
			return defaultPolicies[index]->ChooseAction(context);
		}

		NativeMultiWorldTesting::BootstrapExplorer<NativeContext>* Get()
		{
			return m_explorer;
		}

	private:
		cli::array<IPolicy<Ctx>^>^ defaultPolicies;
		NativeMultiWorldTesting::BootstrapExplorer<NativeContext>* m_explorer;
	};

	/// <summary>
	/// The top level MwtExplorer class.  Using this makes sure that the
	/// right bits are recorded and good random actions are chosen.
	/// </summary>
	/// <typeparam name="Ctx">The Context type.</typeparam>
	generic <class Ctx>
	public ref class MwtExplorer : public RecorderCallback<Ctx>
	{
	public:
		/// <summary>
		/// Constructor.
		/// </summary>
		/// <param name="appId">This should be unique to each experiment to avoid correlation bugs.</param>
		/// <param name="recorder">A user-specified class for recording the appropriate bits for use in evaluation and learning.</param>
		MwtExplorer(String^ appId, IRecorder<Ctx>^ recorder)
		{
			this->appId = appId;
			this->recorder = recorder;
		}

		/// <summary>
		/// Choose_Action should be drop-in replacement for any existing policy function.
		/// </summary>
		/// <param name="explorer">An existing exploration algorithm (one of the above) which uses the default policy as a callback.</param>
		/// <param name="unique_key">A unique identifier for the experimental unit. This could be a user id, a session id, etc...</param>
		/// <param name="context">The context upon which a decision is made. See SimpleContext above for an example.</param>
		/// <returns>An unsigned 32-bit integer representing the 1-based chosen action.</returns>
		UInt32 ChooseAction(IExplorer<Ctx>^ explorer, String^ unique_key, Ctx context)
		{
			String^ salt = this->appId;
			NativeMultiWorldTesting::MwtExplorer<NativeContext> mwt(marshal_as<std::string>(salt), *GetNativeRecorder());

			GCHandle selfHandle = GCHandle::Alloc(this);
			IntPtr selfPtr = (IntPtr)selfHandle;

			GCHandle contextHandle = GCHandle::Alloc(context);
			IntPtr contextPtr = (IntPtr)contextHandle;

			GCHandle explorerHandle = GCHandle::Alloc(explorer);
			IntPtr explorerPtr = (IntPtr)explorerHandle;

			NativeContext native_context(selfPtr.ToPointer(), explorerPtr.ToPointer(), contextPtr.ToPointer());
			u32 action = 0;
			if (explorer->GetType() == EpsilonGreedyExplorer<Ctx>::typeid)
			{
				EpsilonGreedyExplorer<Ctx>^ epsilonGreedyExplorer = (EpsilonGreedyExplorer<Ctx>^)explorer;
				action = mwt.Choose_Action(*epsilonGreedyExplorer->Get(), marshal_as<std::string>(unique_key), native_context);
			}
			else if (explorer->GetType() == TauFirstExplorer<Ctx>::typeid)
			{
				TauFirstExplorer<Ctx>^ tauFirstExplorer = (TauFirstExplorer<Ctx>^)explorer;
				action = mwt.Choose_Action(*tauFirstExplorer->Get(), marshal_as<std::string>(unique_key), native_context);
			}
			else if (explorer->GetType() == SoftmaxExplorer<Ctx>::typeid)
			{
				SoftmaxExplorer<Ctx>^ softmaxExplorer = (SoftmaxExplorer<Ctx>^)explorer;
				action = mwt.Choose_Action(*softmaxExplorer->Get(), marshal_as<std::string>(unique_key), native_context);
			}
			else if (explorer->GetType() == GenericExplorer<Ctx>::typeid)
			{
				GenericExplorer<Ctx>^ genericExplorer = (GenericExplorer<Ctx>^)explorer;
				action = mwt.Choose_Action(*genericExplorer->Get(), marshal_as<std::string>(unique_key), native_context);
			}
			else if (explorer->GetType() == BootstrapExplorer<Ctx>::typeid)
			{
				BootstrapExplorer<Ctx>^ bootstrapExplorer = (BootstrapExplorer<Ctx>^)explorer;
				action = mwt.Choose_Action(*bootstrapExplorer->Get(), marshal_as<std::string>(unique_key), native_context);
			}

			explorerHandle.Free();
			contextHandle.Free();
			selfHandle.Free();

			return action;
		}

	internal:
		virtual void InvokeRecorderCallback(Ctx context, UInt32 action, float probability, String^ unique_key) override
		{
			recorder->Record(context, action, probability, unique_key);
		}

	private:
		IRecorder<Ctx>^ recorder;
		String^ appId;
	};

	[StructLayout(LayoutKind::Sequential)]
	public value struct Feature
	{
		float Value;
		UInt32 Id;
	};

	/// <summary>
	/// A sample recorder class that converts the exploration tuple into string format.
	/// </summary>
	/// <typeparam name="Ctx">The Context type.</typeparam>
	generic <class Ctx> where Ctx : IStringContext
	public ref class StringRecorder : public IRecorder<Ctx>, public ToStringCallback<Ctx>
	{
	public:
		StringRecorder()
		{
			m_string_recorder = new NativeMultiWorldTesting::StringRecorder<NativeStringContext>();
		}

		~StringRecorder()
		{
			delete m_string_recorder;
		}

		virtual void Record(Ctx context, UInt32 action, float probability, String^ uniqueKey)
		{
			GCHandle contextHandle = GCHandle::Alloc(context);
			IntPtr contextPtr = (IntPtr)contextHandle;

			NativeStringContext native_context(contextPtr.ToPointer(), GetCallback());
			m_string_recorder->Record(native_context, (u32)action, probability, marshal_as<string>(uniqueKey));
		}

		/// <summary>
		/// Gets the content of the recording so far as a string and clears internal content.
		/// </summary>
		/// <returns>
		/// A string with recording content.
		/// </returns>
		String^ GetRecording()
		{
			// Workaround for C++-CLI bug which does not allow default value for parameter
			return GetRecording(true);
		}

		/// <summary>
		/// Gets the content of the recording so far as a string and optionally clears internal content.
		/// </summary>
		/// <param name="flush">A boolean value indicating whether to clear the internal content.</param>
		/// <returns>
		/// A string with recording content.
		/// </returns>
		String^ GetRecording(bool flush)
		{
			return gcnew String(m_string_recorder->Get_Recording(flush).c_str());
		}

	private:
		NativeMultiWorldTesting::StringRecorder<NativeStringContext>* m_string_recorder;
	};

	public ref class SimpleContext : public IStringContext
	{
	public:
		SimpleContext(cli::array<Feature>^ features)
		{
			Features = features;

			// TODO: add another constructor overload for native SimpleContext to avoid copying feature values
			m_features = new vector<NativeMultiWorldTesting::Feature>();
			for (int i = 0; i < features->Length; i++)
			{
				m_features->push_back({ features[i].Value, features[i].Id });
			}

			m_native_context = new NativeMultiWorldTesting::SimpleContext(*m_features);
		}

		String^ ToString() override
		{
			return gcnew String(m_native_context->To_String().c_str());
		}

		~SimpleContext()
		{
			delete m_native_context;
		}

	public:
		cli::array<Feature>^ GetFeatures() { return Features; }

	internal:
		cli::array<Feature>^ Features;

	private:
		vector<NativeMultiWorldTesting::Feature>* m_features;
		NativeMultiWorldTesting::SimpleContext* m_native_context;
	};
}
