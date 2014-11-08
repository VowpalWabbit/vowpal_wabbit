#pragma once

using namespace System;
using namespace System::Collections::Generic;

namespace MultiWorldTesting {

generic <class Ctx>
public interface class IRecorder
{
public:
	virtual void Record(Ctx context, UInt32 action, float probability, String^ uniqueKey) = 0;
};

generic <class Ctx>
public interface class IPolicy
{
public:
	virtual UInt32 ChooseAction(Ctx context) = 0;
};

generic <class Ctx>
public interface class IScorer
{
public:
	virtual List<float>^ ScoreActions(Ctx context) = 0;
};

generic <class Ctx>
public interface class IExplorer
{
};

}