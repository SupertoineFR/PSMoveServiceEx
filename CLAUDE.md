# Building PSMoveServiceEx from source (Windows x64)

Toolchain: **Visual Studio 2026 Build Tools (v145 / MSVC 14.5x), CMake 4.x, Boost 1.92**.

## Quick rebuild (deps already built)

```powershell
$cmake = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
& $cmake -S . -B build          # only needed after adding/removing source files (CMake uses file(GLOB))
& $cmake --build build --config Release --target PSMoveService PSMoveClient_CAPI PSMoveConfigTool --parallel 8
```

Omit `--target ...` to build everything. If this fails with `cannot open input file ...\thirdparty\...` or
`MSB8066 ... PSMoveProtocol.pb.h.rule exited with code 3` (protoc.exe missing), a third-party build folder
was deleted: redo the matching step below.

### Ready-to-run folder (service + config tool together)

```powershell
& $cmake --build build --config Release --target stage_release --parallel 8
```

Builds whatever is out of date, then copies everything into **`release\PSMoveService\`** in the same layout as the
release package: `PSMoveService.exe`, `PSMoveServiceAdmin.exe`, `PSMoveConfigTool.exe`, `PSMoveClient_CAPI.dll`,
`assets\` (config tool assets + `misc\calibration`) and `LICENSE.txt`. Run the programs from there. The target is
defined in the root `CMakeLists.txt`; `release/` is git-ignored. (`CreateRelease.bat` does the same plus a zip, but
also expects the stale `src/tests` binaries.)

Raw outputs if needed (`build\src\...\Release\`): `psmoveservice\PSMoveService*.exe`,
`psmoveconfigtool\PSMoveConfigTool.exe`, `psmoveclient\PSMoveClient_CAPI.dll`. Run from there, the config tool can't
find `PSMoveClient_CAPI.dll` — use the staged folder instead.

## Prerequisites

- VS 2026 Build Tools with the C++ workload.
- Use VS's CMake (path above). The `cmake` on the Git Bash PATH has no VS generators.
- Boost 1.92 prebuilt for msvc-14.5 at `C:\local\boost_1_92_0` (libs in `lib64-msvc-14.5`).
- Git + network (OpenCV 3.4.20 is cloned on the first build).
- Submodules: `git submodule update --init --recursive` — this **resets the local submodule patches** listed at the end.

## Full build from scratch (Git Bash, from repo root)

```sh
CMAKE="/c/Program Files (x86)/Microsoft Visual Studio/18/BuildTools/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe"
MSBUILD="/c/Program Files (x86)/Microsoft Visual Studio/18/BuildTools/MSBuild/Current/Bin/MSBuild.exe"
ROOT=$(pwd)

# 1. protobuf -> thirdparty/protobuf/vsprojects/x64/<Config>  (protoc.exe + libs)
cd "$ROOT/thirdparty/protobuf" && rm -rf vsprojects && mkdir vsprojects && cd vsprojects
"$CMAKE" -G "Visual Studio 18 2026" -A x64 -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
  -Dprotobuf_DEBUG_POSTFIX="" -Dprotobuf_BUILD_TESTS=OFF ../cmake
