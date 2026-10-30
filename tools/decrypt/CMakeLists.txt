# Host build of sf_decrypt, run by the main CMakeLists.txt at configure time when the disc's
# BOOT.BIN is not a plain ELF (works the same when the main build is cross-compiling).
cmake_minimum_required(VERSION 3.16)
project(sf_decrypt_host C CXX)
set(CMAKE_CXX_STANDARD 17)
set(R ${CMAKE_CURRENT_SOURCE_DIR}/../..)
add_executable(sf_decrypt sf_decrypt.cpp ${R}/src/psp/boot_module.cpp ${R}/src/psp/iso.cpp
  ${R}/src/psp/ppsspp/PrxDecrypter.cpp ${R}/third_party/libkirk/kirk_engine.c ${R}/third_party/libkirk/AES.c
  ${R}/third_party/libkirk/SHA1.c ${R}/third_party/libkirk/bn.c ${R}/third_party/libkirk/ec.c)
target_include_directories(sf_decrypt PRIVATE ${R}/src ${R}/src/psp/ppsspp ${R}/third_party/libkirk)
if(NOT MSVC)
  set_source_files_properties(${R}/third_party/libkirk/kirk_engine.c ${R}/third_party/libkirk/AES.c
    ${R}/third_party/libkirk/SHA1.c ${R}/third_party/libkirk/bn.c ${R}/third_party/libkirk/ec.c PROPERTIES COMPILE_OPTIONS -w)
endif()
