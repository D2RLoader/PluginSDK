set(D2RLPLUGIN_PUBLIC_HEADERS
	api console context core_exports custom_tables data_tables diagnostics encounters
	game_rules handles hooks http input inventory item item_interactions lifecycle lifecycle_events
	localization logging mutations network panels patching reimplementation_exports
	overlay plugin_communication resource resources services shared_events threads version widgets
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
add_executable(D2RLPluginContractTests item_contract_tests.cpp)
target_link_libraries(D2RLPluginContractTests PRIVATE D2RLPlugin::D2RLPlugin)
target_compile_definitions(D2RLPluginContractTests PRIVATE D2RLPLUGIN_EXPECTED_SDK_VERSION="${PROJECT_VERSION}")
add_test(NAME D2RLPluginContracts COMMAND D2RLPluginContractTests)
add_executable(D2RLPluginServiceTests service_contract_tests.cpp)
target_link_libraries(D2RLPluginServiceTests PRIVATE D2RLPlugin::D2RLPlugin)
add_test(NAME D2RLPluginServices COMMAND D2RLPluginServiceTests)

# Every target in this directory is part of the test suite, including fixtures.
get_property(test_targets DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)
add_custom_target(D2RLPluginTests DEPENDS ${test_targets})
get_property(tests DIRECTORY PROPERTY TESTS)
if(tests)
	set_property(TEST ${tests} APPEND PROPERTY LABELS "offline;sdk")
	foreach(test IN LISTS tests)
		get_property(has_timeout TEST "${test}" PROPERTY TIMEOUT SET)
		if(NOT has_timeout)
			set_tests_properties("${test}" PROPERTIES TIMEOUT 60)
		endif()
	endforeach()
endif()
