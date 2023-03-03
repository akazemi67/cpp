#ifndef P2PCHAT_LOGGING_H
#define P2PCHAT_LOGGING_H

#include <spdlog/spdlog.h>
#include "spdlog/fmt/bin_to_hex.h"

void init_logging();
std::shared_ptr<spdlog::logger> logger;

#endif
