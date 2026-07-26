#include "BotPlayerPopup.hpp"

extern ManualTab g_manualTab;
extern GeneticBot g_geneticBot;

BotPlayerPopup* BotPlayerPopup::create() {
    auto ret = new BotPlayerPopup();
    if (ret->init()) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool BotPlayerPopup::init() {
    if (!Popup::init(POPUP_WIDTH, POPUP_HEIGHT)) return false;

    this->setTitle("BOTPLAYER");

    m_tabMenu = CCMenu::create();
    m_tabMenu->setPosition({ POPUP_WIDTH / 2, POPUP_HEIGHT - 45.f });

    auto manualBtnSpr = ButtonSprite::create("Manual");
    auto manualBtn = CCMenuItemSpriteExtra::create(
        manualBtnSpr, this, menu_selector(BotPlayerPopup::onTabManual)
    );
    manualBtn->setPosition({ -55.f, 0.f });

    auto autoBtnSpr = ButtonSprite::create("Automatic");
    auto autoBtn = CCMenuItemSpriteExtra::create(
        autoBtnSpr, this, menu_selector(BotPlayerPopup::onTabAutomatic)
    );
    autoBtn->setPosition({ 55.f, 0.f });

    m_tabMenu->addChild(manualBtn);
    m_tabMenu->addChild(autoBtn);
    m_mainLayer->addChild(m_tabMenu);

    m_manualLayer = buildManualTab();
    m_automaticLayer = buildAutomaticTab();
    m_mainLayer->addChild(m_manualLayer);
    m_mainLayer->addChild(m_automaticLayer);

    switchTab(Tab::Manual);

    return true;
}

void BotPlayerPopup::switchTab(Tab tab) {
    m_currentTab = tab;
    m_manualLayer->setVisible(tab == Tab::Manual);
    m_automaticLayer->setVisible(tab == Tab::Automatic);
}

CCLayer* BotPlayerPopup::buildManualTab() {
    auto layer = CCLayer::create();
    layer->setPosition({ 0.f, 0.f });

    auto menu = CCMenu::create();
    menu->setPosition({ POPUP_WIDTH / 2, POPUP_HEIGHT / 2 - 20.f });

    auto recordBtnSpr = ButtonSprite::create("Record / Stop");
    auto recordBtn = CCMenuItemSpriteExtra::create(
        recordBtnSpr, this, menu_selector(BotPlayerPopup::onRecordToggle)
    );
    recordBtn->setPosition({ 0.f, 30.f });

    auto playBtnSpr = ButtonSprite::create("Playback");
    auto playBtn = CCMenuItemSpriteExtra::create(
        playBtnSpr, this, menu_selector(BotPlayerPopup::onPlayback)
    );
    playBtn->setPosition({ 0.f, -20.f });

    menu->addChild(recordBtn);
    menu->addChild(playBtn);
    layer->addChild(menu);

    m_manualStatusLabel = CCLabelBMFont::create("Status: Idle", "bigFont.fnt");
    m_manualStatusLabel->setScale(0.4f);
    m_manualStatusLabel->setPosition({ POPUP_WIDTH / 2, POPUP_HEIGHT / 2 - 70.f });
    layer->addChild(m_manualStatusLabel);

    return layer;
}

CCLayer* BotPlayerPopup::buildAutomaticTab() {
    auto layer = CCLayer::create();
    layer->setPosition({ 0.f, 0.f });

    auto menu = CCMenu::create();
    menu->setPosition({ POPUP_WIDTH / 2, POPUP_HEIGHT / 2 - 20.f });

    auto autoBtnSpr = CCSprite::createWithSpriteFrameName("GJ_playBtn_001.png");
    auto autoBtn = CCMenuItemSpriteExtra::create(
        autoBtnSpr, this, menu_selector(BotPlayerPopup::onAutoToggle)
    );
    autoBtn->setPosition({ 0.f, 10.f });

    menu->addChild(autoBtn);
    layer->addChild(menu);

    m_autoStatusLabel = CCLabelBMFont::create("Gen: 0 | Best: 0%", "bigFont.fnt");
    m_autoStatusLabel->setScale(0.4f);
    m_autoStatusLabel->setPosition({ POPUP_WIDTH / 2, POPUP_HEIGHT / 2 - 70.f });
    layer->addChild(m_autoStatusLabel);

    return layer;
}

void BotPlayerPopup::onTabManual(CCObject*) {
    switchTab(Tab::Manual);
}

void BotPlayerPopup::onTabAutomatic(CCObject*) {
    switchTab(Tab::Automatic);
}

void BotPlayerPopup::onRecordToggle(CCObject*) {
    if (g_manualTab.getState() == ManualTab::State::Recording) {
        g_manualTab.stopRecording();
        m_manualStatusLabel->setString(
            fmt::format("Status: Idle ({} frame tersimpan)", g_manualTab.getMacro().frames.size()).c_str()
        );
    } else {
        g_manualTab.startRecording();
        m_manualStatusLabel->setString("Status: Recording...");
    }
}

void BotPlayerPopup::onPlayback(CCObject*) {
    if (g_manualTab.getMacro().empty()) {
        FLAlertLayer::create("Oops", "macro ini kosong!", "OK")->show();
        return;
    }
    g_manualTab.startPlayback();
}

void BotPlayerPopup::onAutoToggle(CCObject*) {
    if (!g_geneticBot.isRunning()) {
        g_geneticBot.startNewLevel();
    } else {
        g_geneticBot.stop();
    }
}
