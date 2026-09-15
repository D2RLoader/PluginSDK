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
		OUTPUT_NAME "${output_name}"
		RUNTIME_OUTPUT_DIRECTORY_DEBUG "${D2RLPLUGIN_SAMPLE_PLUGINS_DIR_DEBUG}"
		RUNTIME_OUTPUT_DIRECTORY_RELEASE "${D2RLPLUGIN_SAMPLE_PLUGINS_DIR_RELEASE}"
		LIBRARY_OUTPUT_DIRECTORY_DEBUG "${D2RLPLUGIN_SAMPLE_PLUGINS_DIR_DEBUG}"
		LIBRARY_OUTPUT_DIRECTORY_RELEASE "${D2RLPLUGIN_SAMPLE_PLUGINS_DIR_RELEASE}"
		ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/build/lib/debug/samples"
		ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/build/lib/release/samples"
	)

	if(MSVC)
		target_compile_options(${target} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:/W3;/permissive-;/EHsc>")
	endif()
endfunction()

d2rlplugin_add_sample(D2RLHelloConsolePlugin "d2rl-hello-console"
	hello-console/hello_console_plugin.cpp
	hello-console/hello_console_plugin.rc
)

d2rlplugin_add_sample(D2RLDataTablesSamplePlugin "d2rl-data-tables-sample"
	data-tables/data_tables_sample_plugin.cpp
	data-tables/data_tables_sample_plugin.rc
)

d2rlplugin_add_sample(D2RLUiPanelSamplePlugin "d2rl-ui-panel-sample"
	ui-panel/ui_panel_sample_plugin.cpp
	ui-panel/ui_panel_sample_plugin.rc
)

d2rlplugin_add_sample(D2RLWidgetLocalizationSamplePlugin "d2rl-widget-localization-sample"
	widget-localization/widget_localization_sample_plugin.cpp
	widget-localization/widget_localization_sample_plugin.rc
)

d2rlplugin_add_sample(D2RLInputActionSamplePlugin "d2rl-input-action-sample"
	input-action/input_action_sample_plugin.cpp
	input-action/input_action_sample_plugin.rc
)

d2rlplugin_add_sample(D2RLSharedEventsSamplePlugin "d2rl-shared-events-sample"
	shared-events/shared_events_sample_plugin.cpp
	shared-events/shared_events_sample_plugin.rc
)

d2rlplugin_add_sample(D2RLGameRulesSamplePlugin "d2rl-game-rules-sample"
	game-rules/game_rules_sample_plugin.cpp
	game-rules/game_rules_sample_plugin.rc
)

d2rlplugin_add_sample(D2RLGameplayLifecycleSamplePlugin "d2rl-gameplay-lifecycle-sample"
	gameplay-lifecycle/gameplay_lifecycle_sample_plugin.cpp
	gameplay-lifecycle/gameplay_lifecycle_sample_plugin.rc
)

d2rlplugin_add_sample(D2RLItemTransactionSamplePlugin "d2rl-item-transaction-sample"
	item-transaction/item_transaction_sample_plugin.cpp
	item-transaction/item_transaction_sample_plugin.rc
)

d2rlplugin_add_sample(D2RLItemInteractionsSamplePlugin "d2rl-item-interactions-sample"
	item-interactions/item_interactions_sample_plugin.cpp
	item-interactions/item_interactions_sample_plugin.rc
)

d2rlplugin_add_sample(D2RLNetworkPingSamplePlugin "d2rl-network-ping-sample"
	network-ping/network_ping_sample_plugin.cpp
	network-ping/network_ping_sample_plugin.rc
)

d2rlplugin_add_sample(D2RLHttpsSamplePlugin "d2rl-https-sample"
	https/https_sample_plugin.cpp
	https/https_sample_plugin.rc
)

d2rlplugin_add_sample(D2RLConfigFileSamplePlugin "d2rl-config-file-sample"
	config-file/config_file_plugin.cpp
	config-file/config_file_plugin.rc
)
d2rlplugin_embed_config(
	TARGET D2RLConfigFileSamplePlugin
	FILE "${CMAKE_CURRENT_SOURCE_DIR}/config-file/config-file-sample.toml"
)

d2rlplugin_add_sample(D2RLPatchingSamplePlugin "d2rl-patching-sample"
	patching/patching_sample_plugin.cpp
	patching/patching_sample_plugin.rc
)

set(D2RLPLUGIN_PATCHING_SAMPLE_PATCH_OUTPUT_DIR "$<IF:$<CONFIG:Debug>,${D2RLPLUGIN_SAMPLE_PATCHES_DIR_DEBUG},${D2RLPLUGIN_SAMPLE_PATCHES_DIR_RELEASE}>")
set(D2RLPLUGIN_PATCHING_SAMPLE_CONFIG_OUTPUT_DIR "$<IF:$<CONFIG:Debug>,${D2RLPLUGIN_SAMPLE_CONFIG_DIR_DEBUG},${D2RLPLUGIN_SAMPLE_CONFIG_DIR_RELEASE}>")
add_custom_command(TARGET D2RLPatchingSamplePlugin POST_BUILD
	COMMAND "${CMAKE_COMMAND}" -E make_directory "${D2RLPLUGIN_PATCHING_SAMPLE_PATCH_OUTPUT_DIR}"
	COMMAND "${CMAKE_COMMAND}" -E make_directory "${D2RLPLUGIN_PATCHING_SAMPLE_CONFIG_OUTPUT_DIR}"
	COMMAND "${CMAKE_COMMAND}" -E copy_if_different
		"${CMAKE_CURRENT_SOURCE_DIR}/json-patches/example-patches.json"
		"${D2RLPLUGIN_PATCHING_SAMPLE_PATCH_OUTPUT_DIR}/example-patches.json"
	COMMAND "${CMAKE_COMMAND}" -E copy_if_different
		"${CMAKE_CURRENT_SOURCE_DIR}/patching/patching-sample.toml"
		"${D2RLPLUGIN_PATCHING_SAMPLE_CONFIG_OUTPUT_DIR}/patching-sample.toml"
	COMMENT "Copying D2RL patching sample files"
	VERBATIM
)
