#include <D2RLPlugin/api.h>

static_assert(static_cast<uint32_t>(D2RL::ServiceId::Item) == 15);
static_assert(D2RL::Items::MakeItemCode("r01") == 0x20313072);
static_assert(D2RL::Items::MakeItemCode("cap") == 0x20706163);
static_assert(D2RL::Items::MakeItemCode("abcd") == 0x64636261);
static_assert(D2RL::Items::MakeExactProperty(0, 25).minimum == 25);
static_assert(D2RL::Items::MakeExactProperty(0, 25).maximum == 25);
static_assert(D2RL::Items::TransactionRequiredSize == D2RL::Items::TransactionSize);
static_assert(D2RL::Items::TransactionResultRequiredSize == D2RL::Items::TransactionResultSize);
static_assert(D2RL::InventoryServiceV1RequiredSize == 112);
static_assert(D2RL::ItemServiceV1RequiredSize == 56);
static_assert(D2RL::NetworkServiceV1RequiredSize == 64);
static_assert(D2RL::SharedEvents::ItemTooltipEventRequiredSize == 32);
static_assert(D2RL::SharedEvents::ItemTooltipListenerRequiredSize == 48);
static_assert(static_cast<uint32_t>(D2RL::SharedEvents::ItemTooltipRegion::ActionFooter) == 2);
static_assert(static_cast<uint32_t>(D2RL::SharedEvents::ItemTooltipPosition::BelowAnchor) == 3);
static_assert(static_cast<uint32_t>(D2RL::SharedEvents::ItemTooltipAnchor::Sockets) == 10);
static_assert(static_cast<uint32_t>(D2RL::SharedEvents::ItemTooltipFallback::RegionBottom) == 2);

auto main() -> int {
	return D2RL::InvalidPlayerHandle == 0 && D2RL::InvalidItemHandle == 0 ? 0 : 1;
}
