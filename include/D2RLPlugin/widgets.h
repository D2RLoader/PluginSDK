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
	BufferTooSmall  = 10,
};

struct Rect {
	int32_t x;
	int32_t y;
	int32_t width;
	int32_t height;
};

// Screen coordinates include parent positions, anchors, and UI scaling.
struct ScreenRect {
	float x;
	float y;
	float width;
	float height;
};

struct State {
	bool visible;
	bool enabled;
	bool focused;
};

enum class EventKind : uint32_t {
	Click           = 1,
	InputChanged    = 2,
	InputReturn     = 3,
	DropdownChanged = 4,
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

using FindPanelFn                = Result(__cdecl*)(const PluginContext* context, const char* name, WidgetHandle* handle) noexcept;
using FindWidgetFn               = Result(__cdecl*)(const PluginContext* context, WidgetHandle parent, const char* name, WidgetHandle* handle) noexcept;
using GetWidgetRectFn            = Result(__cdecl*)(const PluginContext* context, WidgetHandle handle, Rect* rect) noexcept;
using SetWidgetVisibleFn         = Result(__cdecl*)(const PluginContext* context, WidgetHandle handle, bool visible) noexcept;
using SetWidgetEnabledFn         = Result(__cdecl*)(const PluginContext* context, WidgetHandle handle, bool enabled) noexcept;
using DispatchUiActionFn         = Result(__cdecl*)(const PluginContext* context, const UiAction* action) noexcept;
using GetInputTextFn             = Result(__cdecl*)(const PluginContext* context, WidgetHandle handle, char* output, uint32_t outputSize, uint32_t* requiredSize) noexcept;
using SetTextFn                  = Result(__cdecl*)(const PluginContext* context, WidgetHandle handle, const char* text) noexcept;
using SetTooltipTextFn           = Result(__cdecl*)(const PluginContext* context, WidgetHandle handle, const char* text) noexcept;
using SetImageSpriteFn           = Result(__cdecl*)(const PluginContext* context, WidgetHandle handle, const char* filename, const Rect* box) noexcept;
using GetDropdownSelectionFn     = Result(__cdecl*)(const PluginContext* context, WidgetHandle handle, uint32_t* index) noexcept;
using SetDropdownUpdateMessageFn = Result(__cdecl*)(const PluginContext* context, WidgetHandle handle, const char* target, const char* command) noexcept;
using GetWidgetStateFn           = Result(__cdecl*)(const PluginContext* context, WidgetHandle handle, State* state) noexcept;
using GetTextFn                  = Result(__cdecl*)(const PluginContext* context, WidgetHandle handle, char* output, uint32_t outputSize, uint32_t* requiredSize) noexcept;
using SetInputTextFn             = Result(__cdecl*)(const PluginContext* context, WidgetHandle handle, const char* text) noexcept;
using ClearInputTextFn           = Result(__cdecl*)(const PluginContext* context, WidgetHandle handle) noexcept;
using FocusWidgetFn              = Result(__cdecl*)(const PluginContext* context, WidgetHandle handle) noexcept;
using SetWidgetRectFn            = Result(__cdecl*)(const PluginContext* context, WidgetHandle handle, const Rect* rect) noexcept;
using SetDropdownSelectionFn     = Result(__cdecl*)(const PluginContext* context, WidgetHandle handle, uint32_t index) noexcept;
using GetToggleStateFn           = Result(__cdecl*)(const PluginContext* context, WidgetHandle handle, bool* toggled) noexcept;
using SetToggleStateFn           = Result(__cdecl*)(const PluginContext* context, WidgetHandle handle, bool toggled) noexcept;
using GetSliderValueFn           = Result(__cdecl*)(const PluginContext* context, WidgetHandle handle, int32_t* value) noexcept;
using SetSliderValueFn           = Result(__cdecl*)(const PluginContext* context, WidgetHandle handle, int32_t value) noexcept;
using SetWidgetEventMessageFn    = Result(__cdecl*)(const PluginContext* context, WidgetHandle handle, EventKind event, const char* target, const char* command) noexcept;
using GetWidgetScreenRectFn      = Result(__cdecl*)(const PluginContext* context, WidgetHandle handle, ScreenRect* rect) noexcept;

// Every widget call must run on the UI thread, normally through
// ThreadService::runOnUiThread. findWidget searches below its parent handle.
// dispatchUiAction broadcasts the same target/command/text shape used by native
// panel messages. getInputText accepts InputTextBoxWidget and derived widgets.
// It copies UTF-8 text including the trailing null byte. Pass a null or small
// buffer first to receive BufferTooSmall and the required byte count.
// setText changes a TextBoxWidget label. It does not edit an input field.
// Pass a null-terminated UTF-8 string of at most 4,095 bytes.
// setTooltipText changes the hover text of a FocusableWidget, such as a button.
// Pass a null-terminated UTF-8 string of at most 16,383 bytes. Empty text clears it.
// For buttons, this sets both enabled and disabled hover text.
// setImageSprite changes an ImageWidget sprite and fits it inside box without
// changing its proportions. box coordinates are local to the image's parent.
// Pass an empty filename to clear the image. The path has no .sprite suffix.
// getDropdownSelection reads the zero-based state index of a DropdownListWidget.
// For states ["All", "Unique"], selecting "Unique" returns 1.
// setDropdownUpdateMessage connects a DropdownListWidget to a UI message listener.
// Call it after opening the panel. Repeat the call after each open.
// Both names must be nonempty, null-terminated strings of at most 127 bytes.
// The setter accepts custom names without registering them with the layout parser.
// Omit onUpdateMessage from the layout.
// The dropdown sends binary selection data, so UiMessageEvent::text is null.
// Read the selection with getDropdownSelection.
// The setter does not change the selection or send a message.
// getText reads a TextBoxWidget label. It does not read an input box.
// getWidgetState reports current visibility, active state, and keyboard focus.
// setInputText changes an InputTextBoxWidget and sends its update message.
// It accepts at most 4,095 UTF-8 bytes and checks the widget's length limit.
// The byte limit is conservative for text with multibyte UTF-8 characters.
// clearInputText also clears autocomplete. focusWidget requires an open, active
// panel and a visible, active focusable widget in that panel.
// setWidgetRect changes local bounds through D2R's native layout setters.
// getWidgetScreenRect reads scaled bounds in native UI screen coordinates.
// Compare two widgets with this function. Do not mix these bounds with local
// Rect values or overlay canvas coordinates. Hidden widgets can also be measured.
// For example, show a wider left panel only when its right edge stays left of
// the inventory's left edge. Leave space between the two frames.
// Dropdown indices start at zero. Setters for dropdown, toggle, and slider do
// not send a UI message. A slider value must be within its min/max range.
// setWidgetEventMessage routes a button click, input change, input Enter, or
// dropdown change to a target and command. Call it after each panel open.
// Use nonempty names of at most 127 bytes. For these events, omit the matching
// message field from the layout and read the resulting UiMessageEvent.

static_assert(sizeof(Result) == sizeof(uint32_t));
static_assert(std::is_standard_layout_v<Rect> && std::is_trivially_copyable_v<Rect>);
static_assert(std::is_standard_layout_v<State> && std::is_trivially_copyable_v<State>);
static_assert(std::is_standard_layout_v<UiAction> && std::is_trivially_copyable_v<UiAction>);
static_assert(sizeof(Rect) == 16);
static_assert(std::is_standard_layout_v<ScreenRect> && std::is_trivially_copyable_v<ScreenRect>);
static_assert(sizeof(ScreenRect) == 16);
static_assert(sizeof(State) == 3);
static_assert(sizeof(EventKind) == sizeof(uint32_t));
static_assert(sizeof(UiAction) == 32);

}

