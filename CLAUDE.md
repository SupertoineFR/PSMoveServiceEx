# Building PSMoveServiceEx from source (Windows x64)

Toolchain this repo is set up for: **Visual Studio 2026 (v145 / MSVC 14.5x), CMake 4.x, Boost 1.92**.
The original upstream scripts targeted VS 2015 / Boost 1.6x / OpenCV 3.1; several fixes were needed to build with the modern toolchain (see "Compatibility notes").

## Prerequisites

- Visual Studio 2026 (Community or Build Tools) with the C++ workload.
- Use the CMake that ships with VS:
  `C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe`
  The `cmake` on the Git Bash/MSYS PATH has no Visual Studio generators — don't use it.
- Boost 1.92 prebuilt for msvc-14.5 at `C:\local\boost_1_92_0` (libs in `lib64-msvc-14.5`).
- Git + network access (OpenCV is cloned at build time).
- Submodules checked out: `git submodule update --init --recursive`
  (warning: this also resets the local submodule patches listed below).

In the commands below, `CMAKE` = the VS cmake.exe path above and
`MSBUILD` = `C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\MSBuild\Current\Bin\MSBuild.exe`.

## 1. Build third-party dependencies (once)

Equivalent of `InitialSetup_X64.bat`, adapted for VS 2026. Run from the repo root.

**protobuf** (Debug + Release; FindProtobuf expects copies under `x64\<Config>`):
```sh
cd thirdparty/protobuf && rm -rf vsprojects && mkdir vsprojects && cd vsprojects
"$CMAKE" -G "Visual Studio 18 2026" -A x64 -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
  -Dprotobuf_DEBUG_POSTFIX="" -Dprotobuf_BUILD_TESTS=OFF ../cmake
for c in Debug Release; do "$CMAKE" --build . --config $c -- -m; mkdir -p x64/$c; cp $c/* x64/$c/; done
```

**libusb** (Debug + Release):
```sh
cd thirdparty/libusb/msvc
for c in Debug Release; do "$MSBUILD" libusb_2015.sln -p:Configuration=$c -p:Platform=x64 \
  -p:PlatformToolset=v145 -p:WindowsTargetPlatformVersion=10.0 -m; done
```

**SDL2** (static, Release only):
```sh
cd thirdparty/SDL2 && rm -rf build && mkdir build && cd build
"$CMAKE" .. -G "Visual Studio 18 2026" -A x64 -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
  -DDIRECTX=OFF -DSDL_STATIC=ON -DFORCE_STATIC_VCRT=ON \
  "-DEXTRA_CFLAGS=-MT -Z7 -DSDL_MAIN_HANDLED -DWIN32 -DNDEBUG -D_CRT_SECURE_NO_WARNINGS -DHAVE_LIBC -D_USE_MATH_DEFINES"
"$CMAKE" --build . --config Release --target SDL2-static -- -m
"$CMAKE" --build . --config Release --target SDL2main -- -m
```

## 2. Configure and build PSMoveService

```sh
"$CMAKE" -S . -B build -G "Visual Studio 18 2026" -A x64 \
  -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DPSM_BUILD_TESTS=OFF \
  -DBOOST_ROOT="C:/local/boost_1_92_0" \
  -DBOOST_LIBRARYDIR="C:/local/boost_1_92_0/lib64-msvc-14.5" \
  -DPROTOBUF_SRC_ROOT_FOLDER=thirdparty/protobuf
"$CMAKE" --build build --config Release -- -m -v:m > build_release.log 2>&1
```
Alternatively: `SetBuildVars_x64.bat` + `GenerateProjectFiles_X64.bat` (both updated for this toolchain), then build `build\PSMoveService.sln`.

The first build clones and compiles OpenCV 3.4.20 via ExternalProject into `deps/` (slow; run in background). Installed libs land in `deps/local/staticlib`.

Outputs:
- `build/src/psmoveservice/Release/PSMoveService.exe`, `PSMoveServiceAdmin.exe`
- `build/src/psmoveconfigtool/Release/PSMoveConfigTool.exe`
- `build/src/psmoveclient/Release/PSMoveClient_CAPI.dll`

Find errors quickly: `grep -E "error [A-Z]+[0-9]+" build_release.log | sort -u`

## Compatibility notes / gotchas

- **OpenCV is pinned to 3.4.20** (`PSM_OPENCV_GIT_TAG` / `PSM_OPENCV_LIB_SUFFIX` in `cmake/ThirdParty.cmake`). 3.1.0 can't configure under CMake 4; 4.x removed the C API (`IplImage`, `CV_CAP_PROP_*`) the code still uses. `dnn`/protobuf/ITT/quirc are disabled. Changing the version requires updating the lib suffix.
- If OpenCV options change, delete `deps/opencv/src/opencv-build`, `deps/opencv/src/opencv-stamp` and `deps/local` to force a clean rebuild.
- **Boost ≥ 1.87** removed deprecated Asio (`io_service`, `resolver::iterator`, `resolver::query`, `mutable_buffers_1`) — use `io_context`, `results_type::iterator`, `mutable_buffer`. Boost.System is header-only (no `system` component in `find_package`). Boost.Filesystem v4: `branch_path()` → `parent_path()`.
- `cmake/Version.cmake` parses `PSM_RELEASE_VERSION_{MAJOR,MINOR,BUILD,REVISION}` from `src/psmoveprotocol/ProtocolVersion.h`.
- `src/tests` is stale (references removed Kalman filter sources): keep `PSM_BUILD_TESTS=OFF`.
- Modern MSVC no longer pulls in `<chrono>`/`<string>` transitively — include them explicitly.
- `ImGui::ButtonChecked` is provided by `src/psmoveconfigtool/ImGuiButtonChecked.h` (not in the imgui submodule).

### Local submodule patches (not tracked by the main repo)

These live inside submodules and are lost on `git submodule update`; re-apply if needed:
- `thirdparty/protobuf/src/google/protobuf/stubs/hash.h`: MSVC `hash_compare` branch limited to `_MSC_VER < 1600`.
- `thirdparty/protobuf/src/google/protobuf/compiler/java/java_file.cc`: `FieldDescriptorCompare::operator()` made `const`.
- `thirdparty/PS3EYEDriver/src/ps3eye.{h,cpp}`: `bool getFrame(uint8_t* frame, bool wait = true)` (non-blocking when `wait=false`, 1 s bounded wait otherwise).
- `thirdparty/libstem_gamepad/source/gamepad/*`: `Gamepad_init(bool xinputOnly)` (skips DirectInput enumeration when true and XInput is available).
- `thirdparty/Boost.Application/include/boost/application/signal_binder.hpp` and `example/setup/windows/setup/service_setup.hpp`: `io_context` / `parent_path` ports.
