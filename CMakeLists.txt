cmake_minimum_required(VERSION 3.10)
project(rsa-test CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(rsa-test
    main.cpp
    src/rsa.cpp
)

# Include directories
# target_include_directories(rsa-test PRIVATE)

# Link libraries
target_link_libraries(rsa-test PRIVATE
  gmp
  gmpxx
  pthread
)
