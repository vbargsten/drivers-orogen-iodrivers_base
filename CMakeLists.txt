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

option(INSTALL_OROGEN_TEST_HELPERS
       "whether test helpers should be installed (requires Ruby)" ON)
if (INSTALL_OROGEN_TEST_HELPERS)
    find_package(Rock REQUIRED)
    include(RockRuby)
    install(FILES test/orogen_test_helpers.rb
            DESTINATION ${RUBY_LIBRARY_INSTALL_DIR}/iodrivers_base)
endif()