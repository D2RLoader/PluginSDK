#pragma once

#include <D2RLPlugin/services.h>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace D2RL {

struct PluginContext;

namespace Widgets {

using WidgetHandle = uint64_t;

inline constexpr WidgetHandle InvalidHandle = 0;

// Widget handles are plugin-owned paths, not native pointers. D2RLoader resolves
// the path on every call, so the same handle survives closing and reopening the
// panel.

enum class Result : uint32_t {
	Success         = 0,
	InvalidArgument = 1,
	Unsupported     = 2,
	Unavailable     = 3,
	NotFound        = 4,
	Busy            = 5,
	OwnerInactive   = 6,
	OwnerMismatch   = 7,
	StaleHandle     = 8,
	CallbackFault   = 9,
};

struct Rect {
	int32_t x;
	int32_t y;
	int32_t width;
	int32_t height;
};

// Rect coordinates are local to the widget's parent.

struct UiAction {
	uint32_t    structSize;
	uint32_t    flags;
	const char* target;
	const char* command;
	const char* text;
};

inline constexpr uint32_t UiActionSize         = static_cast<uint32_t>(sizeof(UiAction));
inline constexpr uint32_t UiActionRequiredSize = UiActionSize;

using FindPanelFn        = Result(__cdecl*)(const PluginContext* context, const char* name, WidgetHandle* handle) noexcept;
using FindWidgetFn       = Result(__cdecl*)(const PluginContext* context, WidgetHandle parent, const char* name, WidgetHandle* handle) noexcept;
using GetWidgetRectFn    = Result(__cdecl*)(const PluginContext* context, WidgetHandle handle, Rect* rect) noexcept;
using SetWidgetVisibleFn = Result(__cdecl*)(const PluginContext* context, WidgetHandle handle, bool visible) noexcept;
using SetWidgetEnabledFn = Result(__cdecl*)(const PluginContext* context, WidgetHandle handle, bool enabled) noexcept;
using DispatchUiActionFn = Result(__cdecl*)(const PluginContext* context, const UiAction* action) noexcept;

// Every widget call must run on the UI thread, normally through
// ThreadServiceV1::runOnUiThread. findWidget searches below its parent handle.
// dispatchUiAction broadcasts the same target/command/text shape used by native
// panel messages.

static_assert(sizeof(Result) == sizeof(uint32_t));
static_assert(std::is_standard_layout_v<Rect> && std::is_trivially_copyable_v<Rect>);
static_assert(std::is_standard_layout_v<UiAction> && std::is_trivially_copyable_v<UiAction>);
static_assert(sizeof(Rect) == 16);
static_assert(sizeof(UiAction) == 32);

}

struct WidgetServiceV1 {
	uint32_t                    serviceSize;
	uint32_t                    serviceVersion;
	Widgets::FindPanelFn        findPanel;
	Widgets::FindWidgetFn       findWidget;
	Widgets::GetWidgetRectFn    getWidgetRect;
	Widgets::SetWidgetVisibleFn setWidgetVisible;
	Widgets::SetWidgetEnabledFn setWidgetEnabled;
	Widgets::DispatchUiActionFn dispatchUiAction;
};

inline constexpr uint32_t WidgetServiceV1Version      = 1;
inline constexpr uint32_t WidgetServiceV1Size         = static_cast<uint32_t>(sizeof(WidgetServiceV1));
inline constexpr uint32_t WidgetServiceV1RequiredSize = WidgetServiceV1Size;

inline auto HasWidgetServiceV1Field(const WidgetServiceV1* service, uint32_t fieldEndOffset) noexcept -> bool {
	return service != nullptr && service->serviceVersion == WidgetServiceV1Version && service->serviceSize >= fieldEndOffset;
}

static_assert(std::is_standard_layout_v<WidgetServiceV1> && std::is_trivially_copyable_v<WidgetServiceV1>);
static_assert(sizeof(WidgetServiceV1) == 56);

}
