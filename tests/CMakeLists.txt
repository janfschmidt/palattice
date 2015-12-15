# this is executed via cmake in libpalattice/ (..) as subdirectory


# compile and use googletest

enable_testing()
add_subdirectory(${GTEST_PATH} gtest)
include_directories(${GTEST_PATH} ${GTEST_PATH}/include)

 #ExternalProject_add() ??


# build
add_executable(test-syli test-syli.cpp)
add_executable(test-sdds test-sdds.cpp)


# link
target_link_libraries(test-syli palattice ${Z_LIBRARY} gtest)
target_link_libraries(test-sdds palattice ${Z_LIBRARY} gtest)
target_link_libraries(test-sdds ${SDDS_LIBRARY} ${MDBCOMMON_LIBRARY} ${MDB_LIBRARY} ${LZMA_LIBRARY})

# target_include_directories(test-syli ${GTEST_PATH} ${GTEST_PATH}/include)
# target_include_directories(test-sdds ${GTEST_PATH} ${GTEST_PATH}/include)


add_test(allTests test-syli)
add_test(allTests test-sdds)