#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include "ManualTab.hpp"
#include "GeneticBot.hpp"

using namespace geode::prelude;

ManualTab g_manualTab;
GeneticBot g_geneticBot;

size_t g_playbackCursor = 0;
bool g_lastPlaybackHold = false;

PlayLayer* g_currentPlayLayer = nullptr;

class $modify(BPPlayerObject, PlayerObject) {
    void pushButton(PlayerButton btn) {
        PlayerObject::pushButton(btn);
        if (btn != PlayerButton::Jump) return;

        if (g_manualTab.getState() == ManualTab::State::Recording && g_currentPlayLayer) {
            int frame = (int)g_currentPlayLayer->m_gameState.m_currentProgress;
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

        int frame = m_gameState.m_currentProgress;

        if (g_manualTab.getState() == ManualTab::State::Playing) {
            const auto& macroFrames = g_manualTab.getMacro().frames;
            applyMacroInput(m_player1, macroFrames, frame);

            if (g_playbackCursor >= macroFrames.size()) {
                g_manualTab.stopPlayback();
                g_playbackCursor = 0;
            }
        }

        if (g_geneticBot.isRunning()) {
            const auto& macroFrames = g_geneticBot.nextIndividual().frames;
            applyMacroInput(m_player1, macroFrames, frame);
        }
    }

    void resetLevel() {
        if (g_geneticBot.isRunning()) {
            float progressPercent = m_gameState.m_currentProgress;
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
