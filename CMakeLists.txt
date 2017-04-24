# cmake Version
cmake_minimum_required(VERSION 2.8)

project(palattice CXX)
set(CMAKE_BUILD_TYPE Release)

# Project Version
set(LIB_VERSION_MAJOR 3.9)
set(LIB_VERSION_MINOR 2)
set(LIB_VERSION ${LIB_VERSION_MAJOR}.${LIB_VERSION_MINOR})

# Install Path
# set(CMAKE_INSTALL_PREFIX "/usr/local")
set(SIMTOOL_PATH ${CMAKE_INSTALL_PREFIX}/lib/libpalattice_simTools)

# Googletest Path
# optional, used to compile tests in tests/ directory
set(GTEST_PATH /home/schmidt/git/googletest) #/set/googletest/path/here


# CXX-Standard
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
  message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()
# CXX All Warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")



# Libraries
set(CMAKE_LIBRARY_PATH
  ${CMAKE_LIBRARY_PATH}
  /usr/lib/x86_64-linux-gnu/SDDS
  /usr/lib/i386-linux-gnu/SDDS
  /usr/lib64/SDDS
  /usr/lib/SDDS
  )

find_library(GSL_LIBRARY gsl)
find_library(GSLCBLAS_LIBRARY gslcblas)
if(NOT GSL_LIBRARY OR NOT GSLCBLAS_LIBRARY)
  message(FATAL_ERROR "Gnu Scientific Library GSL not found!")
endif()
  
find_library(SDDS_LIBRARY SDDS1)
if(SDDS_LIBRARY)
  message("libSDDS1 found! Compile libpalattice with SDDS support.")
  set(LIBPALATTICE_USE_SDDS_TOOLKIT_LIBRARY true)
  find_library(MDBCOMMON_LIBRARY mdbcommon)
  find_library(MDB_LIBRARY mdblib)
  find_library(LZMA_LIBRARY lzma)
  if(NOT MDBCOMMON_LIBRARY OR NOT MDB_LIBRARY OR NOT LZMA_LIBRARY)
    message(FATAL_ERROR "library not found, which is needed for libSDDS1: mdbcommon, mdblib or lzma")
  endif()
  find_library(Z_LIBRARY z)
  if(NOT Z_LIBRARY)
    message(FATAL_ERROR "libz not found!")
  endif()
else(SDDS_LIBRARY)
  message("WARNING: libSDDS1 not found! Compile libpalattice without SDDS support.\nPlease have a look at README file for details.")
  unset(LIBPALATTICE_USE_SDDS_TOOLKIT_LIBRARY)
endif(SDDS_LIBRARY)

# automatically generated header file (by cmake, not build-time!)
# defines LIBPALATTICE_USE_SDDS_TOOLKIT_LIBRARY and simToolPath
configure_file(systemconfig.hpp.in ${CMAKE_SOURCE_DIR}/systemconfig.hpp ESCAPE_QUOTES)





# build 'libpalattice'
add_library(palattice SHARED
  AccElements.cpp
  AccLattice.cpp
  Metadata.cpp
  SimTools.cpp
  Interpolate.cpp
  FunctionOfPos.cpp
  Field.cpp
  Spectrum.cpp
  ELSASpuren.cpp
  )
SET_TARGET_PROPERTIES(
  palattice
  PROPERTIES
  VERSION ${LIB_VERSION_MAJOR}
  SOVERSION ${LIB_VERSION}
  )

# link 'libpalattice'
target_link_libraries(palattice
  ${GSL_LIBRARY}
  ${GSLCBLAS_LIBRARY}
  )
if(SDDS_LIBRARY)
  target_link_libraries(palattice
  ${SDDS_LIBRARY}
  ${MDBCOMMON_LIBRARY}
  ${MDB_LIBRARY}
  ${LZMA_LIBRARY}
  ${Z_LIBRARY}
  )
endif()



# build 'convertlattice'
add_executable(convertlattice convertlattice.cpp)

# link 'convertlattice'
target_link_libraries(convertlattice
  palattice)
if(SDDS_LIBRARY)
  target_link_libraries(convertlattice
    ${Z_LIBRARY}
    )
endif()



# custom target: write header with current version
add_custom_target(version
  ${CMAKE_COMMAND}
  -DSRC=gitversion.hpp.in
  -DDST=gitversion.hpp
  -DLIB_VERSION=${LIB_VERSION}
  -P gitversion.cmake
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  DEPENDS .git/HEAD .git/index
  COMMENT "Write current version to gitversion.hpp"
  )
add_dependencies(palattice version)





# install
install(TARGETS palattice
  LIBRARY DESTINATION lib
  )

install(PROGRAMS simTools/elegant2libpalattice.sh
  DESTINATION bin
  RENAME elegant2libpalattice
  )

install(TARGETS convertlattice
  DESTINATION bin
  )

install(FILES
  AccElements.hpp
  AccLattice.hpp
  AccIterator.hpp
  AccIterator.hxx
  Metadata.hpp
  SimTools.hpp
  Interpolate.hpp
  Interpolate.hxx
  FunctionOfPos.hpp
  FunctionOfPos.hxx
  Field.hpp
  Spectrum.hpp
  ELSASpuren.hpp
  types.hpp
  config.hpp
  systemconfig.hpp
  gitversion.hpp
  libpalattice.hpp
  DESTINATION include/libpalattice
  )

install(FILES
  simTools/libpalattice.ele
  simTools/libpalattice.madx
  simTools/libpalattice_twissonly.madx
  DESTINATION ${SIMTOOL_PATH}
  )

# uninstall
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)
add_custom_target(uninstall
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/cmake_uninstall.cmake)





# some tests with googletest (optional, only compiled if GTEST_PATH is found)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tests)





# example programs
# programs are a separate project & excluded from 'all' target,
# because they are linked against INSTALLED libpalattice
# to demonstrate usage with your own programs.
# use 'make', 'make install' first to install libpalattice
# and then 'make programs'.
# See README for details.
include(ExternalProject)
ExternalProject_Add(programs
  PREFIX programs
  SOURCE_DIR ${CMAKE_SOURCE_DIR}/programs
  INSTALL_COMMAND ""
  )
set_target_properties(programs PROPERTIES EXCLUDE_FROM_ALL TRUE)
