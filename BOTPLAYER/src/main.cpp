#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include "ManualTab.hpp"
#include "GeneticBot.hpp"
#include "BotPlayerPopup.hpp"

using namespace geode::prelude;

// State global mod (sementara simpel dulu, nanti bisa dipindah ke class Popup)
ManualTab g_manualTab;
GeneticBot g_geneticBot;

// Index frame macro yang lagi diputar (buat playback Manual & Automatic)
size_t g_playbackCursor = 0;
bool g_lastPlaybackHold = false;

// Referensi ke PlayLayer aktif, dipakai buat baca frame saat ini dari mana aja
PlayLayer* g_currentPlayLayer = nullptr;

// (Hook PlayerObject::pushButton dihapus -- terbukti nggak ke-trigger buat
// sentuhan layar asli di device ini, 0 frame kerekam. Ganti pakai handleButton
// di PlayLayer, titik hook yang lebih universal buat nangkep semua jenis input.)

// Helper: apply satu macro ke player di frame saat ini, dipake bareng
// buat playback Manual maupun Automatic (logic-nya sama persis)
static void applyMacroInput(PlayerObject* player, const std::vector<InputFrame>& macroFrames, int frame) {
    while (g_playbackCursor < macroFrames.size() &&
           macroFrames[g_playbackCursor].frame <= frame) {
        bool shouldHold = macroFrames[g_playbackCursor].holding;
        if (shouldHold != g_lastPlaybackHold) {
            if (player) {
                if (shouldHold) player->pushButton(PlayerButton::Jump);
                else player->releaseButton(PlayerButton::Jump);
            }
            g_lastPlaybackHold = shouldHold;
        }
        g_playbackCursor++;
    }
}

class $modify(BPPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;
        g_currentPlayLayer = this;

        // Tombol buka popup BOTPLAYER ditaro di sini (bukan di PauseLayer),
        // karena hook PauseLayer::init selalu SIGILL di device ini -- PlayLayer
        // udah kebukti aman di-hook (record/playback jalan tanpa crash)
        auto buttonSprite = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
        if (buttonSprite) {
            auto button = CCMenuItemSpriteExtra::create(
                buttonSprite, this, menu_selector(BPPlayLayer::onOpenBotPlayer)
            );
            auto menu = CCMenu::create();
            menu->addChild(button);
            menu->setPosition({ 40.f, 40.f }); // pojok kiri bawah layar
            this->addChild(menu, 100); // z-order tinggi biar nggak ketutup objek level
        }

        return true;
    }

    void onOpenBotPlayer(CCObject*) {
        BotPlayerPopup::create()->show();
    }

    // Dipanggil GD buat SETIAP jenis input (keyboard, touch, dll) -- lebih
    // reliable daripada hook langsung ke PlayerObject::pushButton
    void handleButton(bool down, int button, bool isPlayer1) {
        PlayLayer::handleButton(down, button, isPlayer1);

        log::info("BOTPLAYER DEBUG: handleButton down={} button={} p1={}", down, button, isPlayer1); // <- diagnostik

        // (Filter "button != 1" dihapus SEMENTARA buat testing -- siapa tau
        // ID tombol jump di touch/mobile beda dari 1, jadi kita rekam semua dulu)

        if (g_manualTab.getState() == ManualTab::State::Recording) {
            int frame = (int)m_gameState.m_currentProgress;
            log::info("BOTPLAYER DEBUG: recording via handleButton, frame={} down={}", frame, down); // <- diagnostik
            g_manualTab.onFrame(frame, down);
        }
    }

    void update(float dt) {
        PlayLayer::update(dt);

        int frame = m_gameState.m_currentProgress; // contoh, nanti disesuaikan API asli

        // Tab Manual: kalau lagi playback, tiru input dari macro yg tersimpan
        if (g_manualTab.getState() == ManualTab::State::Playing) {
            const auto& macroFrames = g_manualTab.getMacro().frames;
            applyMacroInput(m_player1, macroFrames, frame);

            // Macro habis -> otomatis stop
            if (g_playbackCursor >= macroFrames.size()) {
                g_manualTab.stopPlayback();
                g_playbackCursor = 0;
            }
        }

        // Tab Automatic: kalau lagi jalan, GeneticBot yang ambil alih input
        if (g_geneticBot.isRunning()) {
            const auto& macroFrames = g_geneticBot.nextIndividual().frames;
            applyMacroInput(m_player1, macroFrames, frame);
        }
    }

    void resetLevel() {
        // Kalau lagi mode Automatic, sebelum reset, laporin dulu progress attempt yg abis mati
        if (g_geneticBot.isRunning()) {
            float progressPercent = m_gameState.m_currentProgress; // sesuaikan sama API progress asli GD
            g_geneticBot.reportResult(progressPercent);
            g_playbackCursor = 0;
            g_lastPlaybackHold = false;

            if (g_geneticBot.generationDone()) {
                g_geneticBot.evolve();
            }
        }

        PlayLayer::resetLevel();
    }
};

$execute {
    log::info("BOTPLAYER loaded!");
}