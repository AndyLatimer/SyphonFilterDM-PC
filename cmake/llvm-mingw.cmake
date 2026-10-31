# Windows x64 cross build with an llvm-mingw toolchain (Clang + LLD + mingw-w64 UCRT + libc++),
# e.g. https://github.com/mstorsjo/llvm-mingw releases, or one built with its scripts on top of a
# system Clang (BUILD.md). Its bin/ must be on PATH, or set LLVM_MINGW to its root.
# libc++ on mingw-w64 uses the native Win32 threads (SRW locks, condition variables): the same
# cheap thread switches as GCC's win32 thread model in cmake/mingw-w64.cmake.
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)
if(DEFINED ENV{LLVM_MINGW})
  set(_tc "$ENV{LLVM_MINGW}/bin/")
else()
  set(_tc "")
endif()
set(CMAKE_C_COMPILER ${_tc}x86_64-w64-mingw32-clang)
set(CMAKE_CXX_COMPILER ${_tc}x86_64-w64-mingw32-clang++)
set(CMAKE_RC_COMPILER ${_tc}x86_64-w64-mingw32-windres)
set(CMAKE_AR ${_tc}llvm-ar CACHE FILEPATH "")
set(CMAKE_RANLIB ${_tc}llvm-ranlib CACHE FILEPATH "")
add_compile_definitions(_WIN32_WINNT=0x0A00 WINVER=0x0A00)
set(CA_LLVM_MINGW ON)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)
