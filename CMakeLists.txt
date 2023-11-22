cmake_minimum_required(VERSION 3.7)

project(nothello)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_BUILD_TYPE RelWithDebInfo)
#set(CMAKE_BUILD_TYPE Debug)

cmake_policy(SET CMP0069 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

add_compile_options(-Wall)

add_executable(
  nothello-test
  board.cpp
  str.cpp
  test.cpp
)

add_executable(
  nothello
  board.cpp
  game.cpp
  playout.cpp
  random.cpp
  str.cpp
  time.cpp
  uct.cpp
  ugi.cpp
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
