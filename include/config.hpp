#pragma once

#include "HMUI/ViewController.hpp"

#include "config-utils/shared/config-utils.hpp"

DECLARE_CONFIG(ModConfig,
    CONFIG_VALUE(Enabled, bool, "Mod Enabled", true)
    CONFIG_VALUE(Dynamic, bool, "Dynamic Fade Speed", true, "When enabled, slice visuals will fade faster during faster parts of songs")
    CONFIG_VALUE(FadeSpeed, float, "Fade Speed", 1, "The speed that slice visuals fade at, a higher value means it fades quicker")
)

void SettingsDidActivate(HMUI::ViewController* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling);
