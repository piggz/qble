cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(qble STATIC)

# Make sure to choose same Qt version as rest of project
if(TARGET Qt6::Core)
    set(QT_VERSION_MAJOR 6)
    find_package(Qt6 REQUIRED COMPONENTS Core DBus Xml)
elseif(TARGET Qt5::Core)
    set(QT_VERSION_MAJOR 5)
    find_package(Qt5 REQUIRED COMPONENTS Core DBus Xml)
else()
    # Use Qt6 first if available
    find_package(Qt6 COMPONENTS Core DBus Xml QUIET)
    if(Qt6_FOUND)
        set(QT_VERSION_MAJOR 6)
    else()
        find_package(Qt5 REQUIRED COMPONENTS Core DBus Xml)
        set(QT_VERSION_MAJOR 5)
    endif()
endif()

target_include_directories(qble PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(qble PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Xml
)

set(SOURCES
    adaptermodel.cpp
    bluezadapter.cpp
    qbleagent.cpp
    qblecharacteristic.cpp
    qbledescriptor.cpp
    qbledevice.cpp
    qbleservice.cpp
)

set(HEADERS
    adaptermodel.h
    bluezadapter.h
    qbleagent.h
    qblecharacteristic.h
    qbledescriptor.h
    qbledevice.h
    qbleservice.h
)

target_sources(qble PUBLIC ${SOURCES} ${HEADERS})
