#include "main.hpp"
#include "config.hpp"
#include "slices.hpp"
#include "questui/shared/QuestUI.hpp"

using namespace GlobalNamespace;

static ModInfo modInfo;

Logger& getLogger() {
    static auto logger = new Logger(modInfo, LoggerOptions(false, true));
    return *logger;
}

#include "GlobalNamespace/AudioTimeSyncController.hpp"

MAKE_HOOK_MATCH(AudioTimeSyncController_Start, &AudioTimeSyncController::Start, void, AudioTimeSyncController* self) {

    AudioTimeSyncController_Start(self);

    if(getModConfig().Enabled.GetValue())
        Init();
}

MAKE_HOOK_MATCH(AudioTimeSyncController_StartSong, &AudioTimeSyncController::StartSong, void, AudioTimeSyncController* self, float startTimeOffset) {

    if(getModConfig().Enabled.GetValue())
        MakeSprites();

    AudioTimeSyncController_StartSong(self, startTimeOffset);
}

MAKE_HOOK_MATCH(AudioTimeSyncController_Update, &AudioTimeSyncController::Update, void, AudioTimeSyncController* self) {

    AudioTimeSyncController_Update(self);

    if(getModConfig().Enabled.GetValue())
        Update();
}

#include "GlobalNamespace/NoteController.hpp"
#include "GlobalNamespace/NoteData.hpp"

MAKE_HOOK_MATCH(NoteController_SendNoteWasCutEvent, &NoteController::SendNoteWasCutEvent, void, NoteController* self, ByRef<NoteCutInfo> noteCutInfo) {

    if(noteCutInfo->get_allIsOK() && getModConfig().Enabled.GetValue() && self->noteData->gameplayType != NoteData::GameplayType::BurstSliderElement)
        CreateSlice(noteCutInfo.heldRef);

    NoteController_SendNoteWasCutEvent(self, noteCutInfo);
}

#include "GlobalNamespace/GameplayCoreSceneSetupData.hpp"
#include "GlobalNamespace/ColorScheme.hpp"

MAKE_HOOK_FIND_CLASS_UNSAFE_INSTANCE(GameplayCoreSceneSetupData_ctor, "", "GameplayCoreSceneSetupData", ".ctor", void, GameplayCoreSceneSetupData* self, IDifficultyBeatmap* f1, IPreviewBeatmapLevel* f2, GameplayModifiers* f3, PlayerSpecificSettings* f4, PracticeSettings* f5, bool f6, EnvironmentInfoSO* f7, ColorScheme* colorScheme, MainSettingsModelSO* f8, BeatmapDataCache* f9) {

    SetColors(colorScheme->get_saberAColor(), colorScheme->get_saberBColor());

    GameplayCoreSceneSetupData_ctor(self, f1, f2, f3, f4, f5, f6, f7, colorScheme, f8, f9);
}

void SettingsDidActivate(HMUI::ViewController* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
	if(firstActivation) {
		auto vertical = QuestUI::BeatSaberUI::CreateVerticalLayoutGroup(self);
		vertical->set_childControlHeight(false);
		vertical->set_childForceExpandHeight(false);
		vertical->set_spacing(1);

		AddConfigValueToggle(vertical, getModConfig().Enabled);
		AddConfigValueToggle(vertical, getModConfig().Dynamic);
		AddConfigValueIncrementFloat(vertical, getModConfig().FadeSpeed, 1, 0.1, 0.1, 2);
    }
}

extern "C" void setup(ModInfo& info) {
    info.id = MOD_ID;
    info.version = VERSION;
    modInfo = info;

    getModConfig().Init(modInfo);
}

extern "C" void load() {
    QuestUI::Init();

    QuestUI::Register::RegisterModSettingsViewController(modInfo, SettingsDidActivate);

    LOG_INFO("Installing hooks...");
    INSTALL_HOOK(getLogger(), AudioTimeSyncController_Start);
    INSTALL_HOOK(getLogger(), AudioTimeSyncController_StartSong);
    INSTALL_HOOK(getLogger(), AudioTimeSyncController_Update);
    INSTALL_HOOK(getLogger(), NoteController_SendNoteWasCutEvent);
    INSTALL_HOOK(getLogger(), GameplayCoreSceneSetupData_ctor);
    LOG_INFO("Installed all hooks!");
}
