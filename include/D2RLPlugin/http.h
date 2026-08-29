#pragma once

#include <D2RLPlugin/services.h>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace D2RL {

struct PluginContext;

namespace Http {

using RequestHandle = uint64_t;

inline constexpr RequestHandle InvalidRequestHandle   = 0;
inline constexpr uint32_t      DefaultTimeoutMs       = 30'000;
inline constexpr uint32_t      DefaultMaxResponseSize = 8U * 1'024U * 1'024U;
inline constexpr uint32_t      MaxUrlSize             = 8U * 1'024U;
inline constexpr uint32_t      MaxHeaderCount         = 128;
inline constexpr uint32_t      MaxHeaderNameSize      = 256;
inline constexpr uint32_t      MaxHeaderValueSize     = 16U * 1'024U;
inline constexpr uint32_t      MaxRequestHeaderSize   = 1U * 1'024U * 1'024U;
inline constexpr uint32_t      MaxRequestBodySize     = 64U * 1'024U * 1'024U;
inline constexpr uint32_t      MaxResponseSize        = 64U * 1'024U * 1'024U;

enum class Result : uint32_t {
	Success          = 0,
	InvalidArgument  = 1,
	Unsupported      = 2,
	Unavailable      = 3,
	NotFound         = 4,
	Busy             = 5,
	OwnerInactive    = 6,
	OwnerMismatch    = 7,
	CallbackFault    = 8,
	Timeout          = 9,
	NetworkError     = 10,
	TlsError         = 11,
	ResponseTooLarge = 12,
};

enum class Method : uint32_t {
	Get    = 1,
	Post   = 2,
	Put    = 3,
	Patch  = 4,
	Delete = 5,
	Head   = 6,
};

// Names and values are UTF-8 and nul-terminated. Request values are copied by
// send before it returns. Response values are borrowed for the callback only.
struct Header {
		const char* name;
		const char* value;
};

// Only https:// URLs are accepted. Zero timeoutMs/maxResponseSize select the
// defaults above. The body may contain arbitrary bytes. Redirects may stay on
// HTTPS but are never allowed to downgrade to HTTP.
struct Request {
		uint32_t      structSize;
		uint32_t      flags;
		Method        method;
		uint32_t      timeoutMs;
		const char*   url;
		uint32_t      headerCount;
		uint32_t      bodySize;
		const Header* headers;
		const void*   body;
		uint32_t      maxResponseSize;
		uint32_t      reserved;
};

// A transport success may still contain an HTTP error status such as 404. The
// header array, strings, and body are borrowed and valid only during callback.
// Callbacks run on a worker thread, never the game or UI thread.
struct Response {
		uint32_t      structSize;
		uint32_t      flags;
		RequestHandle request;
		Result        result;
		uint32_t      statusCode;
		uint32_t      headerCount;
		uint32_t      bodySize;
		const Header* headers;
		const void*   body;
};

using ResponseCallback = void(__cdecl*)(const PluginContext* context, const Response* response, void* userData) noexcept;
using SendFn           = Result(__cdecl*)(const PluginContext* context, const Request* request, ResponseCallback callback, void* userData, RequestHandle* handle) noexcept;
// A successful cancel suppresses the callback. It returns Busy after the
// callback starts. Plugin unload cancels every request owned by that plugin.
using CancelFn         = Result(__cdecl*)(const PluginContext* context, RequestHandle handle) noexcept;

inline constexpr uint32_t HeaderSize           = static_cast<uint32_t>(sizeof(Header));
inline constexpr uint32_t RequestSize          = static_cast<uint32_t>(sizeof(Request));
inline constexpr uint32_t RequestRequiredSize  = RequestSize;
inline constexpr uint32_t ResponseSize         = static_cast<uint32_t>(sizeof(Response));
inline constexpr uint32_t ResponseRequiredSize = ResponseSize;

inline auto HasRequestField(const Request* request, uint32_t fieldEndOffset) noexcept -> bool {
	return request != nullptr && request->structSize >= fieldEndOffset;
}

inline auto HasResponseField(const Response* response, uint32_t fieldEndOffset) noexcept -> bool {
	return response != nullptr && response->structSize >= fieldEndOffset;
}

static_assert(sizeof(RequestHandle) == sizeof(uint64_t));
static_assert(sizeof(Result) == sizeof(uint32_t));
static_assert(sizeof(Method) == sizeof(uint32_t));
static_assert(std::is_standard_layout_v<Header> && std::is_trivially_copyable_v<Header>);
static_assert(std::is_standard_layout_v<Request> && std::is_trivially_copyable_v<Request>);
static_assert(std::is_standard_layout_v<Response> && std::is_trivially_copyable_v<Response>);
static_assert(sizeof(Header) == 16);
static_assert(sizeof(Request) == 56);
static_assert(sizeof(Response) == 48);

}

struct HttpServiceV1 {
		uint32_t       serviceSize;
		uint32_t       serviceVersion;
		Http::SendFn   send;
		Http::CancelFn cancel;
};

inline constexpr uint32_t HttpServiceV1Version      = 1;
inline constexpr uint32_t HttpServiceV1Size         = static_cast<uint32_t>(sizeof(HttpServiceV1));
inline constexpr uint32_t HttpServiceV1RequiredSize = HttpServiceV1Size;

inline auto HasHttpServiceV1Field(const HttpServiceV1* service, uint32_t fieldEndOffset) noexcept -> bool {
	return service != nullptr && service->serviceVersion == HttpServiceV1Version && service->serviceSize >= fieldEndOffset;
}

static_assert(std::is_standard_layout_v<HttpServiceV1> && std::is_trivially_copyable_v<HttpServiceV1>);
static_assert(sizeof(HttpServiceV1) == 24);

}
