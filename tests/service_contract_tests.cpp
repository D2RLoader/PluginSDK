#include <D2RLPlugin/api.h>
#include <array>
#include <cstdio>
#include <cstring>
#include <utility>

// Service IDs and field layouts shared with plugin DLLs.
static_assert(sizeof(D2RL::CustomTableService) == 40);
static_assert(alignof(D2RL::CustomTableService) == 8);
static_assert(static_cast<uint32_t>(D2RL::CustomTableService::Id) == 3);
static_assert(D2RL::CustomTableService::AbiVersion == 1);
static_assert(offsetof(D2RL::CustomTableService, serviceSize) == 0);
static_assert(offsetof(D2RL::CustomTableService, serviceVersion) == 4);
static_assert(offsetof(D2RL::CustomTableService, registerTable) == 8);
static_assert(offsetof(D2RL::CustomTableService, unregisterTable) == 16);
static_assert(offsetof(D2RL::CustomTableService, getTableInfo) == 24);
static_assert(offsetof(D2RL::CustomTableService, copyRows) == 32);
static_assert(D2RL::CustomTableServiceRequiredSize == 40);

static_assert(sizeof(D2RL::DataTableService) == 40);
static_assert(alignof(D2RL::DataTableService) == 8);
static_assert(static_cast<uint32_t>(D2RL::DataTableService::Id) == 8);
static_assert(D2RL::DataTableService::AbiVersion == 1);
static_assert(offsetof(D2RL::DataTableService, serviceSize) == 0);
static_assert(offsetof(D2RL::DataTableService, serviceVersion) == 4);
static_assert(offsetof(D2RL::DataTableService, getTable) == 8);
static_assert(offsetof(D2RL::DataTableService, getRow) == 16);
static_assert(offsetof(D2RL::DataTableService, findRowById) == 24);
static_assert(offsetof(D2RL::DataTableService, findRowByCode) == 32);
static_assert(D2RL::DataTableServiceRequiredSize == 40);

static_assert(sizeof(D2RL::DiagnosticsService) == 24);
static_assert(alignof(D2RL::DiagnosticsService) == 8);
static_assert(static_cast<uint32_t>(D2RL::DiagnosticsService::Id) == 10);
static_assert(D2RL::DiagnosticsService::AbiVersion == 1);
static_assert(offsetof(D2RL::DiagnosticsService, serviceSize) == 0);
static_assert(offsetof(D2RL::DiagnosticsService, serviceVersion) == 4);
static_assert(offsetof(D2RL::DiagnosticsService, queryHookStatus) == 8);
static_assert(offsetof(D2RL::DiagnosticsService, enumerateModificationRanges) == 16);
static_assert(D2RL::DiagnosticsServiceRequiredSize == 16);
static_assert(D2RL::DiagnosticsServiceEnumerateModificationRangesFieldEnd == 24);
static_assert(static_cast<uint32_t>(D2RL::Diagnostics::Result::BufferTooSmall) == 5);
static_assert(static_cast<uint32_t>(D2RL::Diagnostics::CallThroughState::Unknown) == 0);
static_assert(static_cast<uint32_t>(D2RL::Diagnostics::CallThroughState::No) == 1);
static_assert(static_cast<uint32_t>(D2RL::Diagnostics::CallThroughState::Yes) == 2);
static_assert(sizeof(D2RL::Diagnostics::ModificationRange) == 96);
static_assert(alignof(D2RL::Diagnostics::ModificationRange) == 8);
static_assert(offsetof(D2RL::Diagnostics::ModificationRange, structSize) == 0);
static_assert(offsetof(D2RL::Diagnostics::ModificationRange, flags) == 4);
static_assert(offsetof(D2RL::Diagnostics::ModificationRange, rva) == 8);
static_assert(offsetof(D2RL::Diagnostics::ModificationRange, size) == 16);
static_assert(offsetof(D2RL::Diagnostics::ModificationRange, state) == 20);
static_assert(offsetof(D2RL::Diagnostics::ModificationRange, kind) == 24);
static_assert(offsetof(D2RL::Diagnostics::ModificationRange, callThrough) == 28);
static_assert(offsetof(D2RL::Diagnostics::ModificationRange, ownerPluginId) == 32);

