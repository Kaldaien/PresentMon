#include "PowerTelemetryContainer.h"
#include <algorithm>
#include <ranges>
#include <functional>

bool PowerTelemetryContainer::QueryPowerTelemetrySupport() { 
  try {
    telemetry_providers_.clear();
    telemetry_adapters_.clear();

    // create providers
    for (int iVendor = 0; iVendor < int(PM_GPU_VENDOR_UNKNOWN); iVendor++) {
      try {
        if (auto pProvider = pwr::PowerTelemetryProviderFactory::Make(
                PM_GPU_VENDOR(iVendor))) {
          telemetry_providers_.push_back(std::move(pProvider));
        }
      } catch (...) {
      }  // silent fail (maybe log?) any provider construction
         // exceptions and just keep the good ones
    }
    // collect all adapters together from providers
    for (const auto& pProvider : telemetry_providers_) {
      auto& adapters = pProvider->GetAdapters();
      telemetry_adapters_.insert(telemetry_adapters_.end(), adapters.begin(),
                                 adapters.end());
    }
    // bail if there are not adapters
    if (telemetry_adapters_.size() == 0) {
      return false;
    }
  } catch (...) {
    return false;
  }
  // Re-sort adapters based on video memory size in an attempt to return back the most
  // capable adapters
  constexpr auto ComparisonField = &pwr::PowerTelemetryAdapter::GetDedicatedVideoMemory;
  std::ranges::sort(telemetry_adapters_, std::greater{}, ComparisonField);
  return true;
}