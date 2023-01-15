#pragma once

#include "modloader/shared/modloader.hpp"

#include "beatsaber-hook/shared/config/config-utils.hpp"
#include "beatsaber-hook/shared/utils/hooking.hpp"

Logger& getLogger();

#define LOG_INFO(...) getLogger().info(__VA_ARGS__)
#define LOG_DEBUG(...)
// #define LOG_DEBUG(...) getLogger().debug(__VA_ARGS__)