static_assert(sizeof(D2RL::EncounterService) == 32);
static_assert(alignof(D2RL::EncounterService) == 8);
static_assert(static_cast<uint32_t>(D2RL::EncounterService::Id) == 18);
static_assert(D2RL::EncounterService::AbiVersion == 1);
static_assert(offsetof(D2RL::EncounterService, serviceSize) == 0);
static_assert(offsetof(D2RL::EncounterService, serviceVersion) == 4);
static_assert(offsetof(D2RL::EncounterService, registerProvider) == 8);
static_assert(offsetof(D2RL::EncounterService, unregisterProvider) == 16);
static_assert(offsetof(D2RL::EncounterService, runTestSpawn) == 24);
static_assert(D2RL::EncounterServiceRequiredSize == 32);

static_assert(sizeof(D2RL::GameRuleService) == 40);
static_assert(alignof(D2RL::GameRuleService) == 8);
static_assert(static_cast<uint32_t>(D2RL::GameRuleService::Id) == 11);
static_assert(D2RL::GameRuleService::AbiVersion == 1);
static_assert(offsetof(D2RL::GameRuleService, serviceSize) == 0);
static_assert(offsetof(D2RL::GameRuleService, serviceVersion) == 4);
static_assert(offsetof(D2RL::GameRuleService, getMaxSockets) == 8);
static_assert(offsetof(D2RL::GameRuleService, getTotalMaxStack) == 16);
static_assert(offsetof(D2RL::GameRuleService, getRuntimeMaxSkillLevel) == 24);
static_assert(offsetof(D2RL::GameRuleService, canAllocateSkill) == 32);
static_assert(D2RL::GameRuleServiceRequiredSize == 40);

static_assert(sizeof(D2RL::HttpService) == 24);
static_assert(alignof(D2RL::HttpService) == 8);
static_assert(static_cast<uint32_t>(D2RL::HttpService::Id) == 16);
static_assert(D2RL::HttpService::AbiVersion == 1);
static_assert(offsetof(D2RL::HttpService, serviceSize) == 0);
static_assert(offsetof(D2RL::HttpService, serviceVersion) == 4);
static_assert(offsetof(D2RL::HttpService, send) == 8);
static_assert(offsetof(D2RL::HttpService, cancel) == 16);
static_assert(D2RL::HttpServiceRequiredSize == 24);

static_assert(sizeof(D2RL::InputService) == 32);
static_assert(alignof(D2RL::InputService) == 8);
static_assert(static_cast<uint32_t>(D2RL::InputService::Id) == 7);
static_assert(D2RL::InputService::AbiVersion == 1);
static_assert(offsetof(D2RL::InputService, serviceSize) == 0);
static_assert(offsetof(D2RL::InputService, serviceVersion) == 4);
static_assert(offsetof(D2RL::InputService, registerAction) == 8);
static_assert(offsetof(D2RL::InputService, unregisterAction) == 16);
static_assert(offsetof(D2RL::InputService, getBinding) == 24);
static_assert(D2RL::InputServiceRequiredSize == 32);

static_assert(sizeof(D2RL::InventoryService) == 112);
static_assert(alignof(D2RL::InventoryService) == 8);
static_assert(static_cast<uint32_t>(D2RL::InventoryService::Id) == 5);
static_assert(D2RL::InventoryService::AbiVersion == 1);
static_assert(offsetof(D2RL::InventoryService, serviceSize) == 0);
static_assert(offsetof(D2RL::InventoryService, serviceVersion) == 4);
static_assert(offsetof(D2RL::InventoryService, registerPlayerPage) == 8);
static_assert(offsetof(D2RL::InventoryService, unregisterPlayerPage) == 16);
static_assert(offsetof(D2RL::InventoryService, getRegistrationInfo) == 24);
static_assert(offsetof(D2RL::InventoryService, configurePlayerPage) == 32);
static_assert(offsetof(D2RL::InventoryService, getPlayerPageConfigurationInfo) == 40);
static_assert(offsetof(D2RL::InventoryService, executeLocalPlayerMove) == 48);
static_assert(offsetof(D2RL::InventoryService, isItemTypeCodeKnown) == 56);
static_assert(offsetof(D2RL::InventoryService, configurePlayerPageItemPolicy) == 64);
static_assert(offsetof(D2RL::InventoryService, configurePlayerPageCharmPolicy) == 72);
static_assert(offsetof(D2RL::InventoryService, getLocalPlayer) == 80);
static_assert(offsetof(D2RL::InventoryService, getCursorItem) == 88);
static_assert(offsetof(D2RL::InventoryService, getEquippedItem) == 96);
static_assert(offsetof(D2RL::InventoryService, forEachInventoryItem) == 104);
static_assert(D2RL::InventoryServiceRequiredSize == 112);

