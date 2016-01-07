# this is executed via cmake in libpalattice/ (..) as subdirectory
# programs are not compiled with 'make all', because they
# are linked against INSTALLED libpalattice.
# use 'make programs' followed by another 'make install'

# cmake Version
cmake_minimum_required(VERSION 2.8)

project(palattice-programs CXX)
set(CMAKE_BUILD_TYPE Debug)


# programs are linked against sytemwide installed libpalattice
# so usage is exactly as with user programs
# and convertlattice can be used systemwide independant of build path
find_library(PALATTICE_INSTALLED_LIBRARY palattice)
if(NOT PALATTICE_INSTALLED_LIBRARY)
   message("program targets: palattice library not installed yet!\nUse 'sudo make install' first and then 'make programs' again")
else()

# build
add_executable(lattice EXCLUDE_FROM_ALL lattice.cpp)
add_executable(convertlattice EXCLUDE_FROM_ALL convertlattice.cpp)
add_executable(interptwiss EXCLUDE_FROM_ALL interptwiss.cpp)
add_executable(fields EXCLUDE_FROM_ALL fields.cpp)


# link
target_link_libraries(lattice ${PALATTICE_INSTALLED_LIBRARY} ${Z_LIBRARY})
target_link_libraries(convertlattice ${PALATTICE_INSTALLED_LIBRARY} ${Z_LIBRARY})
target_link_libraries(interptwiss ${PALATTICE_INSTALLED_LIBRARY} ${GSL_LIBRARY} ${GSLCBLAS_LIBRARY} ${Z_LIBRARY})
target_link_libraries(fields ${PALATTICE_INSTALLED_LIBRARY} ${GSL_LIBRARY} ${GSLCBLAS_LIBRARY} ${Z_LIBRARY})


# target 'programs'
add_custom_target(programs
  DEPENDS lattice convertlattice interptwiss fields
  )


# install convertlattice, if existing
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/convertlattice
  DESTINATION bin
  OPTIONAL
  )

endif()