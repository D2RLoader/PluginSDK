#include <D2RLPlugin/api.h>
#include <array>
#include <cstddef>
#include <cstdio>
#include <vector>

static constexpr D2RL::PluginInfo DataTablesPluginInfo {
	.infoSize    = D2RL::PluginInfoSize,
	.apiVersion  = D2RL_PLUGIN_API_VERSION,
	.id          = "data-tables-sample",
	.name        = "Data Tables Sample Plugin",
	.version     = "0.1.0",
	.author      = "D2RLoader",
	.description = "Reads compiled Excel tables after each load.",
	.flags       = D2RL::PluginFlags::Shared,
};

struct OfferRow {
	char     itemCode[4];
	uint32_t price;
};

static_assert(sizeof(OfferRow) == 8);

static constexpr char OfferTableText[] = "item\tprice\nr01\t3\ncap\t25\n";

static constexpr std::array<D2RL::CustomTables::ColumnDefinition, 2> OfferColumns {
	D2RL::CustomTables::ColumnDefinition {
										  .structSize = D2RL::CustomTables::ColumnDefinitionSize,
										  .name       = "item",
										  .type       = D2RL::CustomTables::ColumnType::Ascii,
										  .offset     = static_cast<uint32_t>(offsetof(OfferRow, itemCode)),
										  .length     = sizeof(OfferRow::itemCode),
										  },
	D2RL::CustomTables::ColumnDefinition {
										  .structSize = D2RL::CustomTables::ColumnDefinitionSize,
										  .name       = "price",
										  .type       = D2RL::CustomTables::ColumnType::Dword,
										  .offset     = static_cast<uint32_t>(offsetof(OfferRow, price)),
										  },
};

static const D2RL::DataTableServiceV1*   dataTables;
static const D2RL::CustomTableServiceV1* customTables;
static D2RL::CustomTables::TableHandle   offerTable = D2RL::CustomTables::InvalidHandle;

static auto ReportLevels(const D2RL::PluginContext* context, uint64_t expectedRevision) noexcept -> bool {
	D2RL::DataTables::TableView levels {
		.structSize = D2RL::DataTables::TableViewSize,
	};
	const auto tableResult = dataTables->getTable(context, D2RL::DataTables::Bank::Rotw, D2RL::DataTables::TableId::Levels, &levels);
	if (tableResult != D2RL::DataTables::Result::Success || levels.revision != expectedRevision) {
		return false;
	}

	D2RL::DataTables::RowView level {
		.structSize = D2RL::DataTables::RowViewSize,
	};
	const auto rowResult = dataTables->findRowById(context, D2RL::DataTables::Bank::Rotw, D2RL::DataTables::TableId::Levels, 124, &level);
	if (rowResult != D2RL::DataTables::Result::Success || level.revision != expectedRevision) {
		return false;
	}

	const auto revision = static_cast<unsigned long long>(levels.revision);
	char       message[192] {};
	std::snprintf(message, sizeof(message), "RotW Levels has %u compiled rows of %u bytes; level 124 is row %u (revision %llu).", levels.rowCount, levels.rowSize, level.rowIndex, revision);
	context->LogInfo(message);
	return true;
}

static auto ReportOffers(const D2RL::PluginContext* context) noexcept -> bool {
	try {
		D2RL::CustomTables::TableInfo info {
			.structSize = D2RL::CustomTables::TableInfoSize,
		};
		if (customTables->getTableInfo(context, offerTable, D2RL::CustomTables::TableBank::Rotw, &info) != D2RL::CustomTables::Result::Success) {
			return false;
		}

		if (info.state != D2RL::CustomTables::TableState::Ready) {
			return false;
		}

		if (info.rowSize != sizeof(OfferRow)) {
			return false;
		}
		const uint64_t expectedByteCount = static_cast<uint64_t>(info.rowCount) * sizeof(OfferRow);
		if (info.byteCount != expectedByteCount) {
			return false;
		}

		std::vector<OfferRow> rows(info.rowCount);
		if (customTables->copyRows(context, offerTable, D2RL::CustomTables::TableBank::Rotw, info.revision, rows.data(), info.byteCount) != D2RL::CustomTables::Result::Success) {
			return false;
		}
		char message[160] {};
		if (!rows.empty()) {
			std::snprintf(message, sizeof(message), "Custom offers table copied %u row(s); first item is %.4s for %u.", info.rowCount, rows.front().itemCode, rows.front().price);
		} else {
			std::snprintf(message, sizeof(message), "Custom offers table is ready with no rows.");
		}
		context->LogInfo(message);
		return true;
	} catch (...) {
		return false;
	}
}

static void LogReadFailure(const D2RL::PluginContext* context) noexcept {
	context->LogWarn("The data-tables sample could not read RotW Levels after the load completed.");
}