static_assert(sizeof(D2RL::ItemService) == 88);
static_assert(alignof(D2RL::ItemService) == 8);
static_assert(static_cast<uint32_t>(D2RL::ItemService::Id) == 15);
static_assert(D2RL::ItemService::AbiVersion == 1);
static_assert(offsetof(D2RL::ItemService, serviceSize) == 0);
static_assert(offsetof(D2RL::ItemService, serviceVersion) == 4);
static_assert(offsetof(D2RL::ItemService, getItemInfo) == 8);
static_assert(offsetof(D2RL::ItemService, createItem) == 16);
static_assert(offsetof(D2RL::ItemService, editItem) == 24);
static_assert(offsetof(D2RL::ItemService, destroyItem) == 32);
static_assert(offsetof(D2RL::ItemService, executeTransaction) == 40);
static_assert(offsetof(D2RL::ItemService, editNativeItem) == 48);
static_assert(offsetof(D2RL::ItemService, executeExistingItemTransaction) == 56);
static_assert(offsetof(D2RL::ItemService, splitStack) == 64);
static_assert(offsetof(D2RL::ItemService, capabilities) == 72);
static_assert(offsetof(D2RL::ItemService, augmentItemAffix) == 80);
static_assert(D2RL::ItemServiceRequiredSize == 64);
static_assert(D2RL::ItemServiceSplitStackFieldEnd == 72);
static_assert(D2RL::ItemServiceCapabilitiesFieldEnd == 80);
static_assert(D2RL::ItemServiceAffixAugmentFieldEnd == 88);

static_assert(sizeof(D2RL::ItemInteractionService) == 32);
static_assert(alignof(D2RL::ItemInteractionService) == 8);
static_assert(static_cast<uint32_t>(D2RL::ItemInteractionService::Id) == 17);
static_assert(D2RL::ItemInteractionService::AbiVersion == 1);
static_assert(offsetof(D2RL::ItemInteractionService, serviceSize) == 0);
static_assert(offsetof(D2RL::ItemInteractionService, serviceVersion) == 4);
static_assert(offsetof(D2RL::ItemInteractionService, registerListener) == 8);
static_assert(offsetof(D2RL::ItemInteractionService, unregisterListener) == 16);
static_assert(offsetof(D2RL::ItemInteractionService, supportedContainerMask) == 24);
static_assert(D2RL::ItemInteractionServiceRequiredSize == 24);
static_assert(D2RL::ItemInteractionServiceSupportedContainerMaskFieldEnd == 28);

static_assert(sizeof(D2RL::MutationService) == 64);
static_assert(alignof(D2RL::MutationService) == 8);
static_assert(static_cast<uint32_t>(D2RL::MutationService::Id) == 19);
static_assert(D2RL::MutationService::AbiVersion == 1);
static_assert(offsetof(D2RL::MutationService, serviceSize) == 0);
static_assert(offsetof(D2RL::MutationService, serviceVersion) == 4);
static_assert(offsetof(D2RL::MutationService, beginTransaction) == 8);
static_assert(offsetof(D2RL::MutationService, stageBytePatch) == 16);
static_assert(offsetof(D2RL::MutationService, stageRel32Patch) == 24);
static_assert(offsetof(D2RL::MutationService, stageInlineHook) == 32);
static_assert(offsetof(D2RL::MutationService, commit) == 40);
static_assert(offsetof(D2RL::MutationService, getInlineHookOriginal) == 48);
static_assert(offsetof(D2RL::MutationService, cancelTransaction) == 56);
static_assert(D2RL::MutationServiceRequiredSize == 64);

