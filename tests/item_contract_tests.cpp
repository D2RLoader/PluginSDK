#include <D2RLPlugin/api.h>

static_assert(static_cast<uint32_t>(D2RL::ServiceId::Item) == 15);
static_assert(static_cast<uint32_t>(D2RL::ServiceId::Http) == 16);
static_assert(static_cast<uint32_t>(D2RL::ServiceId::ItemInteraction) == 17);
static_assert(D2RL_PLUGIN_API_VERSION == 4);
static_assert(D2RL::Items::MakeItemCode("r01") == 0x20313072);
static_assert(D2RL::Items::MakeItemCode("cap") == 0x20706163);
static_assert(D2RL::Items::MakeItemCode("abcd") == 0x64636261);
static_assert(D2RL::Items::MakeExactProperty(0, 25).minimum == 25);
static_assert(D2RL::Items::MakeExactProperty(0, 25).maximum == 25);
static_assert(D2RL::Items::TransactionRequiredSize == D2RL::Items::TransactionSize);
static_assert(D2RL::Items::TransactionResultRequiredSize == D2RL::Items::TransactionResultSize);
static_assert(D2RL::InventoryServiceV1RequiredSize == 112);
static_assert(D2RL::ItemServiceV1RequiredSize == 64);
static_assert(D2RL::ItemInteractionServiceV1RequiredSize == 24);
static_assert(D2RL::ItemInteractions::ItemInteractionEventRequiredSize == 56);
static_assert(D2RL::ItemInteractions::ItemInteractionListenerRequiredSize == 32);
static_assert(D2RL::Items::ExistingItemOperationRequiredSize == 56);
static_assert(D2RL::Items::ExistingItemTransactionRequiredSize == 32);
static_assert(D2RL::Items::ExistingItemTransactionResultRequiredSize == 16);
static_assert(D2RL::HttpServiceV1RequiredSize == 24);
static_assert(D2RL::Http::RequestRequiredSize == 56);
static_assert(D2RL::Http::ResponseRequiredSize == 48);
static_assert(D2RL::NetworkServiceV1RequiredSize == 64);
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
