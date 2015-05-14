Exploration Library
=======

The exploration library addresses the ‘gathering the data’ aspect of machine learning rather than the ‘using the data’ aspect we are most familiar with. The primary goal here is to enable individuals (i.e. you) to gather the right data for using machine learning for interventions in a live system based on user feedback (click, dwell, correction, etc…). Empirically, gathering the right data has often made a substantial difference. Theoretically, we know it is required to disentangle causation from correlation effectively in general.

First version of client-side exploration library that includes the following exploration algorithms:
- Epsilon Greedy
- Tau First
- Softmax
- Bootstrap
- Generic (users can specify custom weight for every action)

This release supports C++ and C#. 

For sample usage, see:

C++: https://github.com/multiworldtesting/explore/blob/master/explore_sample.cpp

C#: https://github.com/multiworldtesting/explore/blob/master/ExploreSample/ExploreOnlySample.cs
