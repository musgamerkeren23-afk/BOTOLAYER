#pragma once
#include "MacroData.hpp"

// Tab Manual: rekam input pemain, save, lalu playback ulang
class ManualTab {
public:
    enum class State { Idle, Recording, Playing };

    void startRecording();
    void stopRecording();      // otomatis save ke m_macro
    void startPlayback();      // cek dulu m_macro.empty() -> tampilkan "macro ini kosong!"
    void stopPlayback();

    void onFrame(int frameIndex, bool holding); // dipanggil tiap update game

    const MacroData& getMacro() const { return m_macro; }
    State getState() const { return m_state; }

private:
    MacroData m_macro;
    State m_state = State::Idle;
};
