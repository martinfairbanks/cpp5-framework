@echo off
set CompilerFlags=-nologo -MT -O2 -fp:fast -Gm- -GR- -EHa- -W4 -WX -wd4100 -wd4201 -wd4189 -wd4706 -wd4996 -wd4127 -FC -DDEVELOPER=0 -DDEBUGGER_MSVC=1
set LinkerFlags=-incremental:no -opt:ref -subsystem:windows

IF NOT EXIST build mkdir build
pushd build
    REM 64-bit debug build with statically linked c-runtime library
    REM cl %CompilerFlags% ..\code\examples_git\shapes.cpp -link %LinkerFlags%

    REM 64-bit build release build with statically linked c-runtime library
    cl %CompilerFlags% ../code/examples/shapes.cpp -link %LinkerFlags%
    cl %CompilerFlags% ../code/examples/lissajous_curves.cpp -link %LinkerFlags%
    cl %CompilerFlags% ../code/examples/stars2d.cpp -link %LinkerFlags%
    cl %CompilerFlags% ../code/examples/stars3d.cpp -link %LinkerFlags%
    cl %CompilerFlags% ../code/examples/test_input.cpp -link %LinkerFlags%
    del *.obj
   REM del *.exp
   REM del *.lib
popd

REM Compiler switches
REM -Wall                       - turn on all warnings
REM -W4                         - turn on warning level 4
REM -WX                         - treat warnings as errors and don't compile the file
REM -wd4100                     - disable warning about unreferenced formal parameter
REM -wd4189                     - disable warning about local variable is initialized but not referenced
REM -wd4706                     - disable warning about assignment within conditional expression
REM -wd4996                     - turn of MSVCs warnings about unsafe functions
REM -wd4127                     - warning in glm dependency (conditional expression is constant)
REM -nologo                     - turn off microsoft information
REM -Gm-                        - turn off minimal rebuild, ha på om man har mycket filer i sitt projekt, ingen mening när man har single translation unit
REM -GR-                        - turn off c++ runtime type information
REM -GS- /Gs9999999             - disable security feature 
REM -EHa-                       - turn off c++ excpetions (try and catch)
REM -Od                         - no optimization, to prevent code from changing after a build
REM -Oi                         - turn on compiler intrinsics even in debug build
REM -MD                         - load c runtime library dll dynamicly. Default in visual studio IDE.
REM -MT                         - use the static library and link it into the file. prevents crashes if the system can't find the right version of MSVCR120.dll. default from command line.
REM -MTd                        - -""- debug build
REM -Zi                         - debug build, produces debug information, pdb files, z7 = just produces 1 pdb file, older version 
REM -FC                         - error messages shows full path name to the file 
REM -O2 -fp:fast                - Optimization switches, O2 will override 0d
REM -LD                         - build a dll file

REM Linker switches
REM -sybsystem:windows          - windows app or -subsystem:console for console app
REM -nodefaultlib               - disable C-Runtime library
REM -STACK:0x100000,0x100000    - so executable has 1MB of stack available 
REM -opt:ref                    - eliminates functions and data that are never referenced; /OPT:NOREF keeps functions and data that are never referenced. 
