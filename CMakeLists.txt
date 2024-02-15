cmake_minimum_required(VERSION 3.0)
include(FetchContent)

project(hornyVirus)
set(CMAKE_SYSTEM_NAME Windows)

set(CMAKE_SYSTEM_VERSION 10)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(Optimize "Default" CACHE STRING "Optimization choice")

if($CACHE{Optimize} STREQUAL "Most")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")
elseif($CACHE{Optimize} STREQUAL "More")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
elseif($CACHE{Optimize} STREQUAL "Less")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O1")
elseif($CACHE{Optimize} STREQUAL "No")
else()
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
endif()

set(SOURCES
    main.cpp
)
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )
find_package(CURL REQUIRED)


find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)
link_libraries(${JSONCPP_LIBRARIES})

set(CMAKE_BUILD_TYPE Debug)


add_executable(Virus.exe ${SOURCES})

target_compile_features(Virus.exe PRIVATE cxx_std_23)

target_link_libraries(Virus.exe CURL::libcurl jsoncpp  ${OpenCV_LIBS}  ) 
