cmake_minimum_required(VERSION 3.15)

project(minitar
    VERSION 0.0.1
    DESCRIPTION "minitar")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

if(NOT DEFINED ENV{BUILD_SHARED_LIBS})
    set(OPTION_BUILD_SHARED_LIBS OFF)
else()
    set(OPTION_BUILD_SHARED_LIBS $ENV{BUILD_SHARED_LIBS})
endif()
option(BUILD_SHARED_LIBS "Build using shared libraries"
    ${OPTION_BUILD_SHARED_LIBS})

add_subdirectory(src)

# setup installer
include(InstallRequiredSystemLibraries)
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_PACKAGE_VERSION_MAJOR "${minitar_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${minitar_VERSION_MINOR}")
include(CPack)

install(EXPORT minitarTargets
  FILE minitarTargets.cmake
  DESTINATION share/cmake/minitar
)

include(CMakePackageConfigHelpers)
# the config file that includes the exports
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/minitarConfig.cmake"
  INSTALL_DESTINATION "share/cmake/minitar"
  )

# version file for the config file
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/minitarConfigVersion.cmake"
  VERSION "${minitar_VERSION_MAJOR}.${minitar_VERSION_MINOR}.${minitar_VERSION_PATCH}"
  COMPATIBILITY AnyNewerVersion
)

# install the configuration file
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/minitarConfig.cmake
  DESTINATION share/cmake/minitar
  )

export(EXPORT minitarTargets
  FILE "${CMAKE_CURRENT_BINARY_DIR}/minitarTargets.cmake"
)