static void __cdecl OnDataTablesLoaded(const D2RL::PluginContext* context, const D2RL::Lifecycle::DataTablesLoadedEvent* event, void* /*userData*/) noexcept {
	if (context == nullptr) {
		return;
	}

	if (dataTables == nullptr || customTables == nullptr) {
		LogReadFailure(context);
		return;
	}

	if (!D2RL::Lifecycle::HasDataTablesLoadedEventField(event, D2RL::Lifecycle::DataTablesLoadedEventRequiredSize)) {
		LogReadFailure(context);
		return;
	}

	if (!ReportLevels(context, event->revision)) {
		LogReadFailure(context);
		return;
	}

	if (!ReportOffers(context)) {
		LogReadFailure(context);
	}
}

D2RL_PLUGIN_EXPORT auto D2RLoaderGetPluginInfo() noexcept -> const D2RL::PluginInfo* {
	return &DataTablesPluginInfo;
}

D2RL_PLUGIN_EXPORT auto D2RLoaderLoadPlugin(const D2RL::PluginContext* context) noexcept -> bool {
	if (context == nullptr) {
		return false;
	}

	const D2RL::DataTableServiceV1* tables          = nullptr;
	const auto                      dataTableResult = context->QueryService(D2RL::ServiceId::DataTable, D2RL::DataTableServiceV1Version, &tables);
	if (dataTableResult != D2RL::ServiceQueryResult::Success || tables == nullptr) {
		return false;
	}
	dataTables = tables;
	if (!D2RL::HasDataTableServiceV1Field(dataTables, D2RL::DataTableServiceV1RequiredSize)) {
		return false;
	}

	const D2RL::ResourceServiceV1* resources             = nullptr;
	const auto                     resourceServiceResult = context->QueryService(D2RL::ServiceId::Resource, D2RL::ResourceServiceV1Version, &resources);
	if (resourceServiceResult != D2RL::ServiceQueryResult::Success || !D2RL::HasResourceServiceV1Field(resources, D2RL::ResourceServiceV1RequiredSize)) {
		return false;
	}
	const D2RL::CustomTableServiceV1* customTableService = nullptr;
	const auto                        customTableResult  = context->QueryService(D2RL::ServiceId::CustomTable, D2RL::CustomTableServiceV1Version, &customTableService);
	if (customTableResult != D2RL::ServiceQueryResult::Success || !D2RL::HasCustomTableServiceV1Field(customTableService, D2RL::CustomTableServiceV1RequiredSize)) {
		return false;
	}
	customTables = customTableService;

	const D2RL::Resources::ResourceRegistration resource {
		.structSize = D2RL::Resources::ResourceRegistrationSize,
		.path       = "data/global/excel/d2rloader/data-tables-sample/offers.txt",
		.bytes      = OfferTableText,
		.byteCount  = sizeof(OfferTableText) - 1,
	};
	D2RL::Resources::RegistrationHandle resourceHandle = D2RL::Resources::InvalidHandle;
	if (resources->registerResource(context, &resource, &resourceHandle) != D2RL::Resources::Result::Success) {
		return false;
	}
	const D2RL::CustomTables::TableRegistration table {
		.structSize   = D2RL::CustomTables::TableRegistrationSize,
		.name         = "offers",
		.banks        = D2RL::CustomTables::TableBank::Rotw,
		.rowSize      = sizeof(OfferRow),
		.columns      = OfferColumns.data(),
		.columnCount  = static_cast<uint32_t>(OfferColumns.size()),
		.columnStride = D2RL::CustomTables::ColumnDefinitionSize,
	};
	if (customTables->registerTable(context, &table, &offerTable) != D2RL::CustomTables::Result::Success) {
		return false;
	}

	const D2RL::LifecycleServiceV1* lifecycle       = nullptr;
	const auto                      lifecycleResult = context->QueryService(D2RL::ServiceId::Lifecycle, D2RL::LifecycleServiceV1Version, &lifecycle);
	if (lifecycleResult != D2RL::ServiceQueryResult::Success || lifecycle == nullptr) {
		return false;
	}

	if (!D2RL::HasLifecycleServiceV1Field(lifecycle, D2RL::LifecycleServiceV1RequiredSize)) {
		return false;
	}

	const D2RL::Lifecycle::DataTablesLoadedListener listener {
		.structSize = D2RL::Lifecycle::DataTablesLoadedListenerSize,
		.flags      = 0,
		.callback   = OnDataTablesLoaded,
		.userData   = nullptr,
	};
	D2RL::Lifecycle::ListenerHandle handle = D2RL::Lifecycle::InvalidHandle;
	return lifecycle->registerDataTablesLoadedListener(context, &listener, &handle) == D2RL::Lifecycle::Result::Success && handle != D2RL::Lifecycle::InvalidHandle;
}

D2RL_PLUGIN_EXPORT void D2RLoaderUnloadPlugin() noexcept {}
