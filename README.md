# x64 Deep Dive in practice

This code follows the track from the article [X64 Deep Dive](
https://codemachine.com/articles/x64_deep_dive.html
) and recreate some source code to show how Visual Studio is generating the x64
code.

We will walk thru some changes compared to x86 in order to improve our skills
debugging x64 code:

- Fastcall is the default calling convention.
- RBP is no longer used as frame pointer.
- Last call can be optimized using tail elimination.
- Frame pointer omission (RBP not used as base pointer).

## Generate x86 compilation

In order to enable 32 bits compilation in Windows 64 bits you need to run cmake using the parameter -A.

```
mkdir build && cd build
mkdir Win64 && cd Win64
cmake ..\..
cd ..
mkdir Win32 && cd Win32
cmake ..\.. -A Win32
```

