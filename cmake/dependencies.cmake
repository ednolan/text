# Copyright Louis Dionne 2016
# Copyright Zach Laine 2016-2017
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

###############################################################################
# Boost
###############################################################################
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.71.0 COMPONENTS ${boost_components})
if (Boost_INCLUDE_DIR)
  add_library(boost INTERFACE)
  target_include_directories(boost INTERFACE ${Boost_INCLUDE_DIR})
  target_link_directories(boost INTERFACE ${Boost_INCLUDE_DIR}/stage/lib)
else ()
  if (NOT EXISTS ${CMAKE_BINARY_DIR}/boost_root)
    if (NOT BOOST_BRANCH)
      set(BOOST_BRANCH master)
    endif()
    message("-- Boost was not found; it will be cloned locally from ${BOOST_BRANCH}.")
    add_custom_target(
      boost_root_clone
      git clone --depth 100 -b ${BOOST_BRANCH}
        https://github.com/boostorg/boost.git boost_root
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
    if (MSVC)
      set(bootstrap_cmd ./bootstrap.bat)
    else()
      set(bootstrap_cmd ./bootstrap.sh)
    endif()
    add_custom_target(
      boost_clone
      COMMAND git submodule init tools/boostdep
      COMMAND git submodule init tools/build
      COMMAND git submodule init tools/boost_install
      COMMAND git submodule init libs/headers
      COMMAND git submodule init libs/config
      COMMAND git submodule update --jobs 3 --depth 100
      # Copy the relevant parts of this under the working dir, so depinst.py
      # can see them.
      COMMAND ${CMAKE_COMMAND} -E make_directory libs/text
      COMMAND ${CMAKE_COMMAND} -E copy_directory ../../include ../../test ../../src libs/text
      COMMAND ${CMAKE_COMMAND} -E make_directory libs/text/test
      COMMAND ${CMAKE_COMMAND} -E copy ../../test/break_api.cpp libs/text/test
      COMMAND python tools/boostdep/depinst/depinst.py --git_args "--jobs 3" text
      COMMAND ${bootstrap_cmd}
      COMMAND ./b2 headers
      COMMAND ./b2 -j2
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/boost_root
      DEPENDS boost_root_clone)
  endif()
  add_library(boost INTERFACE)
  target_include_directories(boost INTERFACE ${CMAKE_BINARY_DIR}/boost_root)
  set(Boost_INCLUDE_DIR ${CMAKE_BINARY_DIR}/boost_root)
  if (NOT LIMIT_TESTING_FOR_CI)
    target_link_directories(boost INTERFACE ${CMAKE_BINARY_DIR}/boost_root/stage/lib)
  endif()
endif ()


###############################################################################
# GoogleTest
###############################################################################
add_subdirectory(${CMAKE_SOURCE_DIR}/googletest-release-1.10.0)
target_compile_definitions(gtest      PUBLIC GTEST_LANG_CXX11=1)
target_compile_definitions(gtest_main PUBLIC GTEST_LANG_CXX11=1)
target_include_directories(gtest      INTERFACE $<BUILD_INTERFACE:${CMAKE_HOME_DIRECTORY}/googletest-release-1.10.0/googletest/include>)
target_include_directories(gtest_main INTERFACE $<BUILD_INTERFACE:${CMAKE_HOME_DIRECTORY}/googletest-release-1.10.0/googletest/include>)


###############################################################################
# Google Benchmark
###############################################################################
add_subdirectory(${CMAKE_SOURCE_DIR}/benchmark-v1.2.0)


###############################################################################
# ICU, for perf comparisons
###############################################################################
if (MSVC)
    find_package(ICU 60 COMPONENTS in dt uc)
else()
    find_package(ICU 60 COMPONENTS i18n data uc)
endif()
