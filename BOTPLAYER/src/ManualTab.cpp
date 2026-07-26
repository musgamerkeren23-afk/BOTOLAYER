#include "ManualTab.hpp"
#include <Geode/Geode.hpp>

using namespace geode::prelude;

void ManualTab::startRecording() {
    m_macro.clear();
    m_state = State::Recording;
    log::info("BOTPLAYER: mulai record macro");
}

void ManualTab::stopRecording() {
    if (m_state != State::Recording) return;
    m_state = State::Idle;
    log::info("BOTPLAYER: record selesai, {} frame tersimpan", m_macro.frames.size());
}

void ManualTab::startPlayback() {
    if (m_macro.empty()) {
        log::warn("BOTPLAYER: playback dibatalkan, macro kosong");
        return;
    }
    m_state = State::Playing;
    log::info("BOTPLAYER: mulai playback ({} frame)", m_macro.frames.size());
}

void ManualTab::stopPlayback() {
    if (m_state != State::Playing) return;
    m_state = State::Idle;
    log::info("BOTPLAYER: playback dihentikan");
}

void ManualTab::onFrame(int frameIndex, bool holding) {
    switch (m_state) {
        case State::Recording: {
            if (m_macro.frames.empty() || m_macro.frames.back().holding != holding) {
                m_macro.frames.push_back(InputFrame{ frameIndex, holding });
            }
            break;
        }
        case State::Playing: {
            break;
        }
        case State::Idle:
        default:
            break;
    }
}
