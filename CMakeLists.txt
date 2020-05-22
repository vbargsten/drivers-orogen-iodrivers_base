PROJECT(iodrivers_base)
cmake_minimum_required(VERSION 2.6)

SET (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/.orogen/config")
INCLUDE(iodrivers_baseBase)

# FIND_PACKAGE(KDL)
# FIND_PACKAGE(OCL)

if (ROCK_TEST_ENABLED)
    enable_testing()
    find_package(Syskit REQUIRED)
    syskit_orogen_tests(test)
endif()
