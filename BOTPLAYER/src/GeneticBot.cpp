#include "GeneticBot.hpp"
#include <Geode/Geode.hpp>
#include <algorithm>
#include <random>

using namespace geode::prelude;

static std::mt19937 rng{ std::random_device{}() };

void GeneticBot::startNewLevel(int totalPopulation) {
    m_population.clear();
    m_generation = 0;
    m_currentIndex = 0;
    m_best = Individual{};
    m_running = true;

    std::uniform_int_distribution<int> holdChance(0, 100);
    std::uniform_int_distribution<int> gapDist(5, 40);

    for (int i = 0; i < totalPopulation; i++) {
        Individual ind;
        int frame = 0;
        bool holding = false;
        while (frame < 2000) {
            frame += gapDist(rng);
            holding = !holding;
            ind.macro.frames.push_back(InputFrame{ frame, holding });
        }
        m_population.push_back(ind);
    }

    log::info("BOTPLAYER: generasi 0 dimulai, {} individu", m_population.size());
}

void GeneticBot::stop() {
    m_running = false;
    log::info("BOTPLAYER: automatic play dihentikan di generasi {}", m_generation);
}

void GeneticBot::reportResult(float progress) {
    if (m_currentIndex >= (int)m_population.size()) return;

    m_population[m_currentIndex].fitness = progress;

    if (progress > m_best.fitness) {
        m_best = m_population[m_currentIndex];
        log::info("BOTPLAYER: best baru! progress = {:.1f}%", progress);
    }

    m_currentIndex++;
}

const MacroData& GeneticBot::nextIndividual() {
    if (m_currentIndex >= (int)m_population.size()) {
        m_currentIndex = (int)m_population.size() - 1;
    }
    return m_population[m_currentIndex].macro;
}

bool GeneticBot::generationDone() const {
    return m_currentIndex >= (int)m_population.size();
}

void GeneticBot::evolve() {
    std::sort(m_population.begin(), m_population.end(),
        [](const Individual& a, const Individual& b) {
            return a.fitness > b.fitness;
        });

    int parentCount = std::max(2, (int)m_population.size() / 5);
    std::vector<Individual> parents(m_population.begin(), m_population.begin() + parentCount);

    std::vector<Individual> newPopulation;
    int eliteCount = std::max(1, parentCount / 4);
    for (int i = 0; i < eliteCount; i++) {
        newPopulation.push_back(parents[i]);
    }

    std::uniform_int_distribution<int> parentPick(0, parentCount - 1);
    while ((int)newPopulation.size() < (int)m_population.size()) {
        const auto& parentA = parents[parentPick(rng)].macro;
        const auto& parentB = parents[parentPick(rng)].macro;

        Individual child;
        child.macro = mutate(crossover(parentA, parentB));
        newPopulation.push_back(child);
    }

    m_population = std::move(newPopulation);
    m_currentIndex = 0;
    m_generation++;

    log::info("BOTPLAYER: generasi {} dimulai, best sejauh ini {:.1f}%", m_generation, m_best.fitness);
}

MacroData GeneticBot::crossover(const MacroData& a, const MacroData& b) {
    MacroData child;
    child.levelName = a.levelName;

    size_t half = a.frames.size() / 2;
    for (size_t i = 0; i < half && i < a.frames.size(); i++) {
        child.frames.push_back(a.frames[i]);
    }
    for (size_t i = half; i < b.frames.size(); i++) {
        child.frames.push_back(b.frames[i]);
    }
    return child;
}

MacroData GeneticBot::mutate(const MacroData& parent) {
    MacroData child = parent;
    std::uniform_real_distribution<float> chance(0.f, 1.f);
    std::uniform_int_distribution<int> shift(-10, 10);

    for (auto& f : child.frames) {
        if (chance(rng) < m_mutationRate) {
            f.frame = std::max(0, f.frame + shift(rng));
        }
        if (chance(rng) < m_mutationRate) {
            f.holding = !f.holding;
        }
    }
    return child;
}
