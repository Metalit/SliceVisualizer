#pragma once

#include "GlobalNamespace/NoteCutInfo.hpp"

#include "UnityEngine/Color.hpp"

void Init();
void SetColors(UnityEngine::Color leftColor, UnityEngine::Color rightColor);
void CreateSlice(GlobalNamespace::NoteCutInfo& cutInfo);
void Update();
