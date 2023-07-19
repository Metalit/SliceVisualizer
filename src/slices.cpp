#include "main.hpp"
#include "slices.hpp"
#include "sprites.hpp"
#include "config.hpp"

#include "GlobalNamespace/ComboUIController.hpp"
#include "GlobalNamespace/NoteCutDirectionExtensions.hpp"
#include "GlobalNamespace/NoteData.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnController.hpp"
#include "GlobalNamespace/BeatmapObjectSpawnMovementData.hpp"

#include "UnityEngine/UI/Mask.hpp"
#include "UnityEngine/Time.hpp"

#include "questui/shared/BeatSaberUI.hpp"

using namespace GlobalNamespace;
using namespace UnityEngine;
using namespace QuestUI;

struct Slice {
    GameObject* parent;
    UI::Image* typeImage;
    UI::Image* backgroundImage;
    UI::Image* line;
    float opacity;
};

std::vector<Slice> cuts;
float nextNoteTime;

Transform* mainGO;

Sprite* arrowSprite;
Sprite* dotSprite;
Sprite* arrowBackgroundSprite;
Sprite* dotBackgroundSprite;
Material* spriteMaterial;

BeatmapObjectSpawnController* spawnController;

Color leftColor;
Color rightColor;

bool MakeSprites() {
    LOG_INFO("Initializing sprites");
    arrowSprite = BeatSaberUI::Base64ToSprite(arrowBase64);
    dotSprite = BeatSaberUI::Base64ToSprite(dotBase64);
    arrowBackgroundSprite = BeatSaberUI::Base64ToSprite(arrowBackgroundBase64);
    dotBackgroundSprite = BeatSaberUI::Base64ToSprite(dotBackgroundBase64);
    spriteMaterial = Resources::FindObjectsOfTypeAll<Material*>().First([](auto x) { return x->get_name() == "UINoGlow"; });
    if(!spriteMaterial)
        return false;
    spawnController = Resources::FindObjectsOfTypeAll<BeatmapObjectSpawnController*>().FirstOrDefault();
    if(!spawnController)
        return false;
    return true;
}

bool Init() {
    LOG_INFO("Initializing empty slices set");
    cuts.clear();
    auto comboController = UnityEngine::Object::FindObjectOfType<ComboUIController*>();
    if(!comboController)
        return false;
    mainGO = GameObject::New_ctor("SliceVisualizerGO")->get_transform();
    mainGO->set_position({0, 3, 15});
    mainGO->set_localScale({0.01, 0.01, 0.01});
    mainGO->SetParent(comboController->get_transform(), true);
    return true;
}

void SetColors(Color leftCol, Color rightCol) {
    leftColor = leftCol;
    rightColor = rightCol;
}

UI::Image* CreateImage(Transform* parent, Sprite* sprite, std::string name) {
    auto object = GameObject::New_ctor(name);
    auto image = object->AddComponent<UI::Image*>();
    image->set_sprite(sprite);
    image->set_material(spriteMaterial);
    auto trans = object->get_transform();
    trans->SetParent(parent, false);
    return image;
}

void CreateSlice(NoteCutInfo& cutInfo) {
    static float spriteSize = 0.6;

    nextNoteTime = cutInfo.noteData->timeToNextColorNote;

    Sprite* bgSprite;
    Sprite* fgSprite;
    if(cutInfo.noteData->cutDirection == NoteCutDirection::Any) {
        bgSprite = dotBackgroundSprite;
        fgSprite = dotSprite;
    } else {
        bgSprite = arrowBackgroundSprite;
        fgSprite = arrowSprite;
    }

    auto parent = GameObject::New_ctor("SliceGraphics")->get_transform();
    parent->SetParent(mainGO, false);
    parent->set_localEulerAngles({0, 0, NoteCutDirectionExtensions::RotationAngle(cutInfo.noteData->cutDirection)});
    auto pos = parent->get_position();
    auto newPos = spawnController->beatmapObjectSpawnMovementData->Get2DNoteOffset(cutInfo.noteData->lineIndex, cutInfo.noteData->noteLineLayer);
    parent->set_position({pos.x + newPos.x, pos.y + newPos.y, pos.z});
    parent->set_localScale({spriteSize, spriteSize, spriteSize});

    auto background = CreateImage(parent, bgSprite, "SpriteImage");
    ColorType colorType = cutInfo.noteData->colorType;
    if(colorType == ColorType::ColorA) {
        background->set_color(leftColor);
    } else {
        background->set_color(rightColor);
    }
    background->get_gameObject()->AddComponent<UI::Mask*>()->set_showMaskGraphic(true);

    auto arrow = CreateImage(background->get_transform(), fgSprite, "SpriteArrowImage");

    auto line = CreateImage(background->get_transform(), nullptr, "CutLine");
    line->set_color(Color::get_black());
    line->GetComponent<RectTransform*>()->set_sizeDelta({5, 100});
    auto trans = line->get_transform();
    trans->set_localScale({1/spriteSize, 1/spriteSize, 1/spriteSize});
    trans->set_localEulerAngles({0, 0, cutInfo.cutDirDeviation});
    trans->set_localPosition({(cutInfo.cutDistanceToCenter * 120/spriteSize) - (2.3f/spriteSize), -2.3f/spriteSize, 0});

    cuts.push_back(Slice{
        .parent = parent->get_gameObject(),
        .typeImage = arrow,
        .backgroundImage = background,
        .line = line,
        .opacity = 1
    });
}

void Update() {
    LOG_DEBUG("updating slices, number: %lu", cuts.size());
    for(auto iter = cuts.begin(); iter != cuts.end(); iter++) {
        auto& cut = *iter;
        float dynamicMultiplier = 1;
        if(getModConfig().Dynamic.GetValue())
            dynamicMultiplier = std::clamp(2 - (nextNoteTime * 1.5), 0.4, 2.0);
        float decrease = std::min(0.4f, 1.01f - cut.opacity);
        cut.opacity -= decrease * Time::get_deltaTime() * getModConfig().FadeSpeed.GetValue() * 8 * dynamicMultiplier;
        if(cut.opacity < 0) {
            Object::Destroy(cut.parent);
            iter = cuts.erase(iter) - 1;
        } else {
            auto color = cut.backgroundImage->get_color();
            color.a = cut.opacity;
            cut.backgroundImage->set_color(color);
            cut.typeImage->set_color({1, 1, 1, cut.opacity});
            cut.line->set_color({0, 0, 0, cut.opacity * 2.2f});
        }
    }
}