static_assert(sizeof(D2RL::OverlayService) == 88);
static_assert(alignof(D2RL::OverlayService) == 8);
static_assert(static_cast<uint32_t>(D2RL::OverlayService::Id) == 20);
static_assert(D2RL::OverlayService::AbiVersion == 1);
static_assert(offsetof(D2RL::OverlayService, serviceSize) == 0);
static_assert(offsetof(D2RL::OverlayService, serviceVersion) == 4);
static_assert(offsetof(D2RL::OverlayService, registerFrameCallback) == 8);
static_assert(offsetof(D2RL::OverlayService, unregisterFrameCallback) == 16);
static_assert(offsetof(D2RL::OverlayService, getMetrics) == 24);
static_assert(offsetof(D2RL::OverlayService, drawLine) == 32);
static_assert(offsetof(D2RL::OverlayService, drawRectangle) == 40);
static_assert(offsetof(D2RL::OverlayService, drawFilledRectangle) == 48);
static_assert(offsetof(D2RL::OverlayService, drawText) == 56);
static_assert(offsetof(D2RL::OverlayService, measureText) == 64);
static_assert(offsetof(D2RL::OverlayService, pushClip) == 72);
static_assert(offsetof(D2RL::OverlayService, popClip) == 80);
static_assert(D2RL::OverlayServiceRequiredSize == 88);

static_assert(sizeof(D2RL::PluginCommunicationService) == 56);
static_assert(alignof(D2RL::PluginCommunicationService) == 8);
static_assert(static_cast<uint32_t>(D2RL::PluginCommunicationService::Id) == 21);
static_assert(D2RL::PluginCommunicationService::AbiVersion == 1);
static_assert(offsetof(D2RL::PluginCommunicationService, serviceSize) == 0);
static_assert(offsetof(D2RL::PluginCommunicationService, serviceVersion) == 4);
static_assert(offsetof(D2RL::PluginCommunicationService, publishService) == 8);
static_assert(offsetof(D2RL::PluginCommunicationService, acquireService) == 16);
static_assert(offsetof(D2RL::PluginCommunicationService, releaseService) == 24);
static_assert(offsetof(D2RL::PluginCommunicationService, subscribeEvent) == 32);
static_assert(offsetof(D2RL::PluginCommunicationService, unsubscribeEvent) == 40);
static_assert(offsetof(D2RL::PluginCommunicationService, publishEvent) == 48);
static_assert(D2RL::PluginCommunicationServiceRequiredSize == 56);
static_assert(sizeof(D2RL::PluginCommunication::PublishServiceRequest) == 32);
static_assert(sizeof(D2RL::PluginCommunication::AcquireServiceRequest) == 32);
static_assert(sizeof(D2RL::PluginCommunication::AcquiredService) == 32);
static_assert(sizeof(D2RL::PluginCommunication::PublishEventRequest) == 32);
static_assert(sizeof(D2RL::PluginCommunication::EventView) == 40);
static_assert(sizeof(D2RL::PluginCommunication::EventSubscription) == 48);
static_assert(sizeof(D2RL::Overlay::Frame) == 40);
static_assert(sizeof(D2RL::Overlay::CallbackRegistration) == 32);
static_assert(sizeof(D2RL::Overlay::Metrics) == 32);
static_assert(sizeof(D2RL::Overlay::LineRequest) == 56);
static_assert(sizeof(D2RL::Overlay::RectangleRequest) == 56);
static_assert(sizeof(D2RL::Overlay::FilledRectangleRequest) == 56);
static_assert(sizeof(D2RL::Overlay::TextRequest) == 64);
static_assert(sizeof(D2RL::Overlay::MeasureTextRequest) == 40);
static_assert(sizeof(D2RL::Overlay::TextMetrics) == 16);
static_assert(sizeof(D2RL::Overlay::ClipRequest) == 32);

