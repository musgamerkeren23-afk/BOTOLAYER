#pragma once
#include "MacroData.hpp"
#include <vector>

// Satu individu = satu percobaan macro + skor seberapa jauh dia jalan
struct Individual {
    MacroData macro;
    float fitness = 0.f; // posisi X terjauh yang dicapai sebelum mati
};

class GeneticBot {
public:
    void startNewLevel(int totalPopulation = 30);
    void stop();

    void reportResult(float progress);
    const MacroData& nextIndividual();
    bool generationDone() const;
    void evolve();

    const Individual& getBestSoFar() const { return m_best; }
    bool isRunning() const { return m_running; }

private:
    std::vector<Individual> m_population;
    Individual m_best;
    int m_currentIndex = 0;
    int m_generation = 0;
    bool m_running = false;

    float m_mutationRate = 0.05f;

    MacroData mutate(const MacroData& parent);
    MacroData crossover(const MacroData& a, const MacroData& b);
};
