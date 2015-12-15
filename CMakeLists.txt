# cmake Version
cmake_minimum_required(VERSION 2.8)

project(palattice CXX)
set(CMAKE_BUILD_TYPE Debug)

# Project Version
set(LIB_VERSION_MAJOR 3.7)
set(LIB_VERSION_MINOR 2)
set(LIB_VERSION ${LIB_VERSION_MAJOR}.${LIB_VERSION_MINOR})

# googletest path (used in tests/CMakeLists.txt)
set(GTEST_PATH /home/thiry/dev/gtest-1.7.0/)

# Install Path
set(INSTALL_PATH "/usr/local")
set(SIMTOOL_PATH ${INSTALL_PATH}/lib/libpalattice_simTools)

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



# gtest mitbauen
# set(GTEST_PATH /sgt/gtest/googletest)
# add_subdirectory(${GTEST_PATH} gtest)
# testing innerhalb cmake aktivieren
#enable_testing()



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
find_library(MDBCOMMON_LIBRARY mdbcommon)
find_library(MDB_LIBRARY mdblib)
find_library(LZMA_LIBRARY lzma)
if(NOT SDDS_LIBRARY OR NOT MDBCOMMON_LIBRARY OR NOT MDB_LIBRARY OR NOT LZMA_LIBRARY)
  message(FATAL_ERROR "libSDDS1 not found! Please have a look at README file")
endif()

find_library(Z_LIBRARY z)
if(NOT Z_LIBRARY)
  message(FATAL_ERROR "libz not found!")
endif()



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

# automatically generated header files
execute_process(
  COMMAND git log -n 1 --date=iso --pretty=format:"%h from %ad"
  OUTPUT_VARIABLE GIT_HASH
  )
configure_file(gitversion.template ${CMAKE_SOURCE_DIR}/gitversion.hpp ESCAPE_QUOTES)
configure_file(simToolPath.template ${CMAKE_SOURCE_DIR}/simToolPath.hpp ESCAPE_QUOTES)



# link 'libpalattice'
target_link_libraries(
  palattice ${GSL_LIBRARY}
  ${GSLCBLAS_LIBRARY}
  ${SDDS_LIBRARY}
  ${MDBCOMMON_LIBRARY}
  ${MDB_LIBRARY}
  ${LZMA_LIBRARY}
  ${Z_LIBRARY}
  )



# add_executable(test-resocalc test-resocalc.cpp)
# target_link_libraries(test-resocalc resocalc)
# target_link_libraries(test-resocalc gtest)
# # zusätzliche include-pfade angeben
# target_include_directories(test-resocalc PUBLIC ${GTEST_PATH})
# target_include_directories(test-resocalc PUBLIC ${GTEST_PATH}/include)
# # 'test-resocalc' als test definieren
# add_test(test-resocalc test-resocalc)
# if (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 3.1)
#   set_property(TARGET test-resocalc PROPERTY CXX_STANDARD 11)
# endif (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 3.1)




# install
install(TARGETS palattice
  LIBRARY DESTINATION ${INSTALL_PATH}/lib
  )

install (PROGRAMS simTools/elegant2libpalattice.sh
  DESTINATION ${INSTALL_PATH}/bin
  RENAME elegant2libpalattice
  )

install (FILES
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
  DESTINATION ${INSTALL_PATH}/include/libpalattice
  )

install (FILES
  simTools/libpalattice.ele
  simTools/libpalattice.madx
  simTools/libpalattice_twissonly.madx
  DESTINATION ${SIMTOOL_PATH}
  )



# programs, including convertlattice
add_subdirectory(${CMAKE_SOURCE_DIR}/programs)

# googletests
add_subdirectory(${CMAKE_SOURCE_DIR}/tests)