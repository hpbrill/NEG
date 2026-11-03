#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace neg {

struct DifficultyConfig {
    std::size_t goodsCount;
    std::size_t positions;
    std::size_t maxAttempts;
};

DifficultyConfig difficultyConfig(int difficulty);

enum class Status { Running, Won, Lost };

struct AttemptResult {
    bool accepted;
    std::string feedback;
    std::string error;
};

class Game {
public:
    explicit Game(int difficulty);
    Game(int difficulty, std::uint32_t seed);
    // Explicit setup for deterministic evaluation and embedding; validates invariants.
    Game(int difficulty, std::string availableGoods, std::string secret);

    AttemptResult submit(const std::string& input);
    const DifficultyConfig& config() const { return config_; }
    const std::string& availableGoods() const { return availableGoods_; }
    const std::string& secret() const { return secret_; }
    std::size_t attemptsUsed() const { return attemptsUsed_; }
    std::size_t attemptsRemaining() const { return config_.maxAttempts - attemptsUsed_; }
    Status status() const { return status_; }

private:
    DifficultyConfig config_;
    std::string availableGoods_;
    std::string secret_;
    std::size_t attemptsUsed_ = 0;
    Status status_ = Status::Running;
};

} // namespace neg
