#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/PauseLayer.hpp>
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

// Hook ke PlayerObject: ini yang dipanggil GD PERSIS pas pemain nekan/lepas tombol,
// jauh lebih akurat daripada nebak dari polling di update()
class $modify(BPPlayerObject, PlayerObject) {
    void pushButton(PlayerButton btn) {
        PlayerObject::pushButton(btn);
        if (btn != PlayerButton::Jump) return; // fokus tombol jump dulu

        if (g_manualTab.getState() == ManualTab::State::Recording && g_currentPlayLayer) {
            int frame = (int)g_currentPlayLayer->m_gameState.m_currentProgress; // sesuaikan API progress asli
            g_manualTab.onFrame(frame, true);
        }
    }

    void releaseButton(PlayerButton btn) {
        PlayerObject::releaseButton(btn);
        if (btn != PlayerButton::Jump) return;

        if (g_manualTab.getState() == ManualTab::State::Recording && g_currentPlayLayer) {
            int frame = (int)g_currentPlayLayer->m_gameState.m_currentProgress;
            g_manualTab.onFrame(frame, false);
        }
    }
};

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
        return true;
    }

    void update(float dt) {
        PlayLayer::update(dt);

        int frame = m_gameState.m_currentProgress; // contoh, nanti disesuaikan API asli

        // (Capture record udah di-handle di hook BPPlayerObject::pushButton/releaseButton,
        // bukan di sini lagi -- lebih akurat karena nangkep momen tekan/lepas asli)

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

// Hook ke PauseLayer: nambahin tombol BOTPLAYER di menu pause
class $modify(BPPauseLayer, PauseLayer) {
    bool init(bool useV2) {
        if (!PauseLayer::init(useV2)) return false;

        auto menu = this->getChildByID("right-button-menu");
        if (!menu) {
            // fallback kalau ID beda di versi GD lain
            menu = this->getChildByType<CCMenu>(0);
        }

        if (menu) {
            auto btnSpr = ButtonSprite::create("BOT");
            btnSpr->setScale(0.6f);
            auto btn = CCMenuItemSpriteExtra::create(
                btnSpr, this, menu_selector(BPPauseLayer::onOpenBotPlayer)
            );
            menu->addChild(btn);
            menu->updateLayout();
        }

        return true;
    }

    void onOpenBotPlayer(CCObject*) {
        BotPlayerPopup::create()->show();
    }
};

$execute {
    log::info("BOTPLAYER loaded!");
}
