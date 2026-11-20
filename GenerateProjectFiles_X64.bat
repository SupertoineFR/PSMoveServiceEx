@echo off
call SetBuildVars_x64.bat

IF NOT EXIST build mkdir build
pushd build

echo "Rebuilding PSMoveService x64 Project files..."
IF NOT DEFINED CMAKE_EXE set "CMAKE_EXE=cmake"
"%CMAKE_EXE%" .. -G "Visual Studio 18 2026" -A x64 -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DPSM_BUILD_TESTS=OFF -DBOOST_ROOT=%BOOST_ROOT_PATH% -DBOOST_LIBRARYDIR=%BOOST_LIB_PATH% -DPROTOBUF_SRC_ROOT_FOLDER=..\thirdparty\protobuf
IF %ERRORLEVEL% NEQ 0 (
  echo "Error generating PSMoveService 64-bit project files"
  goto failure
)

popd
EXIT /B 0

:failure
pause
EXIT /B 1