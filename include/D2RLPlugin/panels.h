#pragma once

#include <D2RLPlugin/inventory.h>
#include <D2RLPlugin/services.h>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace D2RL {

struct PluginContext;

namespace Panels {

using RegistrationHandle    = uint64_t;
using ChildLayoutHandle     = uint64_t;
using ControllerRouteHandle = uint64_t;

inline constexpr RegistrationHandle    InvalidHandle                = 0;
inline constexpr ChildLayoutHandle     InvalidChildLayoutHandle     = 0;
inline constexpr ControllerRouteHandle InvalidControllerRouteHandle = 0;

enum class Result : uint32_t {
	Success         = 0,
	InvalidArgument = 1,
	Unsupported     = 2,
	Unavailable     = 3,
	Conflict        = 4,
	NotFound        = 5,
	Busy            = 6,
	OwnerInactive   = 7,
	OwnerMismatch   = 8,
	StaleHandle     = 9,
	CallbackFault   = 10,
};

enum class RegistrationState : uint32_t {
	Unknown = 0,
	Staged  = 1,
	Active  = 2,
	Closing = 3,
};

enum class PresentationState : uint32_t {
	Unknown = 0,
	Closed  = 1,
	Open    = 2,
};

enum class PanelFlags : uint32_t {
	None             = 0,
	CloseOnEscape    = 0x00000001U,
	GameplayLeftSlot = 0x00000002U,
};

enum class ChildLayoutFlags : uint32_t {
	None              = 0,
	KeyboardMouseOnly = 0x00000001U,
	ControllerOnly    = 0x00000002U,
};

enum class StockPanel : uint32_t {
	Unknown         = 0,
	PlayerInventory = 1,
};

enum class ControllerRouteFlags : uint32_t {
	None                    = 0,
	PairWithPlayerInventory = 0x00000001U,
};

enum class ControllerRoutePlacement : uint32_t {
	Unknown              = 0,
	AfterPlayerInventory = 1,
};

constexpr auto FlagsValue(PanelFlags flags) noexcept -> uint32_t {
	return static_cast<uint32_t>(flags);
}

constexpr auto operator |(PanelFlags lhs, PanelFlags rhs) noexcept -> PanelFlags {
	return static_cast<PanelFlags>(FlagsValue(lhs) | FlagsValue(rhs));
}

constexpr auto operator &(PanelFlags lhs, PanelFlags rhs) noexcept -> PanelFlags {
	return static_cast<PanelFlags>(FlagsValue(lhs) & FlagsValue(rhs));
}

constexpr auto HasFlag(PanelFlags flags, PanelFlags flag) noexcept -> bool {
	return FlagsValue(flags & flag) != 0;
}

constexpr auto FlagsValue(ChildLayoutFlags flags) noexcept -> uint32_t {
	return static_cast<uint32_t>(flags);
}

constexpr auto operator |(ChildLayoutFlags lhs, ChildLayoutFlags rhs) noexcept -> ChildLayoutFlags {
	return static_cast<ChildLayoutFlags>(FlagsValue(lhs) | FlagsValue(rhs));
}

constexpr auto operator &(ChildLayoutFlags lhs, ChildLayoutFlags rhs) noexcept -> ChildLayoutFlags {
	return static_cast<ChildLayoutFlags>(FlagsValue(lhs) & FlagsValue(rhs));
}

constexpr auto HasFlag(ChildLayoutFlags flags, ChildLayoutFlags flag) noexcept -> bool {
	return FlagsValue(flags & flag) != 0;
}

constexpr auto FlagsValue(ControllerRouteFlags flags) noexcept -> uint32_t {
	return static_cast<uint32_t>(flags);
}

constexpr auto operator |(ControllerRouteFlags lhs, ControllerRouteFlags rhs) noexcept -> ControllerRouteFlags {
	return static_cast<ControllerRouteFlags>(FlagsValue(lhs) | FlagsValue(rhs));
}

constexpr auto operator &(ControllerRouteFlags lhs, ControllerRouteFlags rhs) noexcept -> ControllerRouteFlags {
	return static_cast<ControllerRouteFlags>(FlagsValue(lhs) & FlagsValue(rhs));
}

constexpr auto HasFlag(ControllerRouteFlags flags, ControllerRouteFlags flag) noexcept -> bool {
	return FlagsValue(flags & flag) != 0;
}

// A missing cached widget reports Closed. Unknown means its native state is
// inconsistent: visible without active, or active without visible.

// Register panels from D2RLoaderLoadPlugin. Set structSize to
// PanelRegistrationSize. localId belongs to the calling plugin and uses 1-64
// ASCII letters, digits, hyphens, or underscores. Do not use a slash. D2RLoader
// copies localId before registerPanel returns.
// The matching layout's root type must be Panel or a derived panel type, and
// its root name must be exactly <plugin-id>/<localId>. A generic Widget root is
// a valid layout resource but is not a panel and will be rejected when opened.
// CloseOnEscape lets Escape close an open panel.
// GameplayLeftSlot is a player-inventory composition: the plugin panel occupies
// the left side and player inventory occupies the right. Opening it closes
// incompatible stock compositions using UISwitcher-style behavior. If D2RCore
// had to open player inventory, closing the plugin panel closes it again;
// inventory that was already open remains open.
struct PanelRegistration {
		uint32_t    structSize;
		PanelFlags  flags;
		const char* localId;
};

// Set structSize to PanelInfoSize before calling getPanelInfo. The panel and
// its handle remain owned by D2RCore.
struct PanelInfo {
		uint32_t           structSize;
		PanelFlags         flags;
		RegistrationHandle handle;
		uint64_t           ownerGeneration;
		RegistrationState  registrationState;
		PresentationState  presentationState;
		uint32_t           reserved;
};

// Bind an InventoryGridWidget child to a registered player page. Both handles
// must belong to the calling plugin. D2RCore copies childName before returning
// and applies the binding whenever the panel opens. D2RCore still owns the
// panel, page, and native widget. flags is zero in v1.
struct PlayerPageGridBinding {
		uint32_t                          structSize;
		uint32_t                          flags;
		RegistrationHandle                panel;
		Inventory::PageRegistrationHandle page;
		const char*                       childName;
};

// Register during plugin load. localId uses the same plugin-local naming rules
// as PanelRegistration and resolves to <plugin-id>/<localId>.
// Panel and child layouts cannot reuse one localId. D2RCore loads that widget
// layout as a child of the selected stock panel. KeyboardMouseOnly excludes it
// from controller layouts; ControllerOnly excludes it from keyboard/mouse
// layouts. The two flags cannot be combined. reserved must be zero in v1.
struct ChildLayoutRegistration {
		uint32_t         structSize;
		ChildLayoutFlags flags;
		StockPanel       stockPanel;
		uint32_t         reserved;
		const char*      localId;
};

// Add a registered panel to the controller UI switcher during plugin load.
// The registered panel is the primary left panel; PairWithPlayerInventory
// opens the stock player inventory as its separate right-side companion and
// closes that companion with the route, matching the hireling-inventory
// lifecycle. The panel must belong to the calling plugin and still be staged.
// D2RCore copies localizedLabelKey before this call returns. In v1, flags must
// be PairWithPlayerInventory, placement must be AfterPlayerInventory, reserved
// must be zero, and localizedLabelKey must be a non-empty D2R string key that
// begins with '@'. Only one controller route can be registered globally.
struct ControllerRouteRegistration {
		uint32_t                 structSize;
		ControllerRouteFlags     flags;
		RegistrationHandle       panel;
		ControllerRoutePlacement placement;
		uint32_t                 reserved;
		const char*              localizedLabelKey;
};

inline constexpr uint32_t PanelRegistrationSize                   = static_cast<uint32_t>(sizeof(PanelRegistration));
inline constexpr uint32_t PanelRegistrationLocalIdFieldEnd        = static_cast<uint32_t>(offsetof(PanelRegistration, localId) + sizeof(const char*));
inline constexpr uint32_t PanelRegistrationRequiredSize           = PanelRegistrationLocalIdFieldEnd;
inline constexpr uint32_t PanelInfoSize                           = static_cast<uint32_t>(sizeof(PanelInfo));
inline constexpr uint32_t PanelInfoHandleFieldEnd                 = static_cast<uint32_t>(offsetof(PanelInfo, handle) + sizeof(RegistrationHandle));
inline constexpr uint32_t PanelInfoOwnerGenerationFieldEnd        = static_cast<uint32_t>(offsetof(PanelInfo, ownerGeneration) + sizeof(uint64_t));
inline constexpr uint32_t PanelInfoRegistrationStateFieldEnd      = static_cast<uint32_t>(offsetof(PanelInfo, registrationState) + sizeof(RegistrationState));
inline constexpr uint32_t PanelInfoPresentationStateFieldEnd      = static_cast<uint32_t>(offsetof(PanelInfo, presentationState) + sizeof(PresentationState));
inline constexpr uint32_t PanelInfoRequiredSize                   = PanelInfoPresentationStateFieldEnd;
inline constexpr uint32_t PlayerPageGridBindingSize               = static_cast<uint32_t>(sizeof(PlayerPageGridBinding));
inline constexpr uint32_t PlayerPageGridBindingRequiredSize       = static_cast<uint32_t>(offsetof(PlayerPageGridBinding, childName) + sizeof(const char*));
inline constexpr uint32_t ChildLayoutRegistrationSize             = static_cast<uint32_t>(sizeof(ChildLayoutRegistration));
inline constexpr uint32_t ChildLayoutRegistrationRequiredSize     = static_cast<uint32_t>(offsetof(ChildLayoutRegistration, localId) + sizeof(const char*));
inline constexpr uint32_t ControllerRouteRegistrationSize         = static_cast<uint32_t>(sizeof(ControllerRouteRegistration));
inline constexpr uint32_t ControllerRouteRegistrationRequiredSize = static_cast<uint32_t>(offsetof(ControllerRouteRegistration, localizedLabelKey) + sizeof(const char*));

inline auto HasPanelRegistrationField(const PanelRegistration* registration, uint32_t fieldEndOffset) noexcept -> bool {
	return registration != nullptr && registration->structSize >= fieldEndOffset;
}

inline auto HasPanelInfoField(const PanelInfo* info, uint32_t fieldEndOffset) noexcept -> bool {
	return info != nullptr && info->structSize >= fieldEndOffset;
}

inline auto HasPlayerPageGridBindingField(const PlayerPageGridBinding* binding, uint32_t fieldEndOffset) noexcept -> bool {
	return binding != nullptr && binding->structSize >= fieldEndOffset;
}

inline auto HasChildLayoutRegistrationField(const ChildLayoutRegistration* registration, uint32_t fieldEndOffset) noexcept -> bool {
	return registration != nullptr && registration->structSize >= fieldEndOffset;
}

inline auto HasControllerRouteRegistrationField(const ControllerRouteRegistration* registration, uint32_t fieldEndOffset) noexcept -> bool {
	return registration != nullptr && registration->structSize >= fieldEndOffset;
}

using RegisterPanelFn             = Result(__cdecl*)(const PluginContext* context, const PanelRegistration* registration, RegistrationHandle* handle) noexcept;
// Unregister closes the panel and removes its registration. D2R may keep the
// widget in its internal cache. Registering again is not a layout hot reload;
// restart the game before testing changed panel JSON.
using UnregisterPanelFn           = Result(__cdecl*)(const PluginContext* context, RegistrationHandle handle) noexcept;
using GetPanelInfoFn              = Result(__cdecl*)(const PluginContext* context, RegistrationHandle handle, PanelInfo* info) noexcept;
using OpenPanelFn                 = Result(__cdecl*)(const PluginContext* context, RegistrationHandle handle) noexcept;
using ClosePanelFn                = Result(__cdecl*)(const PluginContext* context, RegistrationHandle handle) noexcept;
using TogglePanelFn               = Result(__cdecl*)(const PluginContext* context, RegistrationHandle handle) noexcept;
using BindPlayerPageGridFn        = Result(__cdecl*)(const PluginContext* context, const PlayerPageGridBinding* binding) noexcept;
using RegisterChildLayoutFn       = Result(__cdecl*)(const PluginContext* context, const ChildLayoutRegistration* registration, ChildLayoutHandle* handle) noexcept;
using UnregisterChildLayoutFn     = Result(__cdecl*)(const PluginContext* context, ChildLayoutHandle handle) noexcept;
using RegisterControllerRouteFn   = Result(__cdecl*)(const PluginContext* context, const ControllerRouteRegistration* registration, ControllerRouteHandle* handle) noexcept;
using UnregisterControllerRouteFn = Result(__cdecl*)(const PluginContext* context, ControllerRouteHandle handle) noexcept;

// Active operations run on the captured panel/UI thread. Busy means the caller
// is on the wrong thread, the registration is staged or closing, or D2R did not
// reach the requested visible and active state. Unavailable means the root UI
// is absent, native UI access or allocation failed, or the plugin reached the
// 64-panel limit. Open and toggle return NotFound when the JSON is missing,
// invalid, not rooted at a Panel, named incorrectly, or otherwise cannot load.

static_assert(sizeof(RegistrationHandle) == sizeof(uint64_t));
static_assert(sizeof(ChildLayoutHandle) == sizeof(uint64_t));
static_assert(sizeof(ControllerRouteHandle) == sizeof(uint64_t));
static_assert(sizeof(Result) == sizeof(uint32_t));
static_assert(sizeof(RegistrationState) == sizeof(uint32_t));
static_assert(sizeof(PresentationState) == sizeof(uint32_t));
static_assert(sizeof(PanelFlags) == sizeof(uint32_t));
static_assert(sizeof(ChildLayoutFlags) == sizeof(uint32_t));
static_assert(sizeof(StockPanel) == sizeof(uint32_t));
static_assert(sizeof(ControllerRouteFlags) == sizeof(uint32_t));
static_assert(sizeof(ControllerRoutePlacement) == sizeof(uint32_t));
static_assert(std::is_standard_layout_v<PanelRegistration>);
static_assert(std::is_trivially_copyable_v<PanelRegistration>);
static_assert(std::is_standard_layout_v<PanelInfo>);
static_assert(std::is_trivially_copyable_v<PanelInfo>);
static_assert(std::is_standard_layout_v<PlayerPageGridBinding>);
static_assert(std::is_trivially_copyable_v<PlayerPageGridBinding>);
static_assert(std::is_standard_layout_v<ChildLayoutRegistration>);
static_assert(std::is_trivially_copyable_v<ChildLayoutRegistration>);
static_assert(std::is_standard_layout_v<ControllerRouteRegistration>);
static_assert(std::is_trivially_copyable_v<ControllerRouteRegistration>);
static_assert(offsetof(PanelRegistration, structSize) == 0);
static_assert(offsetof(PanelRegistration, flags) == 4);
static_assert(offsetof(PanelRegistration, localId) == 8);
static_assert(PanelRegistrationLocalIdFieldEnd == 16);
static_assert(PanelRegistrationRequiredSize == 16);
static_assert(sizeof(PanelRegistration) == 16);
static_assert(offsetof(PanelInfo, structSize) == 0);
static_assert(offsetof(PanelInfo, flags) == 4);
static_assert(offsetof(PanelInfo, handle) == 8);
static_assert(offsetof(PanelInfo, ownerGeneration) == 16);
static_assert(offsetof(PanelInfo, registrationState) == 24);
static_assert(offsetof(PanelInfo, presentationState) == 28);
static_assert(offsetof(PanelInfo, reserved) == 32);
static_assert(PanelInfoHandleFieldEnd == 16);
static_assert(PanelInfoOwnerGenerationFieldEnd == 24);
static_assert(PanelInfoRegistrationStateFieldEnd == 28);
static_assert(PanelInfoPresentationStateFieldEnd == 32);
static_assert(PanelInfoRequiredSize == 32);
static_assert(sizeof(PanelInfo) == 40);
static_assert(offsetof(PlayerPageGridBinding, structSize) == 0);
static_assert(offsetof(PlayerPageGridBinding, flags) == 4);
static_assert(offsetof(PlayerPageGridBinding, panel) == 8);
static_assert(offsetof(PlayerPageGridBinding, page) == 16);
static_assert(offsetof(PlayerPageGridBinding, childName) == 24);
static_assert(PlayerPageGridBindingRequiredSize == 32);
static_assert(sizeof(PlayerPageGridBinding) == 32);
static_assert(offsetof(ChildLayoutRegistration, structSize) == 0);
static_assert(offsetof(ChildLayoutRegistration, flags) == 4);
static_assert(offsetof(ChildLayoutRegistration, stockPanel) == 8);
static_assert(offsetof(ChildLayoutRegistration, reserved) == 12);
static_assert(offsetof(ChildLayoutRegistration, localId) == 16);
static_assert(ChildLayoutRegistrationRequiredSize == 24);
static_assert(sizeof(ChildLayoutRegistration) == 24);
static_assert(offsetof(ControllerRouteRegistration, structSize) == 0);
static_assert(offsetof(ControllerRouteRegistration, flags) == 4);
static_assert(offsetof(ControllerRouteRegistration, panel) == 8);
static_assert(offsetof(ControllerRouteRegistration, placement) == 16);
static_assert(offsetof(ControllerRouteRegistration, reserved) == 20);
static_assert(offsetof(ControllerRouteRegistration, localizedLabelKey) == 24);
static_assert(ControllerRouteRegistrationRequiredSize == 32);
static_assert(sizeof(ControllerRouteRegistration) == 32);

}

