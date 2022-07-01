@echo off

if not exist "cmake_vstudio" mkdir cmake_vstudio
cd cmake_vstudio

if "%VisualStudioVersion%" == "15.0" goto generate_vs_2017

echo ERROR: VS 2017 found!
echo Try running from a corresponding VS command prompt, please.
echo Exiting...
goto EOF


:generate_vs_2017
echo Using VS 2017 generator
cmake -DCMAKE_SYSTEM_VERSION=8.1 -G "Visual Studio 15 Win64" -T v141 ./..
goto EOF

:EOF
cd ..