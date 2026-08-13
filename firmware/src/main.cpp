#include <iostream>
#include <string>

#include "game/GameManager.h"
#include "dispenser/MockDispenser.h"

int main() {
    MockDispenser dispenser;
    GameManager game(dispenser);

    while (true) {

        // =====================
        // HOME
        // =====================

        std::cout << "\n=========================\n";
        std::cout << "      CANDY QUEST\n";
        std::cout << "=========================\n\n";

        std::cout << "[ENTER] Start\n";
        std::cin.get();

        game.startGame();

        Quiz quiz = game.getCurrentQuiz();

        bool gameCancelled = false;
        bool correct = false;

        // =====================
        // QUIZ
        // =====================

        while (!correct && !gameCancelled) {

            std::cout << "\nQuestion:\n";
            std::cout << quiz.question << "\n\n";

            std::string answer;

            std::cout << "Your answer: ";
            std::getline(std::cin, answer);

            correct = game.submitAnswer(answer);

            // =====================
            // WRONG
            // =====================

            if (!correct) {

                std::cout << "\nWRONG!\n\n";

                std::cout << "[1] Try again\n";
                std::cout << "[2] Cancel\n\n";

                std::cout << "Choose: ";

                std::string choice;
                std::getline(std::cin, choice);

                if (choice == "2") {
                    game.cancelGame();
                    gameCancelled = true;

                    std::cout << "\nGame cancelled.\n";
                }
            }
        }

        // Cancel → HOME
        if (gameCancelled) {
            continue;
        }

        // =====================
        // CORRECT
        // =====================

        std::cout << "\nCORRECT!\n";
        std::cout << "You earned a candy!\n\n";

        // =====================
        // CANDY SELECT
        // =====================

        bool dispensed = false;

        while (!dispensed) {

            std::cout << "Choose candy [1-6]: ";

            int candySlot;
            std::cin >> candySlot;

            if (std::cin.fail()) {

                std::cin.clear();
                std::cin.ignore(10000, '\n');

                std::cout
                    << "Please enter a number from 1 to 6.\n";

                continue;
            }

            std::cin.ignore(10000, '\n');

            dispensed =
                game.selectCandy(candySlot);

            if (!dispensed) {

                std::cout
                    << "Invalid candy slot. Choose 1-6.\n";
            }
        }

        // =====================
        // SUCCESS
        // =====================

        std::cout << "\nSUCCESS!\n";
        std::cout << "Enjoy your candy!\n";

        // while loop starts again
        // → HOME
    }

    return 0;
}