struct PanelServiceV1 {
		uint32_t                            serviceSize;
		uint32_t                            serviceVersion;
		Panels::RegisterPanelFn             registerPanel;
		Panels::UnregisterPanelFn           unregisterPanel;
		Panels::GetPanelInfoFn              getPanelInfo;
		Panels::OpenPanelFn                 openPanel;
		Panels::ClosePanelFn                closePanel;
		Panels::TogglePanelFn               togglePanel;
		Panels::BindPlayerPageGridFn        bindPlayerPageGrid;
		Panels::RegisterChildLayoutFn       registerChildLayout;
		Panels::UnregisterChildLayoutFn     unregisterChildLayout;
		Panels::RegisterControllerRouteFn   registerControllerRoute;
		Panels::UnregisterControllerRouteFn unregisterControllerRoute;
};

inline constexpr uint32_t PanelServiceV1Version                           = 1;
inline constexpr uint32_t PanelServiceV1Size                              = static_cast<uint32_t>(sizeof(PanelServiceV1));
inline constexpr uint32_t PanelServiceV1RegisterPanelFieldEnd             = static_cast<uint32_t>(offsetof(PanelServiceV1, registerPanel) + sizeof(Panels::RegisterPanelFn));
inline constexpr uint32_t PanelServiceV1UnregisterPanelFieldEnd           = static_cast<uint32_t>(offsetof(PanelServiceV1, unregisterPanel) + sizeof(Panels::UnregisterPanelFn));
inline constexpr uint32_t PanelServiceV1GetPanelInfoFieldEnd              = static_cast<uint32_t>(offsetof(PanelServiceV1, getPanelInfo) + sizeof(Panels::GetPanelInfoFn));
inline constexpr uint32_t PanelServiceV1OpenPanelFieldEnd                 = static_cast<uint32_t>(offsetof(PanelServiceV1, openPanel) + sizeof(Panels::OpenPanelFn));
inline constexpr uint32_t PanelServiceV1ClosePanelFieldEnd                = static_cast<uint32_t>(offsetof(PanelServiceV1, closePanel) + sizeof(Panels::ClosePanelFn));
inline constexpr uint32_t PanelServiceV1TogglePanelFieldEnd               = static_cast<uint32_t>(offsetof(PanelServiceV1, togglePanel) + sizeof(Panels::TogglePanelFn));
inline constexpr uint32_t PanelServiceV1BindPlayerPageGridFieldEnd        = static_cast<uint32_t>(offsetof(PanelServiceV1, bindPlayerPageGrid) + sizeof(Panels::BindPlayerPageGridFn));
inline constexpr uint32_t PanelServiceV1RegisterChildLayoutFieldEnd       = static_cast<uint32_t>(offsetof(PanelServiceV1, registerChildLayout) + sizeof(Panels::RegisterChildLayoutFn));
inline constexpr uint32_t PanelServiceV1UnregisterChildLayoutFieldEnd     = static_cast<uint32_t>(offsetof(PanelServiceV1, unregisterChildLayout) + sizeof(Panels::UnregisterChildLayoutFn));
inline constexpr uint32_t PanelServiceV1RegisterControllerRouteFieldEnd   = static_cast<uint32_t>(offsetof(PanelServiceV1, registerControllerRoute) + sizeof(Panels::RegisterControllerRouteFn));
inline constexpr uint32_t PanelServiceV1UnregisterControllerRouteFieldEnd = static_cast<uint32_t>(offsetof(PanelServiceV1, unregisterControllerRoute) + sizeof(Panels::UnregisterControllerRouteFn));
inline constexpr uint32_t PanelServiceV1RequiredSize                      = PanelServiceV1Size;

