#include "main.hpp"
#include "config.hpp"

DEFINE_CONFIG(ModConfig);

using namespace QuestUI;

inline void SetButtons(IncrementSetting* increment) {
    auto child = increment->get_gameObject()->get_transform()->GetChild(1);
    auto decButton = child->GetComponentsInChildren<UnityEngine::UI::Button*>().First();
    auto incButton = child->GetComponentsInChildren<UnityEngine::UI::Button*>().Last();
    increment->OnValueChange = [oldFunc = std::move(increment->OnValueChange), increment, decButton, incButton](float value) {
        oldFunc(value);
        decButton->set_interactable(value > increment->MinValue);
        incButton->set_interactable(value < increment->MaxValue);
    };
    decButton->set_interactable(increment->CurrentValue > increment->MinValue);
    incButton->set_interactable(increment->CurrentValue < increment->MaxValue);
}

void SettingsDidActivate(HMUI::ViewController* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
	if(firstActivation) {
		auto vertical = BeatSaberUI::CreateVerticalLayoutGroup(self);
		vertical->set_childControlHeight(false);
		vertical->set_childForceExpandHeight(false);
		vertical->set_spacing(1);
		auto transform = vertical->get_transform();

		AddConfigValueToggle(transform, getModConfig().Enabled);
		AddConfigValueToggle(transform, getModConfig().Dynamic);
		SetButtons(AddConfigValueIncrementFloat(transform, getModConfig().FadeSpeed, 1, 0.1, 0.1, 2));
    }
}