static_assert(sizeof(D2RL::LifecycleService) == 56);
static_assert(alignof(D2RL::LifecycleService) == 8);
static_assert(static_cast<uint32_t>(D2RL::LifecycleService::Id) == 1);
static_assert(D2RL::LifecycleService::AbiVersion == 1);
static_assert(offsetof(D2RL::LifecycleService, serviceSize) == 0);
static_assert(offsetof(D2RL::LifecycleService, serviceVersion) == 4);
static_assert(offsetof(D2RL::LifecycleService, registerDataTablesLoadedListener) == 8);
static_assert(offsetof(D2RL::LifecycleService, unregisterDataTablesLoadedListener) == 16);
static_assert(offsetof(D2RL::LifecycleService, registerGameplayEventListener) == 24);
static_assert(offsetof(D2RL::LifecycleService, unregisterGameplayEventListener) == 32);
static_assert(offsetof(D2RL::LifecycleService, registerMonsterDeathListener) == 40);
static_assert(offsetof(D2RL::LifecycleService, unregisterMonsterDeathListener) == 48);
static_assert(D2RL::LifecycleServiceRequiredSize == 56);
static_assert(sizeof(D2RL::Lifecycle::UnitIdentity) == 16);
static_assert(sizeof(D2RL::Lifecycle::MonsterDeathEvent) == 56);
static_assert(sizeof(D2RL::Lifecycle::MonsterDeathListener) == 24);
static_assert(offsetof(D2RL::Lifecycle::MonsterDeathEvent, monster) == 24);
static_assert(offsetof(D2RL::Lifecycle::MonsterDeathEvent, killer) == 40);

static_assert(sizeof(D2RL::LocalizationService) == 24);
static_assert(alignof(D2RL::LocalizationService) == 8);
static_assert(static_cast<uint32_t>(D2RL::LocalizationService::Id) == 14);
static_assert(D2RL::LocalizationService::AbiVersion == 2);
static_assert(offsetof(D2RL::LocalizationService, serviceSize) == 0);
static_assert(offsetof(D2RL::LocalizationService, serviceVersion) == 4);
static_assert(offsetof(D2RL::LocalizationService, getStringById) == 8);
static_assert(offsetof(D2RL::LocalizationService, getStringByKey) == 16);
static_assert(D2RL::LocalizationServiceRequiredSize == 24);

static_assert(sizeof(D2RL::NetworkService) == 64);
static_assert(alignof(D2RL::NetworkService) == 8);
static_assert(static_cast<uint32_t>(D2RL::NetworkService::Id) == 6);
static_assert(D2RL::NetworkService::AbiVersion == 1);
static_assert(offsetof(D2RL::NetworkService, serviceSize) == 0);
static_assert(offsetof(D2RL::NetworkService, serviceVersion) == 4);
static_assert(offsetof(D2RL::NetworkService, registerChannel) == 8);
static_assert(offsetof(D2RL::NetworkService, unregisterChannel) == 16);
static_assert(offsetof(D2RL::NetworkService, getChannelInfo) == 24);
static_assert(offsetof(D2RL::NetworkService, connectToHost) == 32);
static_assert(offsetof(D2RL::NetworkService, sendToHost) == 40);
static_assert(offsetof(D2RL::NetworkService, sendToClient) == 48);
static_assert(offsetof(D2RL::NetworkService, getPeerPlayer) == 56);
static_assert(D2RL::NetworkServiceRequiredSize == 64);

static_assert(sizeof(D2RL::PanelService) == 96);
static_assert(alignof(D2RL::PanelService) == 8);
static_assert(static_cast<uint32_t>(D2RL::PanelService::Id) == 4);
static_assert(D2RL::PanelService::AbiVersion == 1);
static_assert(offsetof(D2RL::PanelService, serviceSize) == 0);
static_assert(offsetof(D2RL::PanelService, serviceVersion) == 4);
static_assert(offsetof(D2RL::PanelService, registerPanel) == 8);
static_assert(offsetof(D2RL::PanelService, unregisterPanel) == 16);
static_assert(offsetof(D2RL::PanelService, getPanelInfo) == 24);
static_assert(offsetof(D2RL::PanelService, openPanel) == 32);
static_assert(offsetof(D2RL::PanelService, closePanel) == 40);
static_assert(offsetof(D2RL::PanelService, togglePanel) == 48);
static_assert(offsetof(D2RL::PanelService, bindPlayerPageGrid) == 56);
static_assert(offsetof(D2RL::PanelService, registerChildLayout) == 64);
static_assert(offsetof(D2RL::PanelService, unregisterChildLayout) == 72);
static_assert(offsetof(D2RL::PanelService, registerControllerRoute) == 80);
static_assert(offsetof(D2RL::PanelService, unregisterControllerRoute) == 88);
static_assert(D2RL::PanelServiceRequiredSize == 96);