struct WidgetService {
	static constexpr ServiceId Id         = ServiceId::Widget;
	static constexpr uint32_t  AbiVersion = 1;

	uint32_t                            serviceSize;
	uint32_t                            serviceVersion;
	Widgets::FindPanelFn                findPanel;
	Widgets::FindWidgetFn               findWidget;
	Widgets::GetWidgetRectFn            getWidgetRect;
	Widgets::SetWidgetVisibleFn         setWidgetVisible;
	Widgets::SetWidgetEnabledFn         setWidgetEnabled;
	Widgets::DispatchUiActionFn         dispatchUiAction;
	Widgets::GetInputTextFn             getInputText;
	Widgets::SetTextFn                  setText;
	Widgets::SetTooltipTextFn           setTooltipText;
	Widgets::SetImageSpriteFn           setImageSprite;
	Widgets::GetDropdownSelectionFn     getDropdownSelection;
	Widgets::SetDropdownUpdateMessageFn setDropdownUpdateMessage;
	Widgets::GetWidgetStateFn           getWidgetState;
	Widgets::GetTextFn                  getText;
	Widgets::SetInputTextFn             setInputText;
	Widgets::ClearInputTextFn           clearInputText;
	Widgets::FocusWidgetFn              focusWidget;
	Widgets::SetWidgetRectFn            setWidgetRect;
	Widgets::SetDropdownSelectionFn     setDropdownSelection;
	Widgets::GetToggleStateFn           getToggleState;
	Widgets::SetToggleStateFn           setToggleState;
	Widgets::GetSliderValueFn           getSliderValue;
	Widgets::SetSliderValueFn           setSliderValue;
	Widgets::SetWidgetEventMessageFn    setWidgetEventMessage;
	Widgets::GetWidgetScreenRectFn      getWidgetScreenRect;
};

