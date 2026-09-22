#pragma once

#include <D2RLPlugin/services.h>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace D2RL {

struct PluginContext;

namespace Overlay {

using CallbackHandle = uint64_t;
using CanvasHandle   = uint64_t;

inline constexpr CallbackHandle InvalidCallbackHandle = 0;
inline constexpr CanvasHandle   InvalidCanvasHandle   = 0;
inline constexpr uint32_t       MaxTextBytes          = 4'096;

enum class Result : uint32_t {
	Success         = 0,
	InvalidArgument = 1,
	Unsupported     = 2,
	OwnerInactive   = 3,
	Busy            = 4,
	NotFound        = 5,
	OwnerMismatch   = 6,
	NotReady        = 7,
	WrongThread     = 8,
	StaleCanvas     = 9,
	LimitExceeded   = 10,
	CallbackFault   = 11,
	Unavailable     = 12,
};

enum class Phase : uint32_t {
	AfterGameUi = 0,
};

struct Point {
	float x;
	float y;
};

struct Rect {
	float left;
	float top;
	float right;
	float bottom;
};

// Each color channel uses a value from 0.0 to 1.0.
struct Color {
	float red;
	float green;
	float blue;
	float alpha;
};

// Coordinates use the top-left corner as (0, 0). The canvas is valid only
// while the callback is running and only on the callback thread.
struct Frame {
	uint32_t     structSize;
	uint32_t     flags;
	CanvasHandle canvas;
	uint64_t     frameNumber;
	float        screenWidth;
	float        screenHeight;
	float        deltaTimeSeconds;
	float        defaultTextSize;
};

using FrameCallback = void(__cdecl*)(const PluginContext* context, const Frame* frame, void* userData) noexcept;

// Higher priority callbacks run first. Callbacks with the same priority run in
// registration order. V1 supports only AfterGameUi and flags must be zero.
struct CallbackRegistration {
	uint32_t      structSize;
	uint32_t      flags;
	Phase         phase;
	int32_t       priority;
	FrameCallback callback;
	void*         userData;
};

// getMetrics returns the values from the latest overlay frame. Set structSize
// before the call and set flags to zero.
struct Metrics {
	uint32_t structSize;
	uint32_t flags;
	uint64_t frameNumber;
	float    screenWidth;
	float    screenHeight;
	float    deltaTimeSeconds;
	float    defaultTextSize;
};

// Set flags and reserved fields to zero in V1. Rectangle edges must be in
// order: left <= right and top <= bottom. Thickness must be greater than zero.
// Rounding can be zero but cannot be negative.
struct LineRequest {
	uint32_t     structSize;
	uint32_t     flags;
	CanvasHandle canvas;
	Point        start;
	Point        end;
	Color        color;
	float        thickness;
	uint32_t     reserved;
};

struct RectangleRequest {
	uint32_t     structSize;
	uint32_t     flags;
	CanvasHandle canvas;
	Rect         rect;
	Color        color;
	float        rounding;
	float        thickness;
};

struct FilledRectangleRequest {
	uint32_t     structSize;
	uint32_t     flags;
	CanvasHandle canvas;
	Rect         rect;
	Color        color;
	float        rounding;
	uint32_t     reserved;
};

// Text is UTF-8 and does not need a trailing null byte. textLength cannot
// exceed MaxTextBytes. For example, 4,096 bytes is accepted, but 4,097 is not.
// Set textSize to zero to use Frame::defaultTextSize.
struct TextRequest {
	uint32_t     structSize;
	uint32_t     flags;
	CanvasHandle canvas;
	Point        position;
	Color        color;
	float        textSize;
	uint32_t     reserved;
	const char*  text;
	uint32_t     textLength;
	uint32_t     reserved2;
};

struct MeasureTextRequest {
	uint32_t     structSize;
	uint32_t     flags;
	CanvasHandle canvas;
	float        textSize;
	uint32_t     reserved;
	const char*  text;
	uint32_t     textLength;
	uint32_t     reserved2;
};

// Set structSize before measureText and set flags to zero.
struct TextMetrics {
	uint32_t structSize;
	uint32_t flags;
	float    width;
	float    height;
};

// A clip rectangle limits later commands from the same callback. Each push
// should have a matching pop. The loader restores any clips left by a callback.
struct ClipRequest {
	uint32_t     structSize;
	uint32_t     flags;
	CanvasHandle canvas;
	Rect         rect;
};

inline constexpr uint32_t FrameSize                          = static_cast<uint32_t>(sizeof(Frame));
inline constexpr uint32_t FrameRequiredSize                  = FrameSize;
inline constexpr uint32_t CallbackRegistrationSize           = static_cast<uint32_t>(sizeof(CallbackRegistration));
inline constexpr uint32_t CallbackRegistrationRequiredSize   = CallbackRegistrationSize;
inline constexpr uint32_t MetricsSize                        = static_cast<uint32_t>(sizeof(Metrics));
inline constexpr uint32_t MetricsRequiredSize                = MetricsSize;
inline constexpr uint32_t LineRequestSize                    = static_cast<uint32_t>(sizeof(LineRequest));
inline constexpr uint32_t LineRequestRequiredSize            = LineRequestSize;
inline constexpr uint32_t RectangleRequestSize               = static_cast<uint32_t>(sizeof(RectangleRequest));
inline constexpr uint32_t RectangleRequestRequiredSize       = RectangleRequestSize;
inline constexpr uint32_t FilledRectangleRequestSize         = static_cast<uint32_t>(sizeof(FilledRectangleRequest));
inline constexpr uint32_t FilledRectangleRequestRequiredSize = FilledRectangleRequestSize;
inline constexpr uint32_t TextRequestSize                    = static_cast<uint32_t>(sizeof(TextRequest));
inline constexpr uint32_t TextRequestRequiredSize            = TextRequestSize;
inline constexpr uint32_t MeasureTextRequestSize             = static_cast<uint32_t>(sizeof(MeasureTextRequest));
inline constexpr uint32_t MeasureTextRequestRequiredSize     = MeasureTextRequestSize;
inline constexpr uint32_t TextMetricsSize                    = static_cast<uint32_t>(sizeof(TextMetrics));
inline constexpr uint32_t TextMetricsRequiredSize            = TextMetricsSize;
inline constexpr uint32_t ClipRequestSize                    = static_cast<uint32_t>(sizeof(ClipRequest));
inline constexpr uint32_t ClipRequestRequiredSize            = ClipRequestSize;

using RegisterFrameCallbackFn   = Result(__cdecl*)(const PluginContext* context, const CallbackRegistration* registration, CallbackHandle* handle) noexcept;
using UnregisterFrameCallbackFn = Result(__cdecl*)(const PluginContext* context, CallbackHandle handle) noexcept;
using GetMetricsFn              = Result(__cdecl*)(const PluginContext* context, Metrics* metrics) noexcept;
using DrawLineFn                = Result(__cdecl*)(const PluginContext* context, const LineRequest* request) noexcept;
using DrawRectangleFn           = Result(__cdecl*)(const PluginContext* context, const RectangleRequest* request) noexcept;
using DrawFilledRectangleFn     = Result(__cdecl*)(const PluginContext* context, const FilledRectangleRequest* request) noexcept;
using DrawTextFn                = Result(__cdecl*)(const PluginContext* context, const TextRequest* request) noexcept;
using MeasureTextFn             = Result(__cdecl*)(const PluginContext* context, const MeasureTextRequest* request, TextMetrics* metrics) noexcept;
using PushClipFn                = Result(__cdecl*)(const PluginContext* context, const ClipRequest* request) noexcept;
using PopClipFn                 = Result(__cdecl*)(const PluginContext* context, CanvasHandle canvas) noexcept;

static_assert(sizeof(Result) == sizeof(uint32_t));
static_assert(sizeof(Phase) == sizeof(uint32_t));
static_assert(std::is_standard_layout_v<Frame> && std::is_trivially_copyable_v<Frame>);
static_assert(std::is_standard_layout_v<CallbackRegistration> && std::is_trivially_copyable_v<CallbackRegistration>);
static_assert(std::is_standard_layout_v<Metrics> && std::is_trivially_copyable_v<Metrics>);
static_assert(std::is_standard_layout_v<LineRequest> && std::is_trivially_copyable_v<LineRequest>);
static_assert(std::is_standard_layout_v<RectangleRequest> && std::is_trivially_copyable_v<RectangleRequest>);
static_assert(std::is_standard_layout_v<FilledRectangleRequest> && std::is_trivially_copyable_v<FilledRectangleRequest>);
static_assert(std::is_standard_layout_v<TextRequest> && std::is_trivially_copyable_v<TextRequest>);
static_assert(std::is_standard_layout_v<MeasureTextRequest> && std::is_trivially_copyable_v<MeasureTextRequest>);
static_assert(std::is_standard_layout_v<TextMetrics> && std::is_trivially_copyable_v<TextMetrics>);
static_assert(std::is_standard_layout_v<ClipRequest> && std::is_trivially_copyable_v<ClipRequest>);

}