static_assert(sizeof(D2RL::ResourceService) == 32);
static_assert(alignof(D2RL::ResourceService) == 8);
static_assert(static_cast<uint32_t>(D2RL::ResourceService::Id) == 2);
static_assert(D2RL::ResourceService::AbiVersion == 1);
static_assert(offsetof(D2RL::ResourceService, serviceSize) == 0);
static_assert(offsetof(D2RL::ResourceService, serviceVersion) == 4);
static_assert(offsetof(D2RL::ResourceService, registerResource) == 8);
static_assert(offsetof(D2RL::ResourceService, unregisterResource) == 16);
static_assert(offsetof(D2RL::ResourceService, getRegistrationInfo) == 24);
static_assert(D2RL::ResourceServiceRequiredSize == 32);

static_assert(sizeof(D2RL::SharedEventService) == 40);
static_assert(alignof(D2RL::SharedEventService) == 8);
static_assert(static_cast<uint32_t>(D2RL::SharedEventService::Id) == 9);
static_assert(D2RL::SharedEventService::AbiVersion == 1);
static_assert(offsetof(D2RL::SharedEventService, serviceSize) == 0);
static_assert(offsetof(D2RL::SharedEventService, serviceVersion) == 4);
static_assert(offsetof(D2RL::SharedEventService, registerItemTooltipListener) == 8);
static_assert(offsetof(D2RL::SharedEventService, unregisterItemTooltipListener) == 16);
static_assert(offsetof(D2RL::SharedEventService, registerUiMessageListener) == 24);
static_assert(offsetof(D2RL::SharedEventService, unregisterUiMessageListener) == 32);
static_assert(D2RL::SharedEventServiceRequiredSize == 40);

static_assert(sizeof(D2RL::ThreadService) == 24);
static_assert(alignof(D2RL::ThreadService) == 8);
static_assert(static_cast<uint32_t>(D2RL::ThreadService::Id) == 13);
static_assert(D2RL::ThreadService::AbiVersion == 1);
static_assert(offsetof(D2RL::ThreadService, serviceSize) == 0);
static_assert(offsetof(D2RL::ThreadService, serviceVersion) == 4);
static_assert(offsetof(D2RL::ThreadService, runOnUiThread) == 8);
static_assert(offsetof(D2RL::ThreadService, runOnGameThread) == 16);
static_assert(D2RL::ThreadServiceRequiredSize == 24);

static_assert(sizeof(D2RL::WidgetService) == 64);
static_assert(alignof(D2RL::WidgetService) == 8);
static_assert(static_cast<uint32_t>(D2RL::WidgetService::Id) == 12);
static_assert(D2RL::WidgetService::AbiVersion == 1);
static_assert(offsetof(D2RL::WidgetService, serviceSize) == 0);
static_assert(offsetof(D2RL::WidgetService, serviceVersion) == 4);
static_assert(offsetof(D2RL::WidgetService, findPanel) == 8);
static_assert(offsetof(D2RL::WidgetService, findWidget) == 16);
static_assert(offsetof(D2RL::WidgetService, getWidgetRect) == 24);
static_assert(offsetof(D2RL::WidgetService, setWidgetVisible) == 32);
static_assert(offsetof(D2RL::WidgetService, setWidgetEnabled) == 40);
static_assert(offsetof(D2RL::WidgetService, dispatchUiAction) == 48);
static_assert(offsetof(D2RL::WidgetService, getInputText) == 56);
static_assert(D2RL::WidgetServiceRequiredSize == 64);

static int failures = 0;

static void Check(bool condition, const char* message) {
	if (!condition) {
		std::fprintf(stderr, "%s\n", message);
		++failures;
	}
}

struct QueryState {
	const D2RL::PluginContext* context         = nullptr;
	const void*                table           = nullptr;
	D2RL::ServiceQueryResult   result          = D2RL::ServiceQueryResult::Success;
	uint32_t                   expectedId      = 0;
	uint32_t                   expectedVersion = 1;
	uint32_t                   calls           = 0;
};

static QueryState state;

