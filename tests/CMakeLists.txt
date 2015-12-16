# this is executed via cmake in libpalattice/ (..) as subdirectory


# compile and use googletest
if(EXISTS ${GTEST_PATH})
  
  enable_testing()
  add_subdirectory(${GTEST_PATH} gtest)
  include_directories(${GTEST_PATH} ${GTEST_PATH}/include)

  # build
  add_executable(test-syli test-syli.cpp)
  add_executable(test-sdds test-sdds.cpp)
    
  # link
  target_link_libraries(test-syli palattice ${Z_LIBRARY} gtest)
  target_link_libraries(test-sdds palattice ${Z_LIBRARY} gtest)
  target_link_libraries(test-sdds ${SDDS_LIBRARY} ${MDBCOMMON_LIBRARY} ${MDB_LIBRARY} ${LZMA_LIBRARY})
  
  add_test(allTests test-syli)
  add_test(allTests test-sdds)
  
endif()