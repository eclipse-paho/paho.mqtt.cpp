# CMakeLists.txt
#
# Top-level CMake file for the Paho C++ library.
#
#*******************************************************************************
# This is part of the Paho MQTT C++ client library.
#
# Copyright (c) 2017-2025, Frank Pagliughi
# Copyright (c) 2016-2017, Guilherme Maciel Ferreira
#
# All rights reserved. This program and the accompanying materials
# are made available under the terms of the Eclipse Public License v2.0
# and Eclipse Distribution License v1.0 which accompany this distribution.
# 
# The Eclipse Public License is available at
#   http://www.eclipse.org/legal/epl-v20.html
# and the Eclipse Distribution License is available at
#   http://www.eclipse.org/org/documents/edl-v10.php.
# 
# Contributors:
#   Guilherme Maciel Ferreira - initial version
#   Frank Pagliughi
#*******************************************************************************/

cmake_minimum_required(VERSION 3.13)

project(PahoMqttCpp VERSION "1.5.3")

## --- Build options ---

if(WIN32)
    option(PAHO_BUILD_STATIC "Build static library" TRUE)
    option(PAHO_BUILD_SHARED "Build shared library (DLL)" FALSE)
    option(PAHO_WITH_SSL "Build SSL-enabled library" FALSE)
else()
    option(PAHO_BUILD_STATIC "Build static library" FALSE)
    option(PAHO_BUILD_SHARED "Build shared library" TRUE)
    option(PAHO_WITH_SSL "Build SSL-enabled library" TRUE)
    option(PAHO_BUILD_DEB_PACKAGE "Build debian package" FALSE)
endif()

option(PAHO_BUILD_SAMPLES "Build sample/example programs" FALSE)
option(PAHO_BUILD_EXAMPLES "Build sample/example programs" FALSE)
option(PAHO_BUILD_TESTS "Build tests (requires Catch2)" FALSE)
option(PAHO_BUILD_DOCUMENTATION "Create and install the API documentation (requires Doxygen)" FALSE)
option(PAHO_WITH_MQTT_C "Build Paho C from the internal GIT submodule." FALSE)

if(NOT PAHO_BUILD_SHARED AND NOT PAHO_BUILD_STATIC)
    message(FATAL_ERROR "You must set either PAHO_BUILD_SHARED, PAHO_BUILD_STATIC, or both")
endif()

# --- Setting naming variables ---

set(PAHO_MQTTPP_GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)

## --- Find Paho C or build it, if reqested ---

if(PAHO_WITH_SSL)
    find_package(OpenSSL REQUIRED)
    set(PAHO_MQTT_C_LIB eclipse-paho-mqtt-c::paho-mqtt3as)
else()
    set(PAHO_MQTT_C_LIB eclipse-paho-mqtt-c::paho-mqtt3a)
endif()

if(PAHO_WITH_MQTT_C)
    message(STATUS "Paho C: Bundled")

    ## Build the Paho C library from the submodule
    set(PAHO_ENABLE_TESTING FALSE CACHE BOOL "No Paho C tests")
    set(PAHO_HIGH_PERFORMANCE TRUE CACHE BOOL "Paho C high performance")
    if(NOT WIN32)
        set(PAHO_WITH_UNIX_SOCKETS TRUE CACHE BOOL "Support for Unix-domain sockets")
    endif()
    set(CMAKE_C_STANDARD 99 CACHE STRING "Paho C language standard")

    add_subdirectory(${PROJECT_SOURCE_DIR}/externals/paho-mqtt-c)

    ## Alias namespace so that the full names can be used with the subdir.
    if(PAHO_BUILD_SHARED)
        add_library(eclipse-paho-mqtt-c::paho-mqtt3a ALIAS paho-mqtt3a)
        list(APPEND PAHO_MQTT_C_LIBS paho-mqtt3a)
        if(PAHO_WITH_SSL)
            add_library(eclipse-paho-mqtt-c::paho-mqtt3as ALIAS paho-mqtt3as)
            list(APPEND PAHO_MQTT_C_LIBS paho-mqtt3as)
        endif()
    endif()

    if(PAHO_BUILD_STATIC)
        add_library(eclipse-paho-mqtt-c::paho-mqtt3a-static ALIAS paho-mqtt3a-static)
        list(APPEND PAHO_MQTT_C_LIBS paho-mqtt3a-static)
        if(PAHO_WITH_SSL)
            add_library(eclipse-paho-mqtt-c::paho-mqtt3as-static ALIAS paho-mqtt3as-static)
            list(APPEND PAHO_MQTT_C_LIBS paho-mqtt3as-static)
        endif()
    endif()

    ## install paho.mqtt.c library (appending to PahoMqttCpp export)
    install(TARGETS ${PAHO_MQTT_C_LIBS}
        EXPORT PahoMqttCpp
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
else()
    find_package(eclipse-paho-mqtt-c REQUIRED)
endif()

# --- System Details ---

include(GNUInstallDirs)

if(WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    set(LIBS_SYSTEM ws2_32)
endif()

# --- The headers ---

add_subdirectory(include/mqtt)

# For the paho_mqtt_c module
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
add_subdirectory(src)

# --- Documentation ---

if(PAHO_BUILD_DOCUMENTATION)
    add_subdirectory(doc)
endif()

# --- Example Apps ---

if(PAHO_BUILD_SAMPLES OR PAHO_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# --- Unit Tests ---

if(PAHO_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test/unit)
endif()

## --- Install generated header(s) ---

install(
    DIRECTORY
        ${PAHO_MQTTPP_GENERATED_DIR}/include/
    DESTINATION
        ${CMAKE_INSTALL_INCLUDEDIR}
)

## --- Packaging settings ---

if(WIN32)
    set(CPACK_GENERATOR "ZIP")
elseif(UNIX)
    if(PAHO_BUILD_DEB_PACKAGE)
        set(CPACK_GENERATOR "DEB")
        include(cmake/CPackDebConfig.cmake)
    else()
        set(CPACK_GENERATOR "TGZ")
	endif()
endif()

include(CPack)

# --- Export CMake TARGETS ---

add_subdirectory(cmake)