static auto __cdecl Query(const D2RL::PluginContext* context, D2RL::ServiceId id, uint32_t version, const void** output) noexcept -> D2RL::ServiceQueryResult {
	Check(context == state.context, "Query forwards the plugin context");
	const auto idValue = static_cast<uint32_t>(id);
	Check(idValue == state.expectedId, "Query requests the service's assigned ID");
	Check(version == state.expectedVersion, "Query requests the service's ABI version");
	Check(*output == nullptr, "Query starts with an empty output");
	++state.calls;
	*output = state.table;
	return state.result;
}

template <typename Service>
static void TestService(uint32_t expectedId) {
	Service             table {};
	D2RL::PluginApi     api { .apiSize = D2RL::PluginApiSize, .queryService = Query };
	D2RL::PluginContext context { .contextSize = D2RL::PluginContextSize, .abiVersion = D2RL_PLUGIN_ABI_VERSION, .api = &api };
	state = { .context = &context, .table = &table, .expectedId = expectedId, .expectedVersion = Service::AbiVersion };

	const Service* service = nullptr;
	auto           result  = context.QueryService(&service);
	Check(result == D2RL::ServiceQueryResult::Success && service == &table, "Member query returns the service table");
	Check(state.calls == 1, "Member query calls the loader once");

	service = nullptr;
	result  = D2RL::QueryService(&context, &service);
	Check(result == D2RL::ServiceQueryResult::Success && service == &table, "Free query returns the service table");
	Check(state.calls == 2, "Free query calls the loader once");

	constexpr std::array errors {
		D2RL::ServiceQueryResult::InvalidArgument,
		D2RL::ServiceQueryResult::UnknownService,
		D2RL::ServiceQueryResult::UnsupportedVersion,
		D2RL::ServiceQueryResult::Unavailable,
		D2RL::ServiceQueryResult::OwnerInactive,
	};
	for (auto error : errors) {
		state.result = error;
		service      = &table;
		result       = context.QueryService(&service);
		Check(result == error && service == nullptr, "Member query preserves errors and clears output");

		service = &table;
		result  = D2RL::QueryService(&context, &service);
		Check(result == error && service == nullptr, "Free query preserves errors and clears output");
	}

	const auto      calls    = state.calls;
	const Service** noOutput = nullptr;
	result                   = context.QueryService(noOutput);
	Check(result == D2RL::ServiceQueryResult::InvalidArgument, "Member query rejects a null output");
	result = D2RL::QueryService(&context, noOutput);
	Check(result == D2RL::ServiceQueryResult::InvalidArgument, "Free query rejects a null output");
	Check(state.calls == calls, "Invalid outputs do not call the loader");

	service = &table;
	result  = D2RL::QueryService(nullptr, &service);
	Check(result == D2RL::ServiceQueryResult::InvalidArgument && service == nullptr, "Null context clears output");
	result = D2RL::QueryService(nullptr, noOutput);
	Check(result == D2RL::ServiceQueryResult::InvalidArgument, "Null context and output are rejected");

	api.queryService = nullptr;
	service          = &table;
	result           = context.QueryService(&service);
	Check(result == D2RL::ServiceQueryResult::Unavailable && service == nullptr, "Missing query callback reports unavailable");

	api.queryService = Query;
	api.apiSize      = D2RL::PluginApiV2Size;
	service          = &table;
	result           = context.QueryService(&service);
	Check(result == D2RL::ServiceQueryResult::Unavailable && service == nullptr, "Older API without service queries reports unavailable");

	api.apiSize = D2RL::PluginApiSize;
	context.api = nullptr;
	service     = &table;
	result      = context.QueryService(&service);
	Check(result == D2RL::ServiceQueryResult::Unavailable && service == nullptr, "Missing API reports unavailable");

	context.api         = &api;
	context.contextSize = 0;
	service             = &table;
	result              = context.QueryService(&service);
	Check(result == D2RL::ServiceQueryResult::Unavailable && service == nullptr, "Incomplete context reports unavailable");
	Check(state.calls == calls, "Unavailable APIs do not call the loader");
}

