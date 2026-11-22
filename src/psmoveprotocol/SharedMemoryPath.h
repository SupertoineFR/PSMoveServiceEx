#ifndef SHARED_MEMORY_PATH_H
#define SHARED_MEMORY_PATH_H

#ifdef _WIN32
#include <string>

// On Windows, boost::interprocess emulates shared memory with files on disk.
// The service and every client have to map the same files, so the folder
// must not depend on the working directory of each process.
//
// The tracker video frames are kept in %APPDATA%\PSMoveService\shared_mem,
// next to the service config files. If APPDATA is not set, the old
// working-directory-relative "shared_mem" folder is used instead.
//
// Defining BOOST_INTERPROCESS_SHARED_DIR_FUNC makes boost::interprocess
// call boost::interprocess::ipcdetail::get_shared_dir(), which is
// implemented in SharedMemoryPath.cpp on top of these functions.
#define BOOST_INTERPROCESS_SHARED_DIR_FUNC

std::string get_shared_memory_directory();
std::wstring get_shared_memory_directory_wide();
#endif // _WIN32

#endif // SHARED_MEMORY_PATH_H