inline constexpr uint32_t WidgetServiceSize                             = static_cast<uint32_t>(sizeof(WidgetService));
inline constexpr uint32_t WidgetServiceRequiredSize                     = static_cast<uint32_t>(offsetof(WidgetService, getInputText) + sizeof(Widgets::GetInputTextFn));
inline constexpr uint32_t WidgetServiceSetTextFieldEnd                  = static_cast<uint32_t>(offsetof(WidgetService, setText) + sizeof(Widgets::SetTextFn));
inline constexpr uint32_t WidgetServiceSetTooltipTextFieldEnd           = static_cast<uint32_t>(offsetof(WidgetService, setTooltipText) + sizeof(Widgets::SetTooltipTextFn));
inline constexpr uint32_t WidgetServiceSetImageSpriteFieldEnd           = static_cast<uint32_t>(offsetof(WidgetService, setImageSprite) + sizeof(Widgets::SetImageSpriteFn));
inline constexpr uint32_t WidgetServiceDropdownSelectionFieldEnd        = static_cast<uint32_t>(offsetof(WidgetService, getDropdownSelection) + sizeof(Widgets::GetDropdownSelectionFn));
inline constexpr uint32_t WidgetServiceSetDropdownUpdateMessageFieldEnd = static_cast<uint32_t>(offsetof(WidgetService, setDropdownUpdateMessage) + sizeof(Widgets::SetDropdownUpdateMessageFn));
inline constexpr uint32_t WidgetServiceGetWidgetStateFieldEnd           = static_cast<uint32_t>(offsetof(WidgetService, getWidgetState) + sizeof(Widgets::GetWidgetStateFn));
inline constexpr uint32_t WidgetServiceGetTextFieldEnd                  = static_cast<uint32_t>(offsetof(WidgetService, getText) + sizeof(Widgets::GetTextFn));
inline constexpr uint32_t WidgetServiceSetInputTextFieldEnd             = static_cast<uint32_t>(offsetof(WidgetService, setInputText) + sizeof(Widgets::SetInputTextFn));
inline constexpr uint32_t WidgetServiceClearInputTextFieldEnd           = static_cast<uint32_t>(offsetof(WidgetService, clearInputText) + sizeof(Widgets::ClearInputTextFn));
inline constexpr uint32_t WidgetServiceFocusWidgetFieldEnd              = static_cast<uint32_t>(offsetof(WidgetService, focusWidget) + sizeof(Widgets::FocusWidgetFn));
inline constexpr uint32_t WidgetServiceSetWidgetRectFieldEnd            = static_cast<uint32_t>(offsetof(WidgetService, setWidgetRect) + sizeof(Widgets::SetWidgetRectFn));
inline constexpr uint32_t WidgetServiceSetDropdownSelectionFieldEnd     = static_cast<uint32_t>(offsetof(WidgetService, setDropdownSelection) + sizeof(Widgets::SetDropdownSelectionFn));
inline constexpr uint32_t WidgetServiceGetToggleStateFieldEnd           = static_cast<uint32_t>(offsetof(WidgetService, getToggleState) + sizeof(Widgets::GetToggleStateFn));
inline constexpr uint32_t WidgetServiceSetToggleStateFieldEnd           = static_cast<uint32_t>(offsetof(WidgetService, setToggleState) + sizeof(Widgets::SetToggleStateFn));
inline constexpr uint32_t WidgetServiceGetSliderValueFieldEnd           = static_cast<uint32_t>(offsetof(WidgetService, getSliderValue) + sizeof(Widgets::GetSliderValueFn));
inline constexpr uint32_t WidgetServiceSetSliderValueFieldEnd           = static_cast<uint32_t>(offsetof(WidgetService, setSliderValue) + sizeof(Widgets::SetSliderValueFn));
inline constexpr uint32_t WidgetServiceSetWidgetEventMessageFieldEnd    = static_cast<uint32_t>(offsetof(WidgetService, setWidgetEventMessage) + sizeof(Widgets::SetWidgetEventMessageFn));
inline constexpr uint32_t WidgetServiceGetWidgetScreenRectFieldEnd      = static_cast<uint32_t>(offsetof(WidgetService, getWidgetScreenRect) + sizeof(Widgets::GetWidgetScreenRectFn));

inline auto HasWidgetServiceField(const WidgetService* service, uint32_t fieldEndOffset) noexcept -> bool {
	return service != nullptr && service->serviceVersion == WidgetService::AbiVersion && service->serviceSize >= fieldEndOffset;
}

static_assert(std::is_standard_layout_v<WidgetService> && std::is_trivially_copyable_v<WidgetService>);
static_assert(offsetof(WidgetService, getInputText) == 56);
static_assert(offsetof(WidgetService, setText) == 64);
static_assert(offsetof(WidgetService, setTooltipText) == 72);
static_assert(offsetof(WidgetService, setImageSprite) == 80);
static_assert(offsetof(WidgetService, getDropdownSelection) == 88);
static_assert(offsetof(WidgetService, setDropdownUpdateMessage) == 96);
static_assert(offsetof(WidgetService, getWidgetScreenRect) == 200);
static_assert(sizeof(WidgetService) == 208);

}
