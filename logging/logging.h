#ifndef P2PCHAT_LOGGING_H
#define P2PCHAT_LOGGING_H

#include <spdlog/spdlog.h>
#include "spdlog/fmt/bin_to_hex.h"

/*
 I wanted this function to be called automatically when .so file is loaded.
 However, in this multi-threaded program this method didn't work as expected.
 Therefore, I call this function manually when the program starts.
 */
void init_logging(); //__attribute__((constructor));
std::shared_ptr<spdlog::logger> getLogger();

#endif
