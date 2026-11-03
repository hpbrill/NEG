#include "neg/game.hpp"

#include <exception>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    constexpr int wonExit = 0;
    constexpr int lostExit = 1;
    constexpr int errorExit = 2;
    if (argc != 2 || std::string(argv[1]).size() != 1 || argv[1][0] < '1' || argv[1][0] > '7') {
        std::cerr << "Error: Provide exactly one difficulty from 1 to 7.\n"
                  << "Usage: NegInit <difficulty 1-7>\n";
        return errorExit;
    }
    try {
        neg::Game game(argv[1][0] - '0');
        std::cout << "Available goods: " << game.availableGoods()
                  << "\nW=Wood F=Food P=Pole T=Timber B=Board S=Straw"
                  << "\nPositions: " << game.config().positions
                  << "\nMaximum attempts: " << game.config().maxAttempts << '\n';
        while (game.status() == neg::Status::Running) {
            std::cout << "Input: " << std::flush;
            std::string input;
            if (!std::getline(std::cin, input)) {
                std::cerr << "Error: Input stream ended before the game finished or could not be read.\n";
                return errorExit;
            }
            const auto result = game.submit(input);
            if (!result.accepted) {
                std::cerr << "Error: " << result.error << '\n';
                return errorExit;
            }
            std::cout << "Feedback: " << result.feedback
                      << " | Remaining attempts: " << game.attemptsRemaining() << '\n';
        }
        if (game.status() == neg::Status::Won) {
            std::cout << "You won!\n";
            return wonExit;
        }
        std::cout << "You lost! Secret: " << game.secret() << '\n';
        return lostExit;
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return errorExit;
    }
}
