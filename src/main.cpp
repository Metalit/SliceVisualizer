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

MAKE_HOOK_MATCH(AudioTimeSyncController_Update, &AudioTimeSyncController::Update, void, AudioTimeSyncController* self) {

    if(getModConfig().Enabled.GetValue())
        Update();

    AudioTimeSyncController_Update(self);
}

MAKE_HOOK_MATCH(AudioTimeSyncController_StartSong, &AudioTimeSyncController::StartSong, void, AudioTimeSyncController* self, float startTimeOffset) {

    AudioTimeSyncController_StartSong(self, startTimeOffset);

    if(getModConfig().Enabled.GetValue())
        Init();
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

MAKE_HOOK_FIND_CLASS_UNSAFE_INSTANCE(GameplayCoreSceneSetupData_ctor, "", "GameplayCoreSceneSetupData", ".ctor", void, GameplayCoreSceneSetupData* self, IDifficultyBeatmap* f1, IPreviewBeatmapLevel* f2, GameplayModifiers* f3, PlayerSpecificSettings* f4, PracticeSettings* f5, bool f6, EnvironmentInfoSO* f7, ColorScheme* colorScheme, MainSettingsModelSO* f8)
{
    SetColors(colorScheme->get_saberAColor(), colorScheme->get_saberBColor());
    GameplayCoreSceneSetupData_ctor(self, f1, f2, f3, f4, f5, f6, f7, colorScheme, f8);
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
    INSTALL_HOOK(getLogger(), AudioTimeSyncController_Update);
    INSTALL_HOOK(getLogger(), AudioTimeSyncController_StartSong);
    INSTALL_HOOK(getLogger(), NoteController_SendNoteWasCutEvent);
    INSTALL_HOOK(getLogger(), GameplayCoreSceneSetupData_ctor);
    LOG_INFO("Installed all hooks!");
}