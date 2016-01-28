# this cmake project can also be executed via cmake in ../ (libpalattice) as ExternalProject

# cmake Version
cmake_minimum_required(VERSION 2.8)

project(palattice-programs CXX)
set(CMAKE_BUILD_TYPE Debug)

# Install Path
set(CMAKE_INSTALL_PREFIX "/usr/local")


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
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")



# programs are linked against sytemwide installed libpalattice
# so usage is exactly as with user programs
# and convertlattice can be used systemwide independant of build path
find_library(PALATTICE_INSTALLED_LIBRARY palattice)
if(NOT PALATTICE_INSTALLED_LIBRARY)
  message(FATAL_ERROR "palattice library not found! Please install it first.
Programs are linked against sytemwide installed libpalattice. So usage is exactly as with user programs and convertlattice can be used systemwide independant of build path.")
endif()

find_library(GSL_LIBRARY gsl)
find_library(GSLCBLAS_LIBRARY gslcblas)
if(NOT GSL_LIBRARY OR NOT GSLCBLAS_LIBRARY)
  message(FATAL_ERROR "Gnu Scientific Library GSL not found!")
endif()


# build
add_executable(lattice lattice.cpp)
add_executable(convertlattice convertlattice.cpp)
add_executable(interptwiss interptwiss.cpp)
add_executable(fields fields.cpp)


# link
target_link_libraries(lattice ${PALATTICE_INSTALLED_LIBRARY} ${Z_LIBRARY})
target_link_libraries(convertlattice ${PALATTICE_INSTALLED_LIBRARY} ${Z_LIBRARY})
target_link_libraries(interptwiss ${PALATTICE_INSTALLED_LIBRARY} ${GSL_LIBRARY} ${GSLCBLAS_LIBRARY} ${Z_LIBRARY})
target_link_libraries(fields ${PALATTICE_INSTALLED_LIBRARY} ${GSL_LIBRARY} ${GSLCBLAS_LIBRARY} ${Z_LIBRARY})



# install convertlattice
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/convertlattice
  DESTINATION bin
  )

