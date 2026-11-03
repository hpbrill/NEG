#include "neg/game.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <random>
#include <stdexcept>
#include <utility>

namespace neg {
namespace {
constexpr char allGoods[] = "WFPTBS";
constexpr std::array<DifficultyConfig, 7> configurations{{
    {3, 3, 3}, {3, 4, 3}, {4, 4, 4}, {4, 5, 4},
    {5, 5, 5}, {5, 6, 5}, {6, 6, 6}
}};
}

DifficultyConfig difficultyConfig(int difficulty) {
    if (difficulty < 1 || difficulty > 7) {
        throw std::invalid_argument("Difficulty must be between 1 and 7.");
    }
    return configurations[static_cast<std::size_t>(difficulty - 1)];
}

Game::Game(int difficulty) : Game(difficulty, std::random_device{}()) {}

Game::Game(int difficulty, std::uint32_t seed) : config_(difficultyConfig(difficulty)) {
    std::mt19937 generator(seed);
    availableGoods_ = allGoods;
    std::shuffle(availableGoods_.begin(), availableGoods_.end(), generator);
    availableGoods_.resize(config_.goodsCount);
    std::uniform_int_distribution<std::size_t> distribution(0, config_.goodsCount - 1);
    for (std::size_t i = 0; i < config_.positions; ++i) {
        secret_ += availableGoods_[distribution(generator)];
    }
}

Game::Game(int difficulty, std::string availableGoods, std::string secret)
    : config_(difficultyConfig(difficulty)), availableGoods_(std::move(availableGoods)),
      secret_(std::move(secret)) {
    if (availableGoods_.size() != config_.goodsCount || secret_.size() != config_.positions) {
        throw std::invalid_argument("Invalid length in the supplied game setup.");
    }
    std::string seen;
    for (char good : availableGoods_) {
        if (std::string(allGoods).find(good) == std::string::npos ||
            seen.find(good) != std::string::npos) {
            throw std::invalid_argument("Invalid or duplicate available good.");
        }
        seen += good;
    }
    if (secret_.find_first_not_of(availableGoods_) != std::string::npos) {
        throw std::invalid_argument("The secret contains unavailable goods.");
    }
}

AttemptResult Game::submit(const std::string& input) {
    if (status_ != Status::Running) {
        return {false, {}, "The game has already ended."};
    }
    const auto whitespace = [](unsigned char c) { return std::isspace(c) != 0; };
    const auto first = std::find_if_not(input.begin(), input.end(), whitespace);
    const auto last = std::find_if_not(input.rbegin(), input.rend(), whitespace).base();
    std::string normalized;
    if (first < last) {
        normalized.assign(first, last);
    }
    if (normalized.size() != config_.positions) {
        return {false, {}, "Input must contain exactly " + std::to_string(config_.positions) +
            " goods letters."};
    }
    for (char& good : normalized) {
        good = static_cast<char>(std::toupper(static_cast<unsigned char>(good)));
        if (availableGoods_.find(good) == std::string::npos) {
            return {false, {}, "Invalid character: Use only available goods without internal whitespace."};
        }
    }
    std::string feedback(config_.positions, '-');
    for (std::size_t i = 0; i < config_.positions; ++i) {
        if (normalized[i] == secret_[i]) {
            feedback[i] = 'X';
        }
    }
    ++attemptsUsed_;
    if (normalized == secret_) {
        status_ = Status::Won;
    } else if (attemptsRemaining() == 0) {
        status_ = Status::Lost;
    }
    return {true, feedback, {}};
}

} // namespace neg
