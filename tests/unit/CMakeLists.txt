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

# guard against in-source builds
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build \
                       directory) and run CMake from there. You may need to remove CMakeCache.txt.")
endif()

add_compile_definitions(WIDE=)
add_compile_definitions(TEST)
add_compile_definitions(TARGET_STAX)
add_compile_definitions(OS_IO_SEPROXYHAL)

include_directories(../../src ./mocks/)

add_executable(test_stax_mnemonic test_stax_mnemonic.c)

add_library(stax SHARED ../../src/ux_common/onboarding_seed_rom_variables.c ../../src/stax/mnemonic.c)

target_link_libraries(test_stax_mnemonic PUBLIC cmocka gcov stax)

add_test(test_stax_mnemonic test_stax_mnemonic)
