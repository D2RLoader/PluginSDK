cmake_minimum_required(VERSION 3.28)

project(D2RLPluginSDK VERSION 0.1.0 LANGUAGES CXX RC)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

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
	"${CMAKE_CURRENT_BINARY_DIR}/D2RLPluginConfigVersion.cmake"
	DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/D2RLPlugin"
)

if(NOT D2RLPLUGIN_BUILD_EXAMPLES)
	return()
endif()

if(NOT DEFINED D2RLOADER_BIN_DIR_DEBUG)
	set(D2RLOADER_BIN_DIR_DEBUG "${CMAKE_BINARY_DIR}/bin/debug")
endif()

if(NOT DEFINED D2RLOADER_BIN_DIR_RELEASE)
	set(D2RLOADER_BIN_DIR_RELEASE "${CMAKE_BINARY_DIR}/bin/release")
endif()

set(D2RLPLUGIN_SAMPLE_PLUGINS_DIR_DEBUG "${D2RLOADER_BIN_DIR_DEBUG}/d2rloader/plugins")
set(D2RLPLUGIN_SAMPLE_PLUGINS_DIR_RELEASE "${D2RLOADER_BIN_DIR_RELEASE}/d2rloader/plugins")
set(D2RLPLUGIN_SAMPLE_PATCHES_DIR_DEBUG "${D2RLOADER_BIN_DIR_DEBUG}/d2rloader/patches")
set(D2RLPLUGIN_SAMPLE_PATCHES_DIR_RELEASE "${D2RLOADER_BIN_DIR_RELEASE}/d2rloader/patches")
set(D2RLPLUGIN_SAMPLE_CONFIG_DIR_DEBUG "${D2RLOADER_BIN_DIR_DEBUG}/d2rloader/config")
set(D2RLPLUGIN_SAMPLE_CONFIG_DIR_RELEASE "${D2RLOADER_BIN_DIR_RELEASE}/d2rloader/config")

function(d2rlplugin_add_sample target output_name)
	set(exclude_from_all)
	if(NOT D2RLPLUGIN_BUILD_EXAMPLES_BY_DEFAULT)
		set(exclude_from_all EXCLUDE_FROM_ALL)
	endif()

	add_library(${target} SHARED ${exclude_from_all} ${ARGN})
	target_link_libraries(${target} PRIVATE D2RLPlugin::D2RLPlugin)
	set_target_properties(${target} PROPERTIES
		CXX_STANDARD 20
		CXX_STANDARD_REQUIRED TRUE
		CXX_EXTENSIONS FALSE
		OUTPUT_NAME "${output_name}"
		RUNTIME_OUTPUT_DIRECTORY_DEBUG "${D2RLPLUGIN_SAMPLE_PLUGINS_DIR_DEBUG}"
		RUNTIME_OUTPUT_DIRECTORY_RELEASE "${D2RLPLUGIN_SAMPLE_PLUGINS_DIR_RELEASE}"
		LIBRARY_OUTPUT_DIRECTORY_DEBUG "${D2RLPLUGIN_SAMPLE_PLUGINS_DIR_DEBUG}"
		LIBRARY_OUTPUT_DIRECTORY_RELEASE "${D2RLPLUGIN_SAMPLE_PLUGINS_DIR_RELEASE}"
		ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/build/lib/debug/samples"
		ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/build/lib/release/samples"
	)

	if(MSVC)
		target_compile_options(${target} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:/W3 /permissive- /EHsc>)
	endif()
endfunction()

d2rlplugin_add_sample(D2RLHelloConsolePlugin "plugin-d2rl-hello-console"
	examples/hello-console/hello_console_plugin.cpp
	examples/hello-console/hello_console_plugin.rc
)

d2rlplugin_add_sample(D2RLConfigFileSamplePlugin "plugin-d2rl-config-file-sample"
	examples/config-file/config_file_plugin.cpp
	examples/config-file/config_file_plugin.rc
)

d2rlplugin_add_sample(D2RLPatchingSamplePlugin "plugin-d2rl-patching-sample"
	examples/patching/patching_sample_plugin.cpp
	examples/patching/patching_sample_plugin.rc
)

set(D2RLPLUGIN_CONFIG_FILE_SAMPLE_CONFIG_OUTPUT_DIR "$<IF:$<CONFIG:Debug>,${D2RLPLUGIN_SAMPLE_CONFIG_DIR_DEBUG},${D2RLPLUGIN_SAMPLE_CONFIG_DIR_RELEASE}>")
add_custom_command(TARGET D2RLConfigFileSamplePlugin POST_BUILD
	COMMAND "${CMAKE_COMMAND}" -E make_directory "${D2RLPLUGIN_CONFIG_FILE_SAMPLE_CONFIG_OUTPUT_DIR}"
	COMMAND "${CMAKE_COMMAND}" -E copy_if_different
		"${CMAKE_CURRENT_SOURCE_DIR}/examples/config-file/config-file-sample.toml"
		"${D2RLPLUGIN_CONFIG_FILE_SAMPLE_CONFIG_OUTPUT_DIR}/config-file-sample.toml"
	COMMENT "Copying D2RL config file sample"
	VERBATIM
)

set(D2RLPLUGIN_PATCHING_SAMPLE_PATCH_OUTPUT_DIR "$<IF:$<CONFIG:Debug>,${D2RLPLUGIN_SAMPLE_PATCHES_DIR_DEBUG},${D2RLPLUGIN_SAMPLE_PATCHES_DIR_RELEASE}>")
set(D2RLPLUGIN_PATCHING_SAMPLE_CONFIG_OUTPUT_DIR "$<IF:$<CONFIG:Debug>,${D2RLPLUGIN_SAMPLE_CONFIG_DIR_DEBUG},${D2RLPLUGIN_SAMPLE_CONFIG_DIR_RELEASE}>")
add_custom_command(TARGET D2RLPatchingSamplePlugin POST_BUILD
	COMMAND "${CMAKE_COMMAND}" -E make_directory "${D2RLPLUGIN_PATCHING_SAMPLE_PATCH_OUTPUT_DIR}"
	COMMAND "${CMAKE_COMMAND}" -E make_directory "${D2RLPLUGIN_PATCHING_SAMPLE_CONFIG_OUTPUT_DIR}"
	COMMAND "${CMAKE_COMMAND}" -E copy_if_different
		"${CMAKE_CURRENT_SOURCE_DIR}/examples/json-patches/example-patches.json"
		"${D2RLPLUGIN_PATCHING_SAMPLE_PATCH_OUTPUT_DIR}/example-patches.json"
	COMMAND "${CMAKE_COMMAND}" -E copy_if_different
		"${CMAKE_CURRENT_SOURCE_DIR}/examples/patching/patching-sample.toml"
		"${D2RLPLUGIN_PATCHING_SAMPLE_CONFIG_OUTPUT_DIR}/patching-sample.toml"
	COMMENT "Copying D2RL patching sample files"
	VERBATIM
)
