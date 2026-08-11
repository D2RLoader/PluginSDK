#pragma once

#include <D2RLPlugin/services.h>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace D2RL {

struct PluginContext;

namespace Input {

using ActionHandle = uint64_t;

inline constexpr ActionHandle InvalidHandle = 0;

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

// These are stable virtual-key values. D2R's private modifier encoding is not
// part of the plugin ABI.
enum class Key : uint32_t {
	None        = 0x00,
	Backspace   = 0x08,
	Tab         = 0x09,
	Enter       = 0x0D,
	Pause       = 0x13,
	CapsLock    = 0x14,
	Space       = 0x20,
	PageUp      = 0x21,
	PageDown    = 0x22,
	End         = 0x23,
	Home        = 0x24,
	Print       = 0x2A,
	PrintScreen = 0x2C,
	Insert      = 0x2D,
	Delete      = 0x2E,
	Help        = 0x2F,

	Number0 = 0x30,
	Number1 = 0x31,
	Number2 = 0x32,
	Number3 = 0x33,
	Number4 = 0x34,
	Number5 = 0x35,
	Number6 = 0x36,
	Number7 = 0x37,
	Number8 = 0x38,
	Number9 = 0x39,

	A = 0x41,
	B = 0x42,
	C = 0x43,
	D = 0x44,
	E = 0x45,
	F = 0x46,
	G = 0x47,
	H = 0x48,
	I = 0x49,
	J = 0x4A,
	K = 0x4B,
	L = 0x4C,
	M = 0x4D,
	N = 0x4E,
	O = 0x4F,
	P = 0x50,
	Q = 0x51,
	R = 0x52,
	S = 0x53,
	T = 0x54,
	U = 0x55,
	V = 0x56,
	W = 0x57,
	X = 0x58,
	Y = 0x59,
	Z = 0x5A,

	Numpad0  = 0x60,
	Numpad1  = 0x61,
	Numpad2  = 0x62,
	Numpad3  = 0x63,
	Numpad4  = 0x64,
	Numpad5  = 0x65,
	Numpad6  = 0x66,
	Numpad7  = 0x67,
	Numpad8  = 0x68,
	Numpad9  = 0x69,
	Multiply = 0x6A,
	Add      = 0x6B,
	Subtract = 0x6D,
	Decimal  = 0x6E,
	Divide   = 0x6F,

	F1  = 0x70,
	F2  = 0x71,
	F3  = 0x72,
	F4  = 0x73,
	F5  = 0x74,
	F6  = 0x75,
	F7  = 0x76,
	F8  = 0x77,
	F9  = 0x78,
	F10 = 0x79,
	F11 = 0x7A,
	F12 = 0x7B,
	F13 = 0x7C,
	F14 = 0x7D,
	F15 = 0x7E,
	F16 = 0x7F,
	F17 = 0x80,
	F18 = 0x81,
	F19 = 0x82,
	F20 = 0x83,
	F21 = 0x84,
	F22 = 0x85,
	F23 = 0x86,
	F24 = 0x87,

	ScrollLock   = 0x91,
	Semicolon    = 0xBA,
	Equals       = 0xBB,
	Comma        = 0xBC,
	Minus        = 0xBD,
	Period       = 0xBE,
	Slash        = 0xBF,
	Backquote    = 0xC0,
	LeftBracket  = 0xDB,
	Backslash    = 0xDC,
	RightBracket = 0xDD,
	Quote        = 0xDE,
};

// The native keyboard menu supports no modifier or exactly one modifier.
enum class Modifier : uint32_t {
	None    = 0,
	Shift   = 1,
	Control = 2,
	Alt     = 3,
};

enum class BindingSlot : uint32_t {
	Primary   = 0,
	Secondary = 1,
};

enum class ActionEventKind : uint32_t {
	Pressed  = 1,
	Released = 2,
};

enum class ActionResult : uint32_t {
	Ignored = 0,
	Handled = 1,
};

struct Binding {
	Key      key;
	Modifier modifier;
};

struct ActionEvent {
	uint32_t        structSize;
	uint32_t        flags;
	ActionHandle    action;
	ActionEventKind kind;
	uint32_t        reserved;
	Binding         binding;
};

using ActionCallback = ActionResult(__cdecl*)(const PluginContext* context, const ActionEvent* event, void* userData) noexcept;

// Register actions while the plugin is loading. D2RLoader copies all text and
// callback fields before returning. Keep logicalId stable, lowercase ASCII, and
// unique inside the plugin. An empty category uses "Extensions".
// V1 actions run in the normal in-game keyboard context and do not fire while
// the player is editing text or binding controls.
struct ActionRegistration {
	uint32_t       structSize;
	uint32_t       flags;
	const char*    logicalId;
	const char*    displayName;
	const char*    category;
	Binding        defaultPrimary;
	Binding        defaultSecondary;
	ActionCallback callback;
	void*          userData;
};

inline constexpr uint32_t BindingSize                    = static_cast<uint32_t>(sizeof(Binding));
inline constexpr uint32_t ActionEventSize                = static_cast<uint32_t>(sizeof(ActionEvent));
inline constexpr uint32_t ActionEventRequiredSize        = static_cast<uint32_t>(offsetof(ActionEvent, binding) + sizeof(Binding));
inline constexpr uint32_t ActionRegistrationSize         = static_cast<uint32_t>(sizeof(ActionRegistration));
inline constexpr uint32_t ActionRegistrationRequiredSize = static_cast<uint32_t>(offsetof(ActionRegistration, userData) + sizeof(void*));

inline auto HasActionEventField(const ActionEvent* event, uint32_t fieldEndOffset) noexcept -> bool {
	return event != nullptr && event->structSize >= fieldEndOffset;
}

inline auto HasActionRegistrationField(const ActionRegistration* registration, uint32_t fieldEndOffset) noexcept -> bool {
	return registration != nullptr && registration->structSize >= fieldEndOffset;
}

using RegisterActionFn = Result(__cdecl*)(const PluginContext* context, const ActionRegistration* registration, ActionHandle* action) noexcept;

using UnregisterActionFn = Result(__cdecl*)(const PluginContext* context, ActionHandle action) noexcept;

using GetBindingFn = Result(__cdecl*)(const PluginContext* context, ActionHandle action, BindingSlot slot, Binding* binding) noexcept;

} // namespace Input

