# cmake Version
cmake_minimum_required(VERSION 2.8)

project(palattice CXX)
set(CMAKE_BUILD_TYPE Debug)

# Project Version
set(LIB_VERSION_MAJOR 3.7)
set(LIB_VERSION_MINOR 2)
set(LIB_VERSION ${LIB_VERSION_MAJOR}.${LIB_VERSION_MINOR})

# Install Path
set(CMAKE_INSTALL_PREFIX "/usr/local")
set(SIMTOOL_PATH ${CMAKE_INSTALL_PREFIX}/lib/libpalattice_simTools)

# Googletest Path
# optional, used to compile tests in tests/ directory
set(GTEST_PATH /home/thiry/dev/gtest-1.7.0) #/set/googletest/path/here


# CXX-Standard & All Warnings
if (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 3.1)
  set_property(TARGET palattice PROPERTY CXX_STANDARD 11)
  set_property(TARGET palattice PROPERTY CXX_STANDARD_REQUIRED ON)
endif ()
if ( CMAKE_COMPILER_IS_GNUCXX )
  if (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 3.1)
    list(APPEND CMAKE_CXX_FLAGS "-Wall")
  else (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 3.1)
    list(APPEND CMAKE_CXX_FLAGS "-std=c++11 -Wall")
  endif (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 3.1)
endif ( CMAKE_COMPILER_IS_GNUCXX )




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
if(NOT SDDS_LIBRARY)
  message("WARNING: libSDDS1 not found! Compile libpalattice without SDDS support.\nPlease have a look at README file for details.")
  remove_definitions(-DLIBPALATTICE_USE_SDDS_TOOLKIT_LIBRARY)
else(NOT SDDS_LIBRARY)
  message("libSDDS1 found! Compile libpalattice with SDDS support.")
  add_definitions(-DLIBPALATTICE_USE_SDDS_TOOLKIT_LIBRARY)
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
endif(NOT SDDS_LIBRARY)





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
  ResStrengths.cpp
  ELSASpuren.cpp
  )
SET_TARGET_PROPERTIES(
  palattice
  PROPERTIES
  VERSION ${LIB_VERSION_MAJOR}
  SOVERSION ${LIB_VERSION}
  )


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



# automatically generated header files (by cmake, not build-time!)
configure_file(simToolPath.hpp.in ${CMAKE_SOURCE_DIR}/simToolPath.hpp ESCAPE_QUOTES)



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




# install
install(TARGETS palattice
  LIBRARY DESTINATION lib
  )

install(PROGRAMS simTools/elegant2libpalattice.sh
  DESTINATION bin
  RENAME elegant2libpalattice
  )

install(FILES
  AccElements.hpp
  AccLattice.hpp
  Metadata.hpp
  SimTools.hpp
  Interpolate.hpp
  Interpolate.hxx
  FunctionOfPos.hpp
  FunctionOfPos.hxx
  Field.hpp
  Spectrum.hpp
  ResStrengths.hpp
  ELSASpuren.hpp
  types.hpp
  config.hpp
  gitversion.hpp
  simToolPath.hpp
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


# programs, including convertlattice
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/programs)

# some tests with googletest
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tests)

