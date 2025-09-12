cmake_minimum_required(VERSION 3.10)

if(${CMAKE_VERSION} VERSION_LESS 3.10)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

# project information
project(unit_tests
        VERSION 0.1
	      DESCRIPTION "Unit tests for the Recovery Check application"
        LANGUAGES C)


# guard against bad build-type strings
if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()

include(CTest)
ENABLE_TESTING()

# specify C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -pedantic -g -O0 --coverage")

set(GCC_COVERAGE_LINK_FLAGS "--coverage -lgcov")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")

# adding cmake module directory
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# guard against in-source builds
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build \
                       directory) and run CMake from there. You may need to remove CMakeCache.txt.")
endif()

add_compile_definitions(WIDE=)
add_compile_definitions(TEST)
add_compile_definitions(SCREEN_SIZE_WALLET)
add_compile_definitions(OS_IO_SEPROXYHAL)

find_package(Sodium REQUIRED)

include_directories(
  ../../src
  ../../src/mnemonic_common/
  ./mocks/
  libsodium/
)

add_executable(test_nbgl_mnemonic test_nbgl_mnemonic.c)

add_library(libs SHARED
  ../../src/mnemonic_common/seed_rom_variables.c
  ../../src/mnemonic_common/seed_bip39.c
  ../../src/nbgl/mnemonic.c
  ./mocks/os.c)

target_link_libraries(test_nbgl_mnemonic PUBLIC cmocka gcov libs sodium)

add_test(test_nbgl_mnemonic test_nbgl_mnemonic)
