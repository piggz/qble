cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(qble STATIC)

find_package(Qt5 REQUIRED COMPONENTS Core DBus Xml)

target_include_directories(qble PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(qble PUBLIC Qt5::Core Qt5::DBus Qt5::Xml)

set(SOURCES
    adaptermodel.cpp
    bluezadapter.cpp
    qblecharacteristic.cpp
    qbledescriptor.cpp
    qbledevice.cpp
    qbleservice.cpp
)

set(HEADERS
    adaptermodel.h
    bluezadapter.h
    qblecharacteristic.h
    qbledescriptor.h
    qbledevice.h
    qbleservice.h
)

target_sources(qble PUBLIC ${SOURCES} ${HEADERS})

