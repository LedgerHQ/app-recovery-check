cmake_minimum_required(VERSION 3.10)
project(unit_tests
    VERSION 0.1
    DESCRIPTION "Unit tests for the Recovery Check application"
    LANGUAGES C
)

#######################################################################
#                        Setup SDK & LedgerUT                         #
#######################################################################
if(NOT DEFINED ENV{BOLOS_SDK})
    message(FATAL_ERROR "BOLOS_SDK environment variable is not set. Please set it to the path of the Ledger Secure SDK.")
endif()

set(SDK_DIR $ENV{BOLOS_SDK})
set(LEDGER_UT_APP_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../src)
include(${SDK_DIR}/cmake/LedgerUT.cmake)

# libsodium computes the real SHA-512, HMAC and PBKDF2 behind the crypto mocks
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(Sodium REQUIRED)

#######################################################################
#             Setup app specific includes & compile defs              #
#######################################################################
set(APP_DIR ${LEDGER_UT_APP_SRC_DIR})
set(FAKES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/fakes)

set(UT_INCLUDE_DIRS
    ${FAKES_DIR}
    ${APP_DIR}
    ${APP_DIR}/mnemonic_common
    ${APP_DIR}/nbgl
    ${UT_INCLUDE_DIRS_SDK}
)

set(UT_COMPILE_DEFS
    ${UT_COMPILE_DEFS_SDK}
    APPNAME="Recovery Check"
    APPVERSION="a.b.c"
)

#######################################################################
#                           LedgerUT Setup                            #
#######################################################################
ledger_unit_tests_init()
ledger_unit_tests_find_sources()

# Built only so that every app source has a .gcno file for the coverage baseline
ledger_unit_tests_build_app_sources_as_lib(
    SOURCES      ${APP_SOURCES}
    INCLUDE_DIRS ${UT_INCLUDE_DIRS}
    COMPILE_DEFS ${UT_COMPILE_DEFS}
)

#######################################################################
#                        LedgerUT test suites                         #
#######################################################################
ledger_unit_tests_add_test(
    NAME         test_nbgl_mnemonic
    SOURCES      ${APP_DIR}/nbgl/mnemonic.c
                 ${APP_DIR}/mnemonic_common/seed_bip39.c
                 ${APP_DIR}/mnemonic_common/seed_rom_variables.c
                 ${FAKES_DIR}/fake_crypto.c
                 ${FAKES_DIR}/fake_sdk.c
    MOCK_HEADERS ${SDK_DIR}/lib_cxng/include/lcx_hmac.h
                 ${SDK_DIR}/lib_cxng/include/lcx_pbkdf2.h
                 ${SDK_DIR}/lib_cxng/include/lcx_sha256.h
                 ${SDK_DIR}/include/os_seed.h
                 ${SDK_DIR}/include/os_utils.h
    INCLUDE_DIRS ${UT_INCLUDE_DIRS}
    COMPILE_DEFS ${UT_COMPILE_DEFS}
)
target_link_libraries(test_nbgl_mnemonic PRIVATE sodium)
