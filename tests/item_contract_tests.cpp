#include <D2RLPlugin/api.h>
#include <string_view>

static_assert(std::string_view(D2RL_SDK_VERSION) == D2RLPLUGIN_EXPECTED_SDK_VERSION);

static_assert(static_cast<uint32_t>(D2RL::ServiceId::Item) == 15);
static_assert(static_cast<uint32_t>(D2RL::ServiceId::Http) == 16);
static_assert(static_cast<uint32_t>(D2RL::ServiceId::ItemInteraction) == 17);
static_assert(static_cast<uint32_t>(D2RL::ServiceId::Overlay) == 20);
static_assert(D2RL_PLUGIN_ABI_VERSION == 4);
static_assert(D2RL_PLUGIN_MIN_ABI_VERSION == 2);
static_assert(D2RL_PLUGIN_ROLES_ABI_VERSION == 3);
static_assert(D2RL_PLUGIN_HTTP_ABI_VERSION == 4);
// These offsets and sizes define the layout shared with plugin DLLs.
static_assert(offsetof(D2RL::PluginInfo, abiVersion) == 4);
static_assert(offsetof(D2RL::PluginInfo, id) == 8);
static_assert(D2RL::PluginInfoAbiVersionSize == 8);
static_assert(sizeof(D2RL::PluginInfo) == 72);
static_assert(offsetof(D2RL::PluginContext, abiVersion) == 4);
static_assert(offsetof(D2RL::PluginContext, api) == 8);
static_assert(sizeof(D2RL::PluginContext) == 200);
static_assert(D2RL::Items::MakeItemCode("r01") == 0x20313072);
static_assert(D2RL::Items::MakeItemCode("cap") == 0x20706163);
static_assert(D2RL::Items::MakeItemCode("abcd") == 0x64636261);
static_assert(D2RL::Items::MakeExactProperty(0, 25).minimum == 25);
static_assert(D2RL::Items::MakeExactProperty(0, 25).maximum == 25);
static_assert(D2RL::Items::TransactionRequiredSize == D2RL::Items::TransactionSize);
static_assert(D2RL::Items::TransactionResultRequiredSize == D2RL::Items::TransactionResultSize);
static_assert(D2RL::InventoryServiceRequiredSize == 112);
static_assert(D2RL::ItemServiceRequiredSize == 64);
static_assert(D2RL::ItemInteractionServiceRequiredSize == 24);
static_assert(D2RL::ItemInteractions::ItemInteractionEventRequiredSize == 56);
static_assert(D2RL::ItemInteractions::ItemInteractionListenerRequiredSize == 32);
static_assert(D2RL::ItemInteractions::ItemInteractionListenerSize == 40);
static_assert(D2RL::ItemInteractions::ItemInteractionListenerContainerMaskFieldEnd == 36);
static_assert(D2RL::ItemInteractionServiceSize == 32);
static_assert(D2RL::ItemInteractionServiceSupportedContainerMaskFieldEnd == 28);
static_assert(D2RL::Mutations::BytePatchRequestRequiredSize == 48);
static_assert(D2RL::Mutations::Rel32PatchRequestRequiredSize == 48);
static_assert(D2RL::Mutations::InlineHookRequestRequiredSize == 48);
static_assert(D2RL::Mutations::CommitResultRequiredSize == 24);
static_assert(D2RL::Overlay::FrameRequiredSize == 40);
static_assert(D2RL::Overlay::CallbackRegistrationRequiredSize == 32);
static_assert(D2RL::Overlay::TextRequestRequiredSize == 64);
static_assert(D2RL::OverlayServiceRequiredSize == 88);
static_assert(D2RL::Diagnostics::ModificationRangeRequiredSize == 96);
static_assert(D2RL::Items::ExistingItemOperationRequiredSize == 56);
static_assert(D2RL::Items::ExistingItemOperationSize == 64);
static_assert(D2RL::Items::ExistingItemTransactionRequiredSize == 32);
static_assert(D2RL::Items::ExistingItemTransactionResultRequiredSize == 16);
static_assert(D2RL::Items::SplitStackRequestRequiredSize == 32);
static_assert(D2RL::Items::SplitStackResultRequiredSize == 24);
static_assert(offsetof(D2RL::Items::SplitStackRequest, sourceItem) == 16);
static_assert(offsetof(D2RL::Items::SplitStackResult, cursorItem) == 8);
static_assert(D2RL::Items::AffixAugmentRequestRequiredSize == 64);
static_assert(D2RL::Items::AffixAugmentResultRequiredSize == 40);
static_assert(offsetof(D2RL::Items::AffixAugmentRequest, paymentItem) == 24);
static_assert(offsetof(D2RL::Items::AffixAugmentRequest, affixId) == 44);
static_assert(offsetof(D2RL::Items::AffixAugmentResult, appliedAffixId) == 16);
static_assert(D2RL::ItemServiceSize == 88);
static_assert(D2RL::ItemServiceCapabilitiesFieldEnd == 80);
static_assert(D2RL::ItemServiceAffixAugmentFieldEnd == 88);
static_assert(D2RL::Items::ItemDestinationRequiredSize == 32);
static_assert(D2RL::Items::ItemDestinationSharedStashPageFieldEnd == 36);
static_assert(D2RL::Items::ItemDestinationSize == 40);
static_assert(D2RL::Items::ItemCreateSpecRequiredSize == 120);
static_assert(D2RL::Items::ItemCreateSpecSharedStashPageFieldEnd == 124);
static_assert(D2RL::Items::ItemCreateSpecSize == 128);
static_assert(D2RL::Items::ExistingItemOperationSharedStashPageFieldEnd == 60);
static_assert(D2RL::Items::MaxExistingItemOperations == 4'096);
static_assert(D2RL::Items::ItemServiceCapabilityBit(D2RL::Items::ItemServiceCapability::SharedStashWrite) == 1ULL);
static_assert(D2RL::Items::ItemServiceCapabilityBit(D2RL::Items::ItemServiceCapability::AffixAugment) == 2ULL);
inline constexpr D2RL::ItemService CurrentItemService {
	.serviceSize    = D2RL::ItemServiceSize,
	.serviceVersion = D2RL::ItemService::AbiVersion,
	.capabilities
	= D2RL::Items::ItemServiceCapabilityBit(D2RL::Items::ItemServiceCapability::SharedStashWrite) | D2RL::Items::ItemServiceCapabilityBit(D2RL::Items::ItemServiceCapability::AffixAugment),
};
inline constexpr D2RL::ItemService LegacyItemService {
	.serviceSize    = D2RL::ItemServiceSplitStackFieldEnd,
	.serviceVersion = D2RL::ItemService::AbiVersion,
	.capabilities   = D2RL::Items::ItemServiceCapabilityBit(D2RL::Items::ItemServiceCapability::SharedStashWrite),
};
static_assert(D2RL::HasItemServiceCapability(&CurrentItemService, D2RL::Items::ItemServiceCapability::SharedStashWrite));
static_assert(D2RL::HasItemServiceCapability(&CurrentItemService, D2RL::Items::ItemServiceCapability::AffixAugment));
static_assert(!D2RL::HasItemServiceCapability(&LegacyItemService, D2RL::Items::ItemServiceCapability::SharedStashWrite));
static_assert(!D2RL::HasItemServiceCapability(&LegacyItemService, D2RL::Items::ItemServiceCapability::AffixAugment));
static_assert(static_cast<uint32_t>(D2RL::Items::AffixSelection::RandomEligible) == 0);
static_assert(static_cast<uint32_t>(D2RL::Items::AffixSelection::ExplicitId) == 1);
static_assert(static_cast<uint32_t>(D2RL::Items::AffixKind::Either) == 0);
static_assert(static_cast<uint32_t>(D2RL::Items::AffixKind::Prefix) == 1);
static_assert(static_cast<uint32_t>(D2RL::Items::AffixKind::Suffix) == 2);
static_assert(static_cast<uint32_t>(D2RL::Items::AffixAugmentFailure::RollbackFailed) == 11);
static_assert(D2RL::HttpServiceRequiredSize == 24);
static_assert(D2RL::Http::RequestRequiredSize == 56);
static_assert(D2RL::Http::ResponseRequiredSize == 48);
static_assert(D2RL::NetworkServiceRequiredSize == 64);
static_assert(D2RL::Lifecycle::GameplayEventRequiredSize == 40);
static_assert(D2RL::SharedEvents::UiMessageEventRequiredSize == 56);
static_assert(D2RL::Items::AllItemCreateFlags == D2RL::Items::ItemCreateFlagBit(D2RL::Items::ItemCreateFlag::AllowDuplicateUnique));
static_assert(static_cast<uint32_t>(D2RL::Lifecycle::GameplayEventKind::PlayerLevelChanged) == 7);
static_assert(static_cast<uint32_t>(D2RL::Lifecycle::GameplayEventKind::QuestCompleted) == 8);
static_assert(static_cast<uint32_t>(D2RL::SharedEvents::CharacterHardcoreMode::Hardcore) == 2);
static_assert(D2RL::SharedEvents::ItemTooltipEventRequiredSize == 32);
static_assert(D2RL::SharedEvents::ItemTooltipListenerRequiredSize == 48);
static_assert(static_cast<uint32_t>(D2RL::SharedEvents::ItemTooltipRegion::ActionFooter) == 2);
static_assert(static_cast<uint32_t>(D2RL::SharedEvents::ItemTooltipPosition::BelowAnchor) == 3);
static_assert(static_cast<uint32_t>(D2RL::SharedEvents::ItemTooltipAnchor::Sockets) == 10);
static_assert(static_cast<uint32_t>(D2RL::SharedEvents::ItemTooltipFallback::RegionBottom) == 2);

auto main() -> int {
	return D2RL::InvalidPlayerHandle == 0 && D2RL::InvalidItemHandle == 0 ? 0 : 1;
}
