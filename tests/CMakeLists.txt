# this is executed via cmake in libpalattice/ (..) as subdirectory


# compile and use googletest
if(EXISTS ${GTEST_PATH})
  
  enable_testing()
  add_subdirectory(${GTEST_PATH} gtest)
  include_directories(${GTEST_PATH} ${GTEST_PATH}/include)

  add_definitions(-DTEST_PARAM_FILE="${CMAKE_CURRENT_SOURCE_DIR}/test-sdds.param")
  add_definitions(-DTEST_TWISS_FILE="${CMAKE_CURRENT_SOURCE_DIR}/test-sdds.twi")
  add_definitions(-DTEST_ORBIT_FILE="${CMAKE_CURRENT_SOURCE_DIR}/test-sdds.clo")
  add_definitions(-DTEST_WATCH_FILE="${CMAKE_CURRENT_SOURCE_DIR}/test-sdds.w")
  add_definitions(-DTEST_LATTICE_FILE="${CMAKE_CURRENT_SOURCE_DIR}/test-sdds.lte")

  # build
  add_executable(test-syli test-syli.cpp)
  add_executable(test-sdds test-sdds.cpp)
    
  # link
  target_link_libraries(test-syli palattice ${Z_LIBRARY} gtest)
  target_link_libraries(test-sdds palattice ${Z_LIBRARY} gtest)
  
  if(LIBPALATTICE_USE_SDDS_TOOLKIT_LIBRARY)
    target_link_libraries(test-sdds ${SDDS_LIBRARY} ${MDBCOMMON_LIBRARY} ${MDB_LIBRARY} ${LZMA_LIBRARY})
  endif()
  
  add_test(allTests test-syli)
  add_test(allTests test-sdds)
  
endif()