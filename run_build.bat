@echo off
cd /d "t:\github\PSMoveServiceEx"
mkdir build 2>nul
cd build
C:\Program Files\CMake\bin\cmake.exe . -G Ninja -DCMAKE_POLICY_VERSION_MINIMUM 3.5 -DBOOST_ROOT "C:\local\boost_1_92_0" -DBOOST_LIBRARYDIR "C:\local\boost_1_92_0\lib64-msvc-14.5" -DPROTOBUF_SRC_ROOT_FOLDER ..\thirdparty\protobuf > cmake_configure.log 2>&1
echo Exit code: %ERRORLEVEL%
type cmake_configure.log