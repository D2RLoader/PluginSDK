#pragma once

#include <D2RLPlugin/services.h>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>

namespace D2RL {

struct PluginContext;
struct PluginCommunicationService;

namespace PluginCommunication {

using ServiceHandle      = uint64_t;
using SubscriptionHandle = uint64_t;

inline constexpr ServiceHandle      InvalidServiceHandle      = 0;
inline constexpr SubscriptionHandle InvalidSubscriptionHandle = 0;
inline constexpr uint32_t           MaxNameBytes              = 127;
inline constexpr uint32_t           MaxEventDataBytes         = 1U * 1'024U * 1'024U;

enum class Result : uint32_t {
	Success            = 0,
	InvalidArgument    = 1,
	Unsupported        = 2,
	Unavailable        = 3,
	AlreadyPublished   = 4,
	NotFound           = 5,
	UnsupportedVersion = 6,
	TableTooSmall      = 7,
	DependencyCycle    = 8,
	OwnerInactive      = 9,
	OwnerMismatch      = 10,
	CallbackFault      = 11,
	LimitExceeded      = 12,
};

// The loader uses the calling plugin as the provider. name is local to that
// provider, so two plugins may use the same short name. The table must remain
// valid until the provider unloads.
struct PublishServiceRequest {
	uint32_t    structSize;
	uint32_t    flags;
	const char* name;
	uint32_t    serviceVersion;
	uint32_t    tableSize;
	const void* table;
};

// A higher service version must still support older callers. Add new table
// fields at the end. Use a new service name for a breaking change.
struct AcquireServiceRequest {
	uint32_t    structSize;
	uint32_t    flags;
	const char* providerPluginId;
	const char* name;
	uint32_t    minimumVersion;
	uint32_t    minimumTableSize;
};

// Keep handle until the last call through table has finished. Releasing it lets
// the provider unload after this consumer.
struct AcquiredService {
	uint32_t      structSize;
	uint32_t      flags;
	ServiceHandle handle;
	const void*   table;
	uint32_t      serviceVersion;
	uint32_t      tableSize;
};

// publishEvent copies data before callbacks begin. Event callbacks receive a
// view of that copy. The pointers in EventView are valid only for the callback.
struct PublishEventRequest {
	uint32_t    structSize;
	uint32_t    flags;
	const char* name;
	uint32_t    eventVersion;
	uint32_t    dataSize;
	const void* data;
};

struct EventView {
	uint32_t    structSize;
	uint32_t    flags;
	const char* publisherPluginId;
	const char* name;
	uint32_t    eventVersion;
	uint32_t    dataSize;
	const void* data;
};

using EventCallback = void(__cdecl*)(const PluginContext* context, const EventView* event, void* userData) noexcept;

// A subscription may be registered before its publisher loads. A matching
// event runs on the thread that calls publishEvent.
struct EventSubscription {
	uint32_t      structSize;
	uint32_t      flags;
	const char*   publisherPluginId;
	const char*   name;
	uint32_t      minimumVersion;
	uint32_t      minimumDataSize;
	EventCallback callback;
	void*         userData;
};

inline constexpr uint32_t PublishServiceRequestSize         = static_cast<uint32_t>(sizeof(PublishServiceRequest));
inline constexpr uint32_t PublishServiceRequestRequiredSize = PublishServiceRequestSize;
inline constexpr uint32_t AcquireServiceRequestSize         = static_cast<uint32_t>(sizeof(AcquireServiceRequest));
inline constexpr uint32_t AcquireServiceRequestRequiredSize = AcquireServiceRequestSize;
inline constexpr uint32_t AcquiredServiceSize               = static_cast<uint32_t>(sizeof(AcquiredService));
inline constexpr uint32_t AcquiredServiceRequiredSize       = AcquiredServiceSize;
inline constexpr uint32_t PublishEventRequestSize           = static_cast<uint32_t>(sizeof(PublishEventRequest));
inline constexpr uint32_t PublishEventRequestRequiredSize   = PublishEventRequestSize;
inline constexpr uint32_t EventViewSize                     = static_cast<uint32_t>(sizeof(EventView));
inline constexpr uint32_t EventViewRequiredSize             = EventViewSize;
inline constexpr uint32_t EventSubscriptionSize             = static_cast<uint32_t>(sizeof(EventSubscription));
inline constexpr uint32_t EventSubscriptionRequiredSize     = EventSubscriptionSize;

using PublishServiceFn   = Result(__cdecl*)(const PluginContext* context, const PublishServiceRequest* request) noexcept;
using AcquireServiceFn   = Result(__cdecl*)(const PluginContext* context, const AcquireServiceRequest* request, AcquiredService* acquired) noexcept;
using ReleaseServiceFn   = Result(__cdecl*)(const PluginContext* context, ServiceHandle handle) noexcept;
using SubscribeEventFn   = Result(__cdecl*)(const PluginContext* context, const EventSubscription* subscription, SubscriptionHandle* handle) noexcept;
using UnsubscribeEventFn = Result(__cdecl*)(const PluginContext* context, SubscriptionHandle handle) noexcept;
using PublishEventFn     = Result(__cdecl*)(const PluginContext* context, const PublishEventRequest* request) noexcept;

static_assert(sizeof(Result) == sizeof(uint32_t));
static_assert(std::is_standard_layout_v<PublishServiceRequest> && std::is_trivially_copyable_v<PublishServiceRequest>);
static_assert(std::is_standard_layout_v<AcquireServiceRequest> && std::is_trivially_copyable_v<AcquireServiceRequest>);
static_assert(std::is_standard_layout_v<AcquiredService> && std::is_trivially_copyable_v<AcquiredService>);
static_assert(std::is_standard_layout_v<PublishEventRequest> && std::is_trivially_copyable_v<PublishEventRequest>);
static_assert(std::is_standard_layout_v<EventView> && std::is_trivially_copyable_v<EventView>);
static_assert(std::is_standard_layout_v<EventSubscription> && std::is_trivially_copyable_v<EventSubscription>);
static_assert(sizeof(PublishServiceRequest) == 32);
static_assert(sizeof(AcquireServiceRequest) == 32);
static_assert(sizeof(AcquiredService) == 32);
static_assert(sizeof(PublishEventRequest) == 32);
static_assert(sizeof(EventView) == 40);
static_assert(sizeof(EventSubscription) == 48);

template <typename Table>
class ServiceLease {
  public:
	ServiceLease() noexcept                               = default;
	ServiceLease(const ServiceLease&)                     = delete;
	auto operator =(const ServiceLease&) -> ServiceLease& = delete;
	ServiceLease(ServiceLease&& other) noexcept;
	auto operator =(ServiceLease&& other) noexcept -> ServiceLease&;
	~ServiceLease();