struct OverlayService {
	static constexpr ServiceId Id         = ServiceId::Overlay;
	static constexpr uint32_t  AbiVersion = 1;

	uint32_t                           serviceSize;
	uint32_t                           serviceVersion;
	Overlay::RegisterFrameCallbackFn   registerFrameCallback;
	Overlay::UnregisterFrameCallbackFn unregisterFrameCallback;
	Overlay::GetMetricsFn              getMetrics;
	Overlay::DrawLineFn                drawLine;
	Overlay::DrawRectangleFn           drawRectangle;
	Overlay::DrawFilledRectangleFn     drawFilledRectangle;
	Overlay::DrawTextFn                drawText;
	Overlay::MeasureTextFn             measureText;
	Overlay::PushClipFn                pushClip;
	Overlay::PopClipFn                 popClip;
};

inline constexpr uint32_t OverlayServiceSize         = static_cast<uint32_t>(sizeof(OverlayService));
inline constexpr uint32_t OverlayServiceRequiredSize = OverlayServiceSize;

inline auto HasOverlayServiceField(const OverlayService* service, uint32_t fieldEndOffset) noexcept -> bool {
	return service != nullptr && service->serviceVersion == OverlayService::AbiVersion && service->serviceSize >= fieldEndOffset;
}

static_assert(std::is_standard_layout_v<OverlayService> && std::is_trivially_copyable_v<OverlayService>);

}
