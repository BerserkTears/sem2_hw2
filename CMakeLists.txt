cmake_minimum_required(VERSION 3.20)
project(hw2_sem2)

set(CMAKE_CXX_STANDARD 14)

#libcurl
include_directories(/opt/homebrew/opt/curl/include)
link_directories(/opt/homebrew/opt/curl/lib)

include(FetchContent)
#библиотека для работы с json(nlohmann)
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.10.5/json.tar.xz)
FetchContent_MakeAvailable(json)


add_executable(hw2_sem2 main.cpp)

target_link_libraries(hw2_sem2 nlohmann_json::nlohmann_json)
target_link_libraries(hw2_sem2 curl)