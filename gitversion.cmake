cmake_minimum_required(VERSION 2.8)

find_package(Git)

execute_process(
  COMMAND ${GIT_EXECUTABLE} log -n 1 --date=iso --pretty=format:"%h from %ad"
  OUTPUT_VARIABLE GIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )

configure_file(${SRC} ${DST} ESCAPE_QUOTES)