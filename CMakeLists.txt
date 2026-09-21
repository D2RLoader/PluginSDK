cmake_minimum_required(VERSION 3.29)

# C++ and CMake share the SDK release number from version.h.
set(D2RLPLUGIN_VERSION_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/include/D2RLPlugin/version.h")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${D2RLPLUGIN_VERSION_HEADER}")
file(STRINGS "${D2RLPLUGIN_VERSION_HEADER}" D2RLPLUGIN_VERSION_LINE REGEX "^#define[ \t]+D2RL_SDK_VERSION[ \t]+")
list(LENGTH D2RLPLUGIN_VERSION_LINE D2RLPLUGIN_VERSION_COUNT)
if(NOT D2RLPLUGIN_VERSION_COUNT EQUAL 1)
	message(FATAL_ERROR "version.h must define D2RL_SDK_VERSION exactly once.")
endif()
if(NOT D2RLPLUGIN_VERSION_LINE MATCHES "^#define[ \t]+D2RL_SDK_VERSION[ \t]+\"([0-9]+\\.[0-9]+\\.[0-9]+)\"[ \t]*$")
	message(FATAL_ERROR "Set D2RL_SDK_VERSION to a quoted MAJOR.MINOR.PATCH value, such as \"0.3.0\".")
endif()
set(D2RLPLUGIN_SDK_VERSION "${CMAKE_MATCH_1}")

project(D2RLPluginSDK VERSION "${D2RLPLUGIN_SDK_VERSION}" LANGUAGES CXX RC)

include(CTest)
include(GNUInstallDirs)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
set(CMAKE_CXX_EXTENSIONS OFF)
include(CMakePackageConfigHelpers)
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/D2RLPluginEmbedConfig.cmake")

if(PROJECT_IS_TOP_LEVEL)
	set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

option(D2RLPLUGIN_BUILD_EXAMPLES "Build D2RLoader plugin SDK examples." ON)
option(D2RLPLUGIN_BUILD_EXAMPLES_BY_DEFAULT "Build D2RLoader plugin SDK examples as part of the default build." ${PROJECT_IS_TOP_LEVEL})

add_library(D2RLPlugin INTERFACE)
add_library(D2RLPlugin::D2RLPlugin ALIAS D2RLPlugin)

target_include_directories(D2RLPlugin INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_compile_features(D2RLPlugin INTERFACE cxx_std_20)

write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/D2RLPluginConfigVersion.cmake"
	VERSION "${PROJECT_VERSION}"
	COMPATIBILITY SameMajorVersion
)

install(TARGETS D2RLPlugin EXPORT D2RLPluginTargets)
install(DIRECTORY include/ DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(FILES LICENSE DESTINATION "${CMAKE_INSTALL_DATADIR}/licenses/D2RLPlugin")
install(EXPORT D2RLPluginTargets
	NAMESPACE D2RLPlugin::
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/D2RLPlugin"
)
install(FILES
	cmake/D2RLPluginConfig.cmake
	cmake/D2RLPluginEmbedConfig.cmake
	cmake/D2RLPluginConfigResource.rc.in
	"${CMAKE_CURRENT_BINARY_DIR}/D2RLPluginConfigVersion.cmake"
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/D2RLPlugin"
)

if(BUILD_TESTING)
	add_subdirectory(tests)
endif()

if(D2RLPLUGIN_BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()
