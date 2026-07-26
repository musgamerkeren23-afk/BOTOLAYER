#pragma once
#include <vector>
#include <string>
#include <cstdint>

// Satu frame input yang direkam / dihasilkan bot
struct InputFrame {
    int frame;       // frame ke berapa di level
    bool holding;    // true = jari nempel/klik ditahan, false = lepas
    int player = 1;  // 1 atau 2 (buat level dual)
};

// Satu macro = kumpulan input dari awal sampai akhir attempt
struct MacroData {
    std::vector<InputFrame> frames;
    std::string levelName;
    float bestProgress = 0.f; // dipakai di mode Automatic sbg fitness

    bool empty() const {
        return frames.empty();
    }

    void clear() {
        frames.clear();
        bestProgress = 0.f;
    }
};
