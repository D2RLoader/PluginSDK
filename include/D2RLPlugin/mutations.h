#pragma once

#include <D2RLPlugin/patching.h>
#include <D2RLPlugin/services.h>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace D2RL {

struct PluginContext;

namespace Mutations {

// A mutation is a byte patch or inline hook. A transaction applies every
// staged mutation together, or restores the original bytes when commit fails.
using TransactionHandle = uint64_t;
using OperationHandle   = uint64_t;

inline constexpr TransactionHandle InvalidTransactionHandle = 0;
inline constexpr OperationHandle   InvalidOperationHandle   = 0;
inline constexpr uint32_t          MaxPatchSize             = 1U * 1'024U * 1'024U;

enum class Result : uint32_t {
	Success               = 0,
	InvalidArgument       = 1,
	Unsupported           = 2,
	OwnerInactive         = 3,
	NotFound              = 4,
	InvalidState          = 5,
	Conflict              = 6,
	ExpectedBytesMismatch = 7,
	Unavailable           = 8,
	CommitFailed          = 9,
	RollbackFailed        = 10,
	CallbackFault         = 11,
};

enum class FailureReason : uint32_t {
	None                  = 0,
	EmptyTransaction      = 1,
	InvalidRange          = 2,
	Overlap               = 3,
	ExpectedBytesMismatch = 4,
	NativeHooksRequired   = 5,
	HookPreparationFailed = 6,
	MemoryWriteFailed     = 7,
	HookEnableFailed      = 8,
	TrackingFailed        = 9,
	RollbackFailed        = 10,
};

enum class CommitFlags : uint32_t {
	None                  = 0,
	OriginalStateRestored = 1U << 0,
};

constexpr auto operator |(CommitFlags left, CommitFlags right) noexcept -> CommitFlags {
	return static_cast<CommitFlags>(static_cast<uint32_t>(left) | static_cast<uint32_t>(right));
}

constexpr auto HasFlag(CommitFlags value, CommitFlags flag) noexcept -> bool {
	return (static_cast<uint32_t>(value) & static_cast<uint32_t>(flag)) != 0;
}

// The service copies expected and replacement bytes before stagePatchBytes
// returns. Both sizes must match and cannot exceed MaxPatchSize. Set flags and
// reserved fields to zero.
struct BytePatchRequest {
	uint32_t    structSize;
	uint32_t    flags;
	uint64_t    rva;
	const void* expected;
	uint32_t    expectedSize;
	uint32_t    reserved;
	const void* bytes;
	uint32_t    size;
	uint32_t    reserved2;
};

// A relative patch writes a near call or jump followed by NOP bytes. The
// target is another RVA in D2R.exe. Size must be at least 5 bytes. Set flags and
// reserved to zero.
struct Rel32PatchRequest {
	uint32_t       structSize;
	uint32_t       flags;
	uint64_t       rva;
	const void*    expected;
	uint32_t       expectedSize;
	Rel32PatchKind kind;
	uint64_t       targetRva;
	uint32_t       size;
	uint32_t       reserved;
};

// Inline hooks require PluginFlags::NativeHooks. The target is the plugin's
// hook function. Use getInlineHookOriginal after commit succeeds to get the
// function pointer that calls the original code. Set flags and reserved fields
// to zero.
struct InlineHookRequest {
	uint32_t    structSize;
	uint32_t    flags;
	uint64_t    rva;
	const void* expected;
	uint32_t    expectedSize;
	uint32_t    reserved;
	void*       target;
	uintptr_t   reserved2;
};

struct CommitResult {
	uint32_t        structSize;
	// OriginalStateRestored means that no operation from a failed commit remains
	// active. A successful commit keeps flags at None.
	CommitFlags     flags;
	Result          result;
	FailureReason   failureReason;
	OperationHandle operation;
};

inline constexpr uint32_t BytePatchRequestSize          = static_cast<uint32_t>(sizeof(BytePatchRequest));
inline constexpr uint32_t BytePatchRequestRequiredSize  = BytePatchRequestSize;
inline constexpr uint32_t Rel32PatchRequestSize         = static_cast<uint32_t>(sizeof(Rel32PatchRequest));
inline constexpr uint32_t Rel32PatchRequestRequiredSize = Rel32PatchRequestSize;
inline constexpr uint32_t InlineHookRequestSize         = static_cast<uint32_t>(sizeof(InlineHookRequest));
inline constexpr uint32_t InlineHookRequestRequiredSize = InlineHookRequestSize;
inline constexpr uint32_t CommitResultSize              = static_cast<uint32_t>(sizeof(CommitResult));
inline constexpr uint32_t CommitResultRequiredSize      = CommitResultSize;

using BeginTransactionFn      = Result(__cdecl*)(const PluginContext* context, TransactionHandle* transaction) noexcept;
using StageBytePatchFn        = Result(__cdecl*)(const PluginContext* context, TransactionHandle transaction, const BytePatchRequest* request, OperationHandle* operation) noexcept;
using StageRel32PatchFn       = Result(__cdecl*)(const PluginContext* context, TransactionHandle transaction, const Rel32PatchRequest* request, OperationHandle* operation) noexcept;
using StageInlineHookFn       = Result(__cdecl*)(const PluginContext* context, TransactionHandle transaction, const InlineHookRequest* request, OperationHandle* operation) noexcept;
using CommitFn                = Result(__cdecl*)(const PluginContext* context, TransactionHandle transaction, CommitResult* result) noexcept;
using GetInlineHookOriginalFn = Result(__cdecl*)(const PluginContext* context, TransactionHandle transaction, OperationHandle operation, void** original) noexcept;
using CancelTransactionFn     = Result(__cdecl*)(const PluginContext* context, TransactionHandle transaction) noexcept;

static_assert(sizeof(Result) == sizeof(uint32_t));
static_assert(sizeof(FailureReason) == sizeof(uint32_t));
static_assert(sizeof(CommitFlags) == sizeof(uint32_t));
static_assert(std::is_standard_layout_v<BytePatchRequest> && std::is_trivially_copyable_v<BytePatchRequest>);
static_assert(std::is_standard_layout_v<Rel32PatchRequest> && std::is_trivially_copyable_v<Rel32PatchRequest>);
static_assert(std::is_standard_layout_v<InlineHookRequest> && std::is_trivially_copyable_v<InlineHookRequest>);
static_assert(std::is_standard_layout_v<CommitResult> && std::is_trivially_copyable_v<CommitResult>);
static_assert(sizeof(BytePatchRequest) == 48);
static_assert(sizeof(Rel32PatchRequest) == 48);
static_assert(sizeof(InlineHookRequest) == 48);
static_assert(sizeof(CommitResult) == 24);

}