	[[nodiscard]]
	auto Get() const noexcept -> const Table* {
		return table;
	}

	[[nodiscard]]
	auto operator ->() const noexcept -> const Table* {
		return table;
	}

	[[nodiscard]]
	explicit operator bool() const noexcept {
		return table != nullptr && handle != InvalidServiceHandle;
	}

	[[nodiscard]]
	auto Version() const noexcept -> uint32_t {
		return serviceVersion;
	}

	[[nodiscard]]
	auto Size() const noexcept -> uint32_t {
		return tableSize;
	}

	auto Reset() noexcept -> Result;

  private:
	void Adopt(const PluginContext* owner, const PluginCommunicationService* ownerService, const AcquiredService& acquired) noexcept;

	const PluginContext*              context        = nullptr;
	const PluginCommunicationService* service        = nullptr;
	ServiceHandle                     handle         = InvalidServiceHandle;
	const Table*                      table          = nullptr;
	uint32_t                          serviceVersion = 0;
	uint32_t                          tableSize      = 0;

	template <typename OtherTable>
	friend auto Acquire(const PluginContext* context,
		const PluginCommunicationService*    service,
		const char*                          providerPluginId,
		const char*                          name,
		uint32_t                             minimumVersion,
		uint32_t                             minimumTableSize,
		ServiceLease<OtherTable>*            acquired) noexcept -> Result;
};

template <typename Table>
auto Acquire(const PluginContext*     context,
	const PluginCommunicationService* service,
	const char*                       providerPluginId,
	const char*                       name,
	uint32_t                          minimumVersion,
	uint32_t                          minimumTableSize,
	ServiceLease<Table>*              acquired) noexcept -> Result;

}

struct PluginCommunicationService {
	static constexpr ServiceId Id         = ServiceId::PluginCommunication;
	static constexpr uint32_t  AbiVersion = 1;

