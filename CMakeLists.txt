cmake_minimum_required(VERSION 3.7)

project(samei)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_BUILD_TYPE RelWithDebInfo)
#set(CMAKE_BUILD_TYPE Debug)

cmake_policy(SET CMP0069 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

add_compile_options(-Wall)

add_executable(
  samei-test
  board.cpp
  str.cpp
  test.cpp
)

add_executable(
  samei
  board.cpp
  game.cpp
  random.cpp
  search.cpp
  str.cpp
  time.cpp
  ugi.cpp
)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads)
target_link_libraries(samei Threads::Threads)

include(FindPkgConfig)

pkg_check_modules(READLINE REQUIRED readline)
target_link_libraries(samei ${READLINE_LIBRARIES})
target_include_directories(samei PUBLIC ${READLINE_INCLUDE_DIRS})
target_compile_options(samei PUBLIC ${READLINE_CFLAGS_OTHER})
