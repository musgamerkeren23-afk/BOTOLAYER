#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include "ManualTab.hpp"
#include "GeneticBot.hpp"

using namespace geode::prelude;

class BotPlayerPopup : public Popup<> {
protected:
    enum class Tab { Manual, Automatic };

    CCMenu* m_tabMenu = nullptr;
    CCLayer* m_manualLayer = nullptr;
    CCLayer* m_automaticLayer = nullptr;

    CCLabelBMFont* m_manualStatusLabel = nullptr;
    CCLabelBMFont* m_autoStatusLabel = nullptr;

    Tab m_currentTab = Tab::Manual;

    bool setup() override;

    void switchTab(Tab tab);
    CCLayer* buildManualTab();
    CCLayer* buildAutomaticTab();

    void onTabManual(CCObject*);
    void onTabAutomatic(CCObject*);
    void onRecordToggle(CCObject*);
    void onPlayback(CCObject*);
    void onAutoToggle(CCObject*);

public:
    static BotPlayerPopup* create();
};
