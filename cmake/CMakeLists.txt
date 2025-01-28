# CMakeLists.txt
#
# CMake export file for the Paho C++ library.
#
#*******************************************************************************
# This is part of the Paho MQTT C++ client library.
#
# Copyright (c) 2017-2023, Frank Pagliughi
#
# All rights reserved. This program and the accompanying materials
# are made available under the terms of the Eclipse Public License v2.0
# and Eclipse Distribution License v1.0 which accompany this distribution.
#
# The Eclipse Public License is available at
#   http://www.eclipse.org/legal/epl-v20.html
# and the Eclipse Distribution License is available at
#   http://www.eclipse.org/org/documents/edl-v10.php.
#*******************************************************************************/

set(package_name PahoMqttCpp)
configure_file(${package_name}Config.cmake.in ${package_name}Config.cmake @ONLY)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/${package_name}ConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

export(EXPORT ${package_name}
  FILE "${CMAKE_CURRENT_BINARY_DIR}/${package_name}Targets.cmake"
  NAMESPACE ${package_name}::
)

install(EXPORT ${package_name}
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${package_name}
  FILE ${package_name}Targets.cmake
  NAMESPACE ${package_name}::
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/${package_name}Config.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/${package_name}ConfigVersion.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${package_name}
)
