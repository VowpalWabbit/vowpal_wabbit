#pragma once

using namespace System;

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
	virtual UInt32 Choose_Action(Ctx context) = 0;
};

generic <class Ctx>
public interface class IExplorer
{
};

}