struct MutationService {
	static constexpr ServiceId Id         = ServiceId::Mutation;
	static constexpr uint32_t  AbiVersion = 1;

	uint32_t                           serviceSize;
	uint32_t                           serviceVersion;
	// Starts an empty transaction owned by the calling plugin.
	Mutations::BeginTransactionFn      beginTransaction;
	// Staging copies each request and does not change game memory.
	Mutations::StageBytePatchFn        stageBytePatch;
	Mutations::StageRel32PatchFn       stageRel32Patch;
	Mutations::StageInlineHookFn       stageInlineHook;
	// Commit may be called once. Check both its return value and CommitResult.
	Mutations::CommitFn                commit;
	// The original pointer becomes available only after a successful commit.
	Mutations::GetInlineHookOriginalFn getInlineHookOriginal;
	// Cancels a transaction that did not commit. Successful changes remain until
	// the plugin unloads.
	Mutations::CancelTransactionFn     cancelTransaction;
};

inline constexpr uint32_t MutationServiceSize         = static_cast<uint32_t>(sizeof(MutationService));
inline constexpr uint32_t MutationServiceRequiredSize = MutationServiceSize;

inline auto HasMutationServiceField(const MutationService* service, uint32_t fieldEndOffset) noexcept -> bool {
	return service != nullptr && service->serviceVersion == MutationService::AbiVersion && service->serviceSize >= fieldEndOffset;
}

static_assert(std::is_standard_layout_v<MutationService> && std::is_trivially_copyable_v<MutationService>);
static_assert(sizeof(MutationService) == 64);

}