static void TestRawQuery() {
	D2RL::InventoryService table {};
	D2RL::PluginApi        api { .apiSize = D2RL::PluginApiSize, .queryService = Query };
	D2RL::PluginContext    context { .contextSize = D2RL::PluginContextSize, .abiVersion = D2RL_PLUGIN_ABI_VERSION, .api = &api };
	state               = { .context = &context, .table = &table, .expectedId = 5 };
	const void* service = nullptr;
	const auto  id      = static_cast<D2RL::ServiceId>(5);
	auto        result  = context.QueryService(id, 1, &service);
	Check(result == D2RL::ServiceQueryResult::Success && service == &table, "Raw query accepts numeric ID and version");
	service = nullptr;
	result  = D2RL::QueryService(&context, id, 1, &service);
	Check(result == D2RL::ServiceQueryResult::Success && service == &table, "Raw free query accepts numeric ID and version");
}

struct SharedPriceTable {
	uint32_t value;
};

static SharedPriceTable priceTable { .value = 42 };
static uint32_t         priceReleases = 0;

static auto __cdecl AcquirePriceService(const D2RL::PluginContext*, const D2RL::PluginCommunication::AcquireServiceRequest* request, D2RL::PluginCommunication::AcquiredService* acquired) noexcept
	-> D2RL::PluginCommunication::Result {
	Check(std::strcmp(request->providerPluginId, "example.prices") == 0, "The lease helper forwards the provider ID");
	Check(std::strcmp(request->name, "price-api") == 0, "The lease helper forwards the service name");
	Check(request->minimumVersion == 1 && request->minimumTableSize == sizeof(SharedPriceTable), "The lease helper forwards version and size requirements");
	*acquired = {
		.structSize     = D2RL::PluginCommunication::AcquiredServiceSize,
		.handle         = 10,
		.table          = &priceTable,
		.serviceVersion = 2,
		.tableSize      = sizeof(priceTable),
	};
	return D2RL::PluginCommunication::Result::Success;
}

static auto __cdecl ReleasePriceService(const D2RL::PluginContext*, D2RL::PluginCommunication::ServiceHandle handle) noexcept -> D2RL::PluginCommunication::Result {
	Check(handle == 10, "The lease helper releases the acquired handle");
	++priceReleases;
	return D2RL::PluginCommunication::Result::Success;
}

static void TestPluginServiceLease() {
	D2RL::PluginContext                    context {};
	const D2RL::PluginCommunicationService communication {
		.serviceSize    = D2RL::PluginCommunicationServiceSize,
		.serviceVersion = D2RL::PluginCommunicationService::AbiVersion,
		.acquireService = AcquirePriceService,
		.releaseService = ReleasePriceService,
	};

	priceReleases = 0;
	{
		D2RL::PluginCommunication::ServiceLease<SharedPriceTable> lease;
		const auto result = D2RL::PluginCommunication::Acquire(&context, &communication, "example.prices", "price-api", 1, sizeof(SharedPriceTable), &lease);
		Check(result == D2RL::PluginCommunication::Result::Success, "The lease helper acquires a compatible plugin service");
		Check(lease && lease->value == 42 && lease.Version() == 2 && lease.Size() == sizeof(priceTable), "The lease exposes the table, version, and size");

		D2RL::PluginCommunication::ServiceLease<SharedPriceTable> moved(std::move(lease));
		Check(!lease && moved, "Moving a lease transfers its handle");
	}
	Check(priceReleases == 1, "Destroying the lease releases its handle once");
}

auto main() -> int {
	TestService<D2RL::CustomTableService>(3);
	TestService<D2RL::DataTableService>(8);
	TestService<D2RL::DiagnosticsService>(10);
	TestService<D2RL::EncounterService>(18);
	TestService<D2RL::GameRuleService>(11);
	TestService<D2RL::HttpService>(16);
	TestService<D2RL::InputService>(7);
	TestService<D2RL::InventoryService>(5);
	TestService<D2RL::ItemService>(15);
	TestService<D2RL::ItemInteractionService>(17);
	TestService<D2RL::LifecycleService>(1);
	TestService<D2RL::LocalizationService>(14);
	TestService<D2RL::NetworkService>(6);
	TestService<D2RL::OverlayService>(20);
	TestService<D2RL::PluginCommunicationService>(21);
	TestService<D2RL::PanelService>(4);
	TestService<D2RL::ResourceService>(2);
	TestService<D2RL::SharedEventService>(9);
	TestService<D2RL::ThreadService>(13);
	TestService<D2RL::WidgetService>(12);
	TestRawQuery();
	TestPluginServiceLease();
	return failures == 0 ? 0 : 1;
}
