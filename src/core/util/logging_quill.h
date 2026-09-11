#pragma once

#include "core/header.h"
#include <quill/Logger.h>

namespace horizon::core
{

// Optional Quill interop for legacy macros and advanced callers. Consumers of
// this header get Quill's usage requirements through horizon-core. Do not remove
// or mutate the logger's sinks: its lifetime and configuration belong to Core.
OC_CORE_API quill::Logger *get_quill_logger();

}  // namespace horizon::core
