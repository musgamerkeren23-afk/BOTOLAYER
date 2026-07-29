#pragma once
#include <Geode/Geode.hpp>
#include <Geode/ui/Popup.hpp>
#include "ManualTab.hpp"
#include "GeneticBot.hpp"

using namespace geode::prelude;

// Tab dipindah ke global scope biar bisa disimpen di variabel global
// (g_lastSelectedTab), supaya tab yang lagi aktif nggak reset tiap popup dibuka ulang
enum class BotPlayerTab { Manual, Automatic };

// Tab terakhir yang aktif, bertahan lintas buka-tutup popup (didefinisikan di .cpp)
extern BotPlayerTab g_lastSelectedTab;

class BotPlayerPopup : public geode::Popup {
protected:
    static constexpr float POPUP_WIDTH = 360.f;
    static constexpr float POPUP_HEIGHT = 260.f;

    CCMenu* m_tabMenu = nullptr;
    CCLayer* m_manualLayer = nullptr;
    CCLayer* m_automaticLayer = nullptr;

    CCLabelBMFont* m_manualStatusLabel = nullptr;
    CCLabelBMFont* m_autoStatusLabel = nullptr;

    bool init();

    void switchTab(BotPlayerTab tab);
    CCLayer* buildManualTab();
    CCLayer* buildAutomaticTab();
    void refreshManualStatusLabel();
    void refreshAutoStatusLabel();

    void onTabManual(CCObject*);
    void onTabAutomatic(CCObject*);
    void onRecordToggle(CCObject*);
    void onPlayback(CCObject*);
    void onAutoToggle(CCObject*);

public:
    static BotPlayerPopup* create();
};
