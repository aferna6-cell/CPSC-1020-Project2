// CPSC 1020 Project 2 - Group 19
// Cafes on the Beach simulation.
//
// Company A (our cafes) and company B (the competitor) each place 1-10 cafes
// on a 10 km beach. Every trial, one customer appears at a random location
// (a float from 0 to 10 km) and buys one coffee from the cafe with the highest
// probability (score / sum of all scores, score = 28 - distance - 3 * price).
// Each coffee costs $2 to make, so a sale earns (price - $2).
// The program prints the average profit per customer for every cafe.

#include "Cafe.h"
#include "Customer.h"

#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {
    const int MIN_CAFES = 1;
    const int MAX_CAFES = 10;
    const int MAX_TRIALS = 10000000;

    // Thrown when input ends (Ctrl+D / end of file) so we never loop forever.
    class InputEnded : public std::runtime_error {
    public:
        InputEnded() : std::runtime_error("Input ended before all values were entered.") {}
    };

    // Print a prompt and read one whole line of input.
    std::string readLine(const std::string& prompt) {
        std::cout << prompt;
        std::string line;
        if (!std::getline(std::cin, line)) {
            throw InputEnded();
        }
        return line;
    }

    // Convert text to an int. Throws std::invalid_argument if it is not a
    // whole number (for example "abc", "3.5", or "4 cafes").
    int parseInt(const std::string& text) {
        std::istringstream in(text);
        int value = 0;
        char extra = '\0';
        if (!(in >> value) || (in >> extra)) {
            throw std::invalid_argument("\"" + text + "\" is not a whole number.");
        }
        return value;
    }

    // Convert text to a float. Throws std::invalid_argument if it is not a number.
    float parseFloat(const std::string& text) {
        std::istringstream in(text);
        float value = 0.0f;
        char extra = '\0';
        if (!(in >> value) || (in >> extra)) {
            throw std::invalid_argument("\"" + text + "\" is not a number.");
        }
        return value;
    }

    // Keep asking until the user enters a whole number in [low, high].
    // Bad values throw an exception, which is caught here and the user is
    // asked again.
    int askInt(const std::string& prompt, int low, int high) {
        while (true) {
            try {
                const int value = parseInt(readLine(prompt));
                if (value < low || value > high) {
                    throw std::out_of_range("Value must be between " + std::to_string(low) +
                                            " and " + std::to_string(high) + ".");
                }
                return value;
            } catch (const std::invalid_argument& error) {
                std::cout << "  Invalid input: " << error.what() << " Please try again.\n";
            } catch (const std::out_of_range& error) {
                std::cout << "  Invalid input: " << error.what() << " Please try again.\n";
            }
        }
    }

    // Keep asking until the user enters a number in [low, high].
    float askFloat(const std::string& prompt, float low, float high) {
        while (true) {
            try {
                const float value = parseFloat(readLine(prompt));
                if (!(value >= low && value <= high)) {
                    std::ostringstream message;
                    message << "Value must be between " << low << " and " << high << ".";
                    throw std::out_of_range(message.str());
                }
                return value;
            } catch (const std::invalid_argument& error) {
                std::cout << "  Invalid input: " << error.what() << " Please try again.\n";
            } catch (const std::out_of_range& error) {
                std::cout << "  Invalid input: " << error.what() << " Please try again.\n";
            }
        }
    }

    // Ask a yes/no question; returns true for y/yes.
    bool askYesNo(const std::string& prompt) {
        while (true) {
            const std::string answer = readLine(prompt);
            if (answer == "y" || answer == "Y" || answer == "yes" || answer == "Yes") {
                return true;
            }
            if (answer == "n" || answer == "N" || answer == "no" || answer == "No") {
                return false;
            }
            std::cout << "  Please enter y or n.\n";
        }
    }

    // Ask for one cafe's location and coffee price, then create it on the heap.
    Cafe* askForCafe(const std::string& name) {
        std::cout << "\nCafe " << name << ":\n";
        const float location =
            askFloat("  Location of " + name + " on the beach (0 to 10 km): ", 0.0f, 10.0f);
        const float price =
            askFloat("  Price of one coffee at " + name + " (dollars, e.g. 5 or 4.50): ",
                     0.0f, 100.0f);
        return new Cafe(price, location);
    }

    // Run the simulation: each trial a new customer appears at a random float
    // location from 0 to 10 km and buys from the highest-probability cafe.
    void runSimulation(Cafe* cafes[], int cafeCount, int trials) {
        for (int i = 0; i < cafeCount; ++i) {
            cafes[i]->resetStatistics();
        }

        std::mt19937 generator(std::random_device{}());
        std::uniform_real_distribution<float> beach(0.0f, 10.0f);

        for (int trial = 0; trial < trials; ++trial) {
            Customer customer(beach(generator));
            customer.chooseCafe(cafes, cafeCount);
        }
    }

    // Print a labeled results table with 2 decimal places of precision.
    void printResults(Cafe* cafes[], const std::string names[], int countA,
                      int cafeCount, int trials) {
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "\n================== Simulation Results (" << trials
                  << " trials) ==================\n";
        std::cout << std::left << std::setw(6) << "Cafe" << std::right
                  << std::setw(15) << "Location (km)" << std::setw(12) << "Price ($)"
                  << std::setw(12) << "Customers" << std::setw(12) << "Share (%)"
                  << std::setw(22) << "Avg Profit ($/cust)" << '\n';
        std::cout << std::string(79, '-') << '\n';

        double companyA = 0.0;
        double companyB = 0.0;
        for (int i = 0; i < cafeCount; ++i) {
            const double averageProfit = cafes[i]->getTotalProfit() / trials;
            const double share = 100.0 * cafes[i]->getCustomerCount() / trials;
            std::cout << std::left << std::setw(6) << names[i] << std::right
                      << std::setw(15) << cafes[i]->getLocation()
                      << std::setw(12) << cafes[i]->getCoffeeCost()
                      << std::setw(12) << cafes[i]->getCustomerCount()
                      << std::setw(12) << share
                      << std::setw(22) << averageProfit << '\n';
            if (i < countA) {
                companyA += averageProfit;
            } else {
                companyB += averageProfit;
            }
            if (i == countA - 1) {
                std::cout << std::string(79, '-') << '\n';
            }
        }
        std::cout << std::string(79, '-') << '\n';
        std::cout << "Company A (your cafes)       average profit per customer: $"
                  << companyA << '\n';
        std::cout << "Company B (competitor cafes) average profit per customer: $"
                  << companyB << '\n';
        std::cout << "(Average profit = total profit / number of trials.)\n";
    }
}

