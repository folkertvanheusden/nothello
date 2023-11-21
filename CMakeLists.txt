cmake_minimum_required(VERSION 3.7)

project(nothello)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_BUILD_TYPE Debug)

add_compile_options(-Wall)

add_executable(
  nothello-test
  board.cpp
  test.cpp
)

add_executable(
  nothello
  board.cpp
  game.cpp
  random.cpp
)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads)
target_link_libraries(nothello Threads::Threads)

include(FindPkgConfig)

pkg_check_modules(READLINE REQUIRED readline)
target_link_libraries(nothello ${READLINE_LIBRARIES})
target_include_directories(nothello PUBLIC ${READLINE_INCLUDE_DIRS})
target_compile_options(nothello PUBLIC ${READLINE_CFLAGS_OTHER})
