cmake_minimum_required(VERSION 3.0.0)
project(kcptest VERSION 0.1.0)

include(CTest)
enable_testing()
include(/home/vcpkg/scripts/buildsystems/vcpkg.cmake)

add_executable(kcptest main.cpp)
find_package(kcp CONFIG REQUIRED)
target_link_libraries(kcptest PRIVATE kcp::kcp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
