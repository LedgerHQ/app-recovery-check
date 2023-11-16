cmake_minimum_required(VERSION 3.10)

if(${CMAKE_VERSION} VERSION_LESS 3.10)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

# project information
project(unit_tests
        VERSION 1.5.1
	      DESCRIPTION "Unit tests for app-seed-tool Ledger Application"
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
  message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there. You may need to remove CMakeCache.txt. ")
endif()

# Fetch cmocka
find_package(cmocka QUIET)
include(FetchContent)
FetchContent_Declare(
    cmocka
    GIT_REPOSITORY https://git.cryptomilk.org/projects/cmocka.git
    GIT_TAG        cmocka-1.1.5
    GIT_SHALLOW    1
)
set(WITH_STATIC_LIB ON CACHE BOOL "CMocka: Build with a static library" FORCE)
set(WITH_CMOCKERY_SUPPORT OFF CACHE BOOL "CMocka: Install a cmockery header" FORCE)
set(WITH_EXAMPLES OFF CACHE BOOL "CMocka: Build examples" FORCE)
set(UNIT_TESTING OFF CACHE BOOL "CMocka: Build with unit testing" FORCE)
set(PICKY_DEVELOPER OFF CACHE BOOL "CMocka: Build with picky developer flags" FORCE)
FetchContent_MakeAvailable(cmocka)

add_compile_definitions(TEST DEBUG=0 SKIP_FOR_CMOCKA)
add_compile_definitions(HAVE_HASH HAVE_HMAC HAVE_SHA256 HAVE_SHA512 HAVE_PBKDF2 HAVE_ECC HAVE_CRC HAVE_RNG IO_HID_EP_LENGTH=64)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib $ENV{LEDGER_SECURE_SDK}/include $ENV{LEDGER_SECURE_SDK}/lib_cxng/src $ENV{LEDGER_SECURE_SDK}/lib_cxng/include $ENV{LEDGER_SECURE_SDK}/lib_ux/include $ENV{LEDGER_SECURE_SDK}/lib_bagl/include)

# add src
set(LIB_SOURCES ./lib/testutils.c $ENV{LEDGER_SECURE_SDK}/lib_cxng/src/cx_ram.c $ENV{LEDGER_SECURE_SDK}/lib_cxng/src/cx_hash.c $ENV{LEDGER_SECURE_SDK}/lib_cxng/src/cx_sha256.c $ENV{LEDGER_SECURE_SDK}/lib_cxng/src/cx_sha512.c $ENV{LEDGER_SECURE_SDK}/lib_cxng/src/cx_pbkdf2.c $ENV{LEDGER_SECURE_SDK}/lib_cxng/src/cx_hmac.c $ENV{LEDGER_SECURE_SDK}/lib_cxng/src/cx_utils.c)
add_library(testutils SHARED ${LIB_SOURCES})

add_library(shamir SHARED ../../src/bc-sskr/bc-shamir/shamir.c ../../src/bc-sskr/bc-shamir/interpolate.c ../../src/bc-sskr/bc-shamir/hazmat.c)
target_include_directories(shamir PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../src/bc-sskr/bc-shamir)

add_library(sskr SHARED ../../src/bc-sskr/sskr.c)
target_include_directories(sskr PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../src/bc-sskr/bc-shamir ${CMAKE_CURRENT_SOURCE_DIR}/../../src/bc-sskr)

# add cmocka tests
add_executable(test_shamir tests/shamir.c)
target_link_libraries(test_shamir PUBLIC cmocka gcov testutils shamir)

add_executable(test_sskr tests/sskr.c)
target_link_libraries(test_sskr PUBLIC cmocka gcov testutils sskr shamir)

add_executable(test_bip39 ./tests/bip39.c ../../src/ux_common/onboarding_seed_rom_variables.c  ../../src/ux_common/onboarding_seed_bip39.c)
target_include_directories(test_bip39 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../src/ux_common)
target_link_libraries(test_bip39 PUBLIC cmocka gcov testutils)

add_executable(test_roundtrip ./tests/roundtrip.c ../../src/ux_common/onboarding_seed_rom_variables.c ../../src/ux_common/onboarding_seed_bip39.c ../../src/ux_common/onboarding_seed_sskr.c)
target_include_directories(test_roundtrip PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../src ${CMAKE_CURRENT_SOURCE_DIR}/../../src/ux_common)
target_link_libraries(test_roundtrip PUBLIC cmocka gcov testutils sskr shamir)

foreach(target test_shamir test_sskr test_bip39 test_roundtrip)
    add_test(NAME ${target} COMMAND ${target})
endforeach()