	uint32_t                                serviceSize;
	uint32_t                                serviceVersion;
	PluginCommunication::PublishServiceFn   publishService;
	PluginCommunication::AcquireServiceFn   acquireService;
	PluginCommunication::ReleaseServiceFn   releaseService;
	PluginCommunication::SubscribeEventFn   subscribeEvent;
	PluginCommunication::UnsubscribeEventFn unsubscribeEvent;
	PluginCommunication::PublishEventFn     publishEvent;
};

inline constexpr uint32_t PluginCommunicationServiceSize         = static_cast<uint32_t>(sizeof(PluginCommunicationService));
inline constexpr uint32_t PluginCommunicationServiceRequiredSize = PluginCommunicationServiceSize;

inline auto HasPluginCommunicationServiceField(const PluginCommunicationService* service, uint32_t fieldEndOffset) noexcept -> bool {
	return service != nullptr && service->serviceVersion == PluginCommunicationService::AbiVersion && service->serviceSize >= fieldEndOffset;
}

static_assert(std::is_standard_layout_v<PluginCommunicationService> && std::is_trivially_copyable_v<PluginCommunicationService>);
static_assert(sizeof(PluginCommunicationService) == 56);

namespace PluginCommunication {

template <typename Table>
ServiceLease<Table>::ServiceLease(ServiceLease&& other) noexcept :
	context(std::exchange(other.context, nullptr)),
	service(std::exchange(other.service, nullptr)),
	handle(std::exchange(other.handle, InvalidServiceHandle)),
	table(std::exchange(other.table, nullptr)),
	serviceVersion(std::exchange(other.serviceVersion, 0)),
	tableSize(std::exchange(other.tableSize, 0)) {}

template <typename Table>
auto ServiceLease<Table>::operator =(ServiceLease&& other) noexcept -> ServiceLease& {
	if (this != &other) {
		Reset();
		context        = std::exchange(other.context, nullptr);
		service        = std::exchange(other.service, nullptr);
		handle         = std::exchange(other.handle, InvalidServiceHandle);
		table          = std::exchange(other.table, nullptr);
		serviceVersion = std::exchange(other.serviceVersion, 0);
		tableSize      = std::exchange(other.tableSize, 0);
	}
	return *this;
}

template <typename Table>
ServiceLease<Table>::~ServiceLease() {
	Reset();
}

template <typename Table>
auto ServiceLease<Table>::Reset() noexcept -> Result {
	Result result = Result::Success;
	if (service != nullptr && service->releaseService != nullptr && context != nullptr && handle != InvalidServiceHandle) {
		result = service->releaseService(context, handle);
	}
	context        = nullptr;
	service        = nullptr;
	handle         = InvalidServiceHandle;
	table          = nullptr;
	serviceVersion = 0;
	tableSize      = 0;
	return result;
}

template <typename Table>
void ServiceLease<Table>::Adopt(const PluginContext* owner, const PluginCommunicationService* ownerService, const AcquiredService& acquired) noexcept {
	context        = owner;
	service        = ownerService;
	handle         = acquired.handle;
	table          = static_cast<const Table*>(acquired.table);
	serviceVersion = acquired.serviceVersion;
	tableSize      = acquired.tableSize;
}

template <typename Table>
auto Acquire(const PluginContext*     context,
	const PluginCommunicationService* service,
	const char*                       providerPluginId,
	const char*                       name,
	uint32_t                          minimumVersion,
	uint32_t                          minimumTableSize,
	ServiceLease<Table>*              acquired) noexcept -> Result {
	static_assert(!std::is_void_v<Table>);
	if (acquired == nullptr) {
		return Result::InvalidArgument;
	}

	acquired->Reset();
	if (context == nullptr || service == nullptr || service->acquireService == nullptr || service->releaseService == nullptr) {
		return Result::Unavailable;
	}

	const AcquireServiceRequest request {
		.structSize       = AcquireServiceRequestSize,
		.flags            = 0,
		.providerPluginId = providerPluginId,
		.name             = name,
		.minimumVersion   = minimumVersion,
		.minimumTableSize = minimumTableSize,
	};
	AcquiredService value { .structSize = AcquiredServiceSize };
	const Result    result = service->acquireService(context, &request, &value);
	if (result == Result::Success) {
		acquired->Adopt(context, service, value);
	}
	return result;
}

}

}