for c in Release Debug; do "$CMAKE" --build . --config $c -- -m; mkdir -p x64/$c; cp $c/* x64/$c/; done

# 2. libusb -> thirdparty/libusb/x64/<Config>/lib
cd "$ROOT/thirdparty/libusb/msvc"
for c in Release Debug; do "$MSBUILD" libusb_2015.sln -p:Configuration=$c -p:Platform=x64 \
  -p:PlatformToolset=v145 -p:WindowsTargetPlatformVersion=10.0 -m; done

# 3. SDL2 (static, Release) -> thirdparty/SDL2/build/Release/SDL2.lib + SDL2main.lib
cd "$ROOT/thirdparty/SDL2" && rm -rf build && mkdir build && cd build
"$CMAKE" .. -G "Visual Studio 18 2026" -A x64 -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
  -DDIRECTX=OFF -DSDL_STATIC=ON -DFORCE_STATIC_VCRT=ON \
  "-DEXTRA_CFLAGS=-MT -Z7 -DSDL_MAIN_HANDLED -DWIN32 -DNDEBUG -D_CRT_SECURE_NO_WARNINGS -DHAVE_LIBC -D_USE_MATH_DEFINES"
"$CMAKE" --build . --config Release --target SDL2-static -- -m
"$CMAKE" --build . --config Release --target SDL2main -- -m

# 4. Configure + build PSMoveService (first build also builds OpenCV into deps/ — slow, ~minutes)
cd "$ROOT"
"$CMAKE" -S . -B build -G "Visual Studio 18 2026" -A x64 \
  -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DPSM_BUILD_TESTS=OFF \
  -DBOOST_ROOT="C:/local/boost_1_92_0" \
  -DBOOST_LIBRARYDIR="C:/local/boost_1_92_0/lib64-msvc-14.5" \
  -DPROTOBUF_SRC_ROOT_FOLDER=thirdparty/protobuf
"$CMAKE" --build build --config Release -- -m -v:m > build_release.log 2>&1
grep -E "error [A-Z]+[0-9]+" build_release.log | sort -u     # list errors, if any

# 5. Assemble the ready-to-run folder release/PSMoveService
"$CMAKE" --build build --config Release --target stage_release
```

Debug builds of protobuf/libusb are only needed for `--config Debug`. Steps 1–3 only need redoing if their output folders are deleted.

## Version bump

- The release version lives only in `src/psmoveprotocol/ProtocolVersion.h`: `PSM_RELEASE_VERSION_{MAJOR,MINOR,BUILD,REVISION}`
  (shown as `MAJOR.MINOR.BUILD.REVISION`, e.g. `0.28.0.0`). `cmake/Version.cmake` reads it from there, so re-run the
  CMake configure step after changing it.
- New feature update → bump MINOR and reset BUILD/REVISION to 0 (0.27.8 → 0.28.0); fixes → bump BUILD.
- Leave the `PSM_PROTOCOL_VERSION_*` defines alone unless the client/service protocol itself changes.
- Do **not** edit `updater/app_version.txt`, `app_hash.txt` or `PSMoveServiceUpdateSFX.dat` by hand. They are the live
  auto-update feed and are regenerated together by `CreateUpdate.bat` when a release is packaged. Bumping
  `app_version.txt` early would offer existing installs an update that doesn't exist yet.

## Running / debugging

- The service logs to the console and to `PSMoveServiceEx.log` in its working directory.
- Configs live in `%APPDATA%\PSMoveService\*.json`.
- Tracker video reaches clients via Boost.Interprocess shared-memory files in `%APPDATA%\PSMoveService\shared_mem`
  (`src/psmoveprotocol/SharedMemoryPath.*`). Service and client DLL must both be built with this; an older
  `PSMoveClient_CAPI.dll` looks in a working-directory-relative `shared_mem` and shows 0 fps video.
- libusb `program assertion failed - existing device should share parent` messages on stderr are noise from the old bundled libusb.

## Compatibility notes / gotchas

- **OpenCV is pinned to 3.4.20** (`PSM_OPENCV_GIT_TAG` / `PSM_OPENCV_LIB_SUFFIX` in `cmake/ThirdParty.cmake`). 3.1.0 can't configure under CMake 4; 4.x removed the C API (`IplImage`, `CV_CAP_PROP_*`) the code still uses. `dnn`/protobuf/ITT/quirc are disabled. Changing the version requires updating the lib suffix.
- If OpenCV options change, delete `deps/opencv/src/opencv-build`, `deps/opencv/src/opencv-stamp` and `deps/local` to force a rebuild.
- **Boost ≥ 1.87** removed deprecated Asio (`io_service`, `resolver::iterator`, `resolver::query`, `mutable_buffers_1`) — use `io_context`, `results_type::iterator`, `mutable_buffer`. Boost.System is header-only (no `system` component in `find_package`). Boost.Filesystem v4: `branch_path()` → `parent_path()`.
- `cmake/Version.cmake` parses `PSM_RELEASE_VERSION_{MAJOR,MINOR,BUILD,REVISION}` from `src/psmoveprotocol/ProtocolVersion.h`.
- `src/tests` is stale: keep `PSM_BUILD_TESTS=OFF`.
- Modern MSVC no longer pulls in `<chrono>`/`<string>` transitively — include them explicitly.
- `ImGui::ButtonChecked` is provided by `src/psmoveconfigtool/ImGuiButtonChecked.h` (not in the imgui submodule).
- `LNK2005` duplicate symbols usually mean Debug and Release libs are mixed.
- "File in use by another process" errors: a previous build or a running PSMoveService still holds the file.

### Local submodule patches

Re-apply if a submodule is reset:
- `thirdparty/protobuf/src/google/protobuf/stubs/hash.h`: MSVC `hash_compare` branch limited to `_MSC_VER < 1600`.
- `thirdparty/protobuf/src/google/protobuf/compiler/java/java_file.cc`: `FieldDescriptorCompare::operator()` made `const`.
- `thirdparty/PS3EYEDriver/src/ps3eye.{h,cpp}`: `bool getFrame(uint8_t* frame, bool wait = true)` (non-blocking when `wait=false`, 1 s bounded wait otherwise).
- `thirdparty/libstem_gamepad/source/gamepad/*`: `Gamepad_init(bool xinputOnly)` (skips DirectInput enumeration when true and XInput is available).
- `thirdparty/Boost.Application/include/boost/application/signal_binder.hpp` and `example/setup/windows/setup/service_setup.hpp`: `io_context` / `parent_path` ports.
