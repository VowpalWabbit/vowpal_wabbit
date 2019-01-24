## Windows specific info

This document details differences in pre-requisites and building instructions under Windows.

## Prerequisite software

### [Visual Studio 2015](https://visualstudio.microsoft.com/vs/older-downloads/)

*Note that this may require an MSDN subscription*

Make sure you install all of the following **Workloads**

- Desktop development with C++
- .NET desktop development
- Azure development
- Python development
- Windows 8.1 SDK

### [Visual Studio 2017](https://visualstudio.microsoft.com/downloads/?utm_medium=microsoft&utm_source=docs.microsoft.com&utm_campaign=button+cta&utm_content=download+vs2017)

Make sure you install all of the following **Workloads**

- Desktop development with C++
- .NET desktop development
- Azure development
- Python development

Make sure you also install all of the following **Individual components**

- VC++ 2015.3 v14.00 (v140) toolset for desktop
- Windows 8.1 SDK

## Compiling

*This assumes running inside of "Developer Command Prompt" for your instance of Visual Studio*

```
cd .scripts
restore
build
```