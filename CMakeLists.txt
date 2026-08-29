cmake_minimum_required(VERSION 3.28)

project(D2RLPluginSDK VERSION 0.1.0 LANGUAGES CXX RC)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/D2RLPluginEmbedConfig.cmake")

if(PROJECT_IS_TOP_LEVEL)
	set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

option(D2RLPLUGIN_BUILD_EXAMPLES "Build D2RLoader plugin SDK examples." ON)
option(D2RLPLUGIN_BUILD_EXAMPLES_BY_DEFAULT "Build D2RLoader plugin SDK examples as part of the default build." ${PROJECT_IS_TOP_LEVEL})
option(D2RLPLUGIN_BUILD_TESTS "Build D2RLoader plugin SDK contract tests." ${PROJECT_IS_TOP_LEVEL})

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

if(D2RLPLUGIN_BUILD_TESTS)
	enable_testing()
	set(D2RLPLUGIN_PUBLIC_HEADERS
		api console context core_exports custom_tables data_tables diagnostics
		game_rules handles hooks http input inventory item item_interactions lifecycle lifecycle_events
		localization logging network panels patching reimplementation_exports
		resource resources services shared_events threads version widgets
	)
	set(D2RLPLUGIN_HEADER_TEST_SOURCES)
	set(D2RLPLUGIN_HEADER_TEST_DIR "${CMAKE_CURRENT_BINARY_DIR}/header-tests")
	file(MAKE_DIRECTORY "${D2RLPLUGIN_HEADER_TEST_DIR}")
	foreach(header IN LISTS D2RLPLUGIN_PUBLIC_HEADERS)
		set(source "${D2RLPLUGIN_HEADER_TEST_DIR}/${header}.cpp")
		file(GENERATE OUTPUT "${source}" CONTENT "#include <D2RLPlugin/${header}.h>\n")
		list(APPEND D2RLPLUGIN_HEADER_TEST_SOURCES "${source}")
	endforeach()

	add_library(D2RLPluginHeaderTests OBJECT ${D2RLPLUGIN_HEADER_TEST_SOURCES})
	target_link_libraries(D2RLPluginHeaderTests PRIVATE D2RLPlugin::D2RLPlugin)
	add_executable(D2RLPluginContractTests tests/item_contract_tests.cpp)
	target_link_libraries(D2RLPluginContractTests PRIVATE D2RLPlugin::D2RLPlugin)
	add_test(NAME D2RLPluginContracts COMMAND D2RLPluginContractTests)
	add_custom_target(D2RLPluginTests DEPENDS D2RLPluginHeaderTests D2RLPluginContractTests)
endif()

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

d2rlplugin_add_sample(D2RLHelloConsolePlugin "d2rl-hello-console"
	examples/hello-console/hello_console_plugin.cpp
	examples/hello-console/hello_console_plugin.rc
)

d2rlplugin_add_sample(D2RLDataTablesSamplePlugin "d2rl-data-tables-sample"
	examples/data-tables/data_tables_sample_plugin.cpp
	examples/data-tables/data_tables_sample_plugin.rc
)

d2rlplugin_add_sample(D2RLUiPanelSamplePlugin "d2rl-ui-panel-sample"
	examples/ui-panel/ui_panel_sample_plugin.cpp
	examples/ui-panel/ui_panel_sample_plugin.rc
)

d2rlplugin_add_sample(D2RLWidgetLocalizationSamplePlugin "d2rl-widget-localization-sample"
	examples/widget-localization/widget_localization_sample_plugin.cpp
	examples/widget-localization/widget_localization_sample_plugin.rc
)

d2rlplugin_add_sample(D2RLInputActionSamplePlugin "d2rl-input-action-sample"
	examples/input-action/input_action_sample_plugin.cpp
	examples/input-action/input_action_sample_plugin.rc
)

d2rlplugin_add_sample(D2RLSharedEventsSamplePlugin "d2rl-shared-events-sample"
	examples/shared-events/shared_events_sample_plugin.cpp
	examples/shared-events/shared_events_sample_plugin.rc
)

d2rlplugin_add_sample(D2RLGameRulesSamplePlugin "d2rl-game-rules-sample"
	examples/game-rules/game_rules_sample_plugin.cpp
	examples/game-rules/game_rules_sample_plugin.rc
)

d2rlplugin_add_sample(D2RLGameplayLifecycleSamplePlugin "d2rl-gameplay-lifecycle-sample"
	examples/gameplay-lifecycle/gameplay_lifecycle_sample_plugin.cpp
	examples/gameplay-lifecycle/gameplay_lifecycle_sample_plugin.rc
)

d2rlplugin_add_sample(D2RLItemTransactionSamplePlugin "d2rl-item-transaction-sample"
	examples/item-transaction/item_transaction_sample_plugin.cpp
	examples/item-transaction/item_transaction_sample_plugin.rc
)

d2rlplugin_add_sample(D2RLItemInteractionsSamplePlugin "d2rl-item-interactions-sample"
	examples/item-interactions/item_interactions_sample_plugin.cpp
	examples/item-interactions/item_interactions_sample_plugin.rc
)

d2rlplugin_add_sample(D2RLNetworkPingSamplePlugin "d2rl-network-ping-sample"
	examples/network-ping/network_ping_sample_plugin.cpp
	examples/network-ping/network_ping_sample_plugin.rc
)

d2rlplugin_add_sample(D2RLHttpsSamplePlugin "d2rl-https-sample"
	examples/https/https_sample_plugin.cpp
	examples/https/https_sample_plugin.rc
)

d2rlplugin_add_sample(D2RLConfigFileSamplePlugin "d2rl-config-file-sample"
	examples/config-file/config_file_plugin.cpp
	examples/config-file/config_file_plugin.rc
)
d2rlplugin_embed_config(
	TARGET D2RLConfigFileSamplePlugin
	FILE "${CMAKE_CURRENT_SOURCE_DIR}/examples/config-file/config-file-sample.toml"
)

d2rlplugin_add_sample(D2RLPatchingSamplePlugin "d2rl-patching-sample"
	examples/patching/patching_sample_plugin.cpp
	examples/patching/patching_sample_plugin.rc
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
