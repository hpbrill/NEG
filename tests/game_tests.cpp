#include "neg/game.hpp"

#include <array>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {
void check(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
template<class Function>
void expectInvalid(Function function) {
    try { function(); } catch (const std::invalid_argument&) { return; }
    throw std::runtime_error("Expected invalid_argument");
}

void configurationsAndRandomness() {
    const std::array<neg::DifficultyConfig, 7> expected{{
        {3,3,3}, {3,4,3}, {4,4,4}, {4,5,4}, {5,5,5}, {5,6,5}, {6,6,6}
    }};
    for (int difficulty = 1; difficulty <= 7; ++difficulty) {
        const auto config = neg::difficultyConfig(difficulty);
        const auto reference = expected[static_cast<std::size_t>(difficulty - 1)];
        check(config.goodsCount == reference.goodsCount && config.positions == reference.positions &&
              config.maxAttempts == reference.maxAttempts, "Difficulty configuration");
        for (unsigned seed = 0; seed < 100; ++seed) {
            const neg::Game game(difficulty, seed), repeat(difficulty, seed);
            check(game.availableGoods().size() == config.goodsCount, "Goods count");
            std::string seen;
            for (char good : game.availableGoods()) {
                check(std::string("WFPTBS").find(good) != std::string::npos, "Known good");
                check(seen.find(good) == std::string::npos, "Distinct goods");
                seen += good;
            }
            check(game.secret().size() == config.positions, "Secret length");
            check(game.secret().find_first_not_of(seen) == std::string::npos, "Secret goods");
            check(game.secret() == repeat.secret() && game.availableGoods() == repeat.availableGoods(),
                  "Seed reproducibility");
            check(game.status() == neg::Status::Running && game.attemptsUsed() == 0 &&
                  game.attemptsRemaining() == config.maxAttempts, "Initial state");
        }
    }
    expectInvalid([] { neg::difficultyConfig(0); });
    expectInvalid([] { neg::difficultyConfig(8); });
    expectInvalid([] { neg::Game game(1, "WWF", "WFF"); });
    expectInvalid([] { neg::Game game(1, "WFZ", "WFF"); });
    expectInvalid([] { neg::Game game(1, "WF", "WFF"); });
    expectInvalid([] { neg::Game game(1, "WFS", "WF"); });
    expectInvalid([] { neg::Game game(1, "WFS", "WFP"); });
}

void feedbackAndEndings() {
    neg::Game game(1, "WFS", "WFF");
    check(game.submit("SFW").feedback == "-X-", "Position feedback with repeats");
    check(game.submit("WFS").feedback == "XX-", "Second feedback");
    check(game.submit("WFF").feedback == "XXX", "Winning feedback");
    check(game.status() == neg::Status::Won && game.attemptsRemaining() == 0,
          "Win on final attempt");
    check(!game.submit("WFF").accepted && game.attemptsUsed() == 3 &&
          game.status() == neg::Status::Won, "Reject after win");

    neg::Game loss(1, "WFS", "WFF");
    check(loss.submit("WFS").feedback == "XX-", "Initial matches");
    check(loss.submit("SSS").feedback == "---", "Matches are not locked");
    check(loss.submit("FFF").feedback == "-XX", "No displaced good feedback");
    check(loss.status() == neg::Status::Lost && loss.attemptsRemaining() == 0, "Loss");
    check(!loss.submit("WFF").accepted && loss.attemptsUsed() == 3 &&
          loss.status() == neg::Status::Lost, "Reject after loss");

    neg::Game early(1, "WFS", "WWW");
    check(early.submit("www").feedback == "XXX" && early.status() == neg::Status::Won &&
          early.attemptsRemaining() == 2, "Early win with repeated goods");
}

void validation() {
    neg::Game game(1, "WFS", "WFF");
    game.submit("SSS");
    for (const std::string input : {"", "  \t", "WF", "WFFF", "W F", "WFP", "W?F", "W\tF"}) {
        const auto result = game.submit(input);
        check(!result.accepted && !result.error.empty() && result.feedback.empty(), "Invalid input");
        check(game.attemptsUsed() == 1 && game.attemptsRemaining() == 2 &&
              game.status() == neg::Status::Running && game.secret() == "WFF" &&
              game.availableGoods() == "WFS", "Invalid input preserves state");
    }
    check(game.submit(" \twfF\r\n").feedback == "XXX", "Trim and case normalization");
}
}

int main() {
    try {
        configurationsAndRandomness();
        feedbackAndEndings();
        validation();
        std::cout << "All core tests passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