inline auto HasPanelServiceV1Field(const PanelServiceV1* service, uint32_t fieldEndOffset) noexcept -> bool {
	return service != nullptr && service->serviceVersion == PanelServiceV1Version && service->serviceSize >= fieldEndOffset;
}

static_assert(std::is_standard_layout_v<PanelServiceV1>);
static_assert(std::is_trivially_copyable_v<PanelServiceV1>);
static_assert(offsetof(PanelServiceV1, serviceSize) == 0);
static_assert(offsetof(PanelServiceV1, serviceVersion) == 4);
static_assert(offsetof(PanelServiceV1, registerPanel) == 8);
static_assert(offsetof(PanelServiceV1, unregisterPanel) == 16);
static_assert(offsetof(PanelServiceV1, getPanelInfo) == 24);
static_assert(offsetof(PanelServiceV1, openPanel) == 32);
static_assert(offsetof(PanelServiceV1, closePanel) == 40);
static_assert(offsetof(PanelServiceV1, togglePanel) == 48);
static_assert(offsetof(PanelServiceV1, bindPlayerPageGrid) == 56);
static_assert(offsetof(PanelServiceV1, registerChildLayout) == 64);
static_assert(offsetof(PanelServiceV1, unregisterChildLayout) == 72);
static_assert(offsetof(PanelServiceV1, registerControllerRoute) == 80);
static_assert(offsetof(PanelServiceV1, unregisterControllerRoute) == 88);
static_assert(PanelServiceV1RegisterPanelFieldEnd == 16);
static_assert(PanelServiceV1UnregisterPanelFieldEnd == 24);
static_assert(PanelServiceV1GetPanelInfoFieldEnd == 32);
static_assert(PanelServiceV1OpenPanelFieldEnd == 40);
static_assert(PanelServiceV1ClosePanelFieldEnd == 48);
static_assert(PanelServiceV1TogglePanelFieldEnd == 56);
static_assert(PanelServiceV1BindPlayerPageGridFieldEnd == 64);
static_assert(PanelServiceV1RegisterChildLayoutFieldEnd == 72);
static_assert(PanelServiceV1UnregisterChildLayoutFieldEnd == 80);
static_assert(PanelServiceV1RegisterControllerRouteFieldEnd == 88);
static_assert(PanelServiceV1UnregisterControllerRouteFieldEnd == 96);
static_assert(PanelServiceV1RequiredSize == 96);
static_assert(sizeof(PanelServiceV1) == 96);

}