int main() {
    std::cout << "=============================================\n"
              << "  Cafes on the Beach Simulation - Group 19\n"
              << "=============================================\n"
              << "The beach runs from 0 km to 10 km. Company A is your company and\n"
              << "company B is the competitor. Each company may have 1 to 10 cafes.\n";

    try {
        do {
            Cafe* cafes[2 * MAX_CAFES] = {nullptr};
            std::string names[2 * MAX_CAFES];

            const int countA =
                askInt("\nHow many cafes does company A (your company) have? (1-10): ",
                       MIN_CAFES, MAX_CAFES);
            const int countB =
                askInt("How many cafes does company B (the competitor) have? (1-10): ",
                       MIN_CAFES, MAX_CAFES);
            const int cafeCount = countA + countB;

            try {
                for (int i = 0; i < countA; ++i) {
                    names[i] = "A" + std::to_string(i + 1);
                    cafes[i] = askForCafe(names[i]);
                }
                for (int i = 0; i < countB; ++i) {
                    names[countA + i] = "B" + std::to_string(i + 1);
                    cafes[countA + i] = askForCafe(names[countA + i]);
                }

                const int trials = askInt("\nHow many trials (customers) should be simulated? "
                                          "(1-10000000, e.g. 50000): ",
                                          1, MAX_TRIALS);

                std::cout << "\nRunning " << trials << " trials...\n";
                runSimulation(cafes, cafeCount, trials);
                printResults(cafes, names, countA, cafeCount, trials);
            } catch (...) {
                for (int i = 0; i < cafeCount; ++i) {
                    delete cafes[i];
                }
                throw;
            }

            for (int i = 0; i < cafeCount; ++i) {
                delete cafes[i];
                cafes[i] = nullptr;
            }
        } while (askYesNo("\nRun another simulation? (y/n): "));
    } catch (const InputEnded& error) {
        std::cout << "\n" << error.what() << " Exiting.\n";
        return 1;
    }

    std::cout << "Goodbye!\n";
    return 0;
}