struct InputServiceV1 {
	uint32_t                  serviceSize;
	uint32_t                  serviceVersion;
	Input::RegisterActionFn   registerAction;
	Input::UnregisterActionFn unregisterAction;
	Input::GetBindingFn       getBinding;
};

inline constexpr uint32_t InputServiceV1Version      = 1;
inline constexpr uint32_t InputServiceV1Size         = static_cast<uint32_t>(sizeof(InputServiceV1));
inline constexpr uint32_t InputServiceV1RequiredSize = static_cast<uint32_t>(offsetof(InputServiceV1, getBinding) + sizeof(Input::GetBindingFn));

inline auto HasInputServiceV1Field(const InputServiceV1* service, uint32_t fieldEndOffset) noexcept -> bool {
	return service != nullptr && service->serviceVersion == InputServiceV1Version && service->serviceSize >= fieldEndOffset;
}

static_assert(sizeof(Input::Key) == sizeof(uint32_t));
static_assert(sizeof(Input::Modifier) == sizeof(uint32_t));
static_assert(sizeof(Input::BindingSlot) == sizeof(uint32_t));
static_assert(sizeof(Input::ActionEventKind) == sizeof(uint32_t));
static_assert(sizeof(Input::ActionResult) == sizeof(uint32_t));
static_assert(sizeof(Input::Result) == sizeof(uint32_t));
static_assert(sizeof(Input::ActionHandle) == sizeof(uint64_t));
static_assert(std::is_standard_layout_v<Input::Binding>);
static_assert(std::is_trivially_copyable_v<Input::Binding>);
static_assert(offsetof(Input::Binding, key) == 0);
static_assert(offsetof(Input::Binding, modifier) == 4);
static_assert(sizeof(Input::Binding) == 8);
static_assert(std::is_standard_layout_v<Input::ActionEvent>);
static_assert(std::is_trivially_copyable_v<Input::ActionEvent>);
static_assert(offsetof(Input::ActionEvent, action) == 8);
static_assert(offsetof(Input::ActionEvent, kind) == 16);
static_assert(offsetof(Input::ActionEvent, binding) == 24);
static_assert(sizeof(Input::ActionEvent) == 32);
static_assert(std::is_standard_layout_v<Input::ActionRegistration>);
static_assert(std::is_trivially_copyable_v<Input::ActionRegistration>);
static_assert(offsetof(Input::ActionRegistration, logicalId) == 8);
static_assert(offsetof(Input::ActionRegistration, displayName) == 16);
static_assert(offsetof(Input::ActionRegistration, category) == 24);
static_assert(offsetof(Input::ActionRegistration, defaultPrimary) == 32);
static_assert(offsetof(Input::ActionRegistration, defaultSecondary) == 40);
static_assert(offsetof(Input::ActionRegistration, callback) == 48);
static_assert(offsetof(Input::ActionRegistration, userData) == 56);
static_assert(sizeof(Input::ActionRegistration) == 64);
static_assert(std::is_standard_layout_v<InputServiceV1>);
static_assert(std::is_trivially_copyable_v<InputServiceV1>);
static_assert(offsetof(InputServiceV1, serviceSize) == 0);
static_assert(offsetof(InputServiceV1, serviceVersion) == 4);
static_assert(offsetof(InputServiceV1, registerAction) == 8);
static_assert(offsetof(InputServiceV1, unregisterAction) == 16);
static_assert(offsetof(InputServiceV1, getBinding) == 24);
static_assert(InputServiceV1RequiredSize == 32);
static_assert(sizeof(InputServiceV1) == 32);

} // namespace D2RL
