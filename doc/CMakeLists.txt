#*******************************************************************************
# Copyright (c) 2024, Frank Pagliughi
# Copyright (c) 2016, Guilherme Maciel Ferreira
# 
#  All rights reserved. This program and the accompanying materials
#  are made available under the terms of the Eclipse Public License v2.0
#  and Eclipse Distribution License v1.0 which accompany this distribution. 
# 
#  The Eclipse Public License is available at 
#     http://www.eclipse.org/legal/epl-v20.html
#  and the Eclipse Distribution License is available at 
#    http://www.eclipse.org/org/documents/edl-v10.php.
# 
#  Contributors:
#     Frank Pagliughi - Updated paths. Fixed conflict with Paho C
#     Guilherme Maciel Ferreira - initial version
#*******************************************************************************/

## documentation settings
find_package(Doxygen REQUIRED)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc)

message(STATUS "Doxygen: ${DOXYGEN_EXECUTABLE}")

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    @ONLY
)

add_custom_target(
    paho-mqttpp3-doc.target
    COMMAND ${DOXYGEN_EXECUTABLE} Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM
)

add_custom_target(paho-mqttpp3-doc ALL 
    DEPENDS paho-mqttpp3-doc.target
)

install(
    DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc/
    DESTINATION share/doc/EclipsePahoCpp
)

