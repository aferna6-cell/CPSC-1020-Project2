#include "Customer.h"

#include <cmath>
#include <random>
#include <stdexcept>

namespace {
    // probability = score / totalScore.
    // A zero or non-finite denominator, or a non-finite score, means the
    // probability is undefined. Return 0 so callers do not divide by zero.
    float probabilityFromTotal(float score, float totalScore) {
        if (!std::isfinite(score) || !std::isfinite(totalScore) || totalScore == 0.0f) {
            return 0.0f;
        }
        return score / totalScore;
    }

    // Random engine used only to break exact ties between cafes, so that two
    // identical cafes split the customers instead of the first one taking all.
    std::mt19937& tieBreaker() {
        static std::mt19937 engine(std::random_device{}());
        return engine;
    }

    // Return a new heap copy of a cafe, or nullptr when there is nothing to copy.
    Cafe* copyOf(const Cafe* cafe) {
        return (cafe == nullptr) ? nullptr : new Cafe(*cafe);
    }
}

Customer::Customer() : location(0.0f), visitedCafe(nullptr) {
}

Customer::Customer(float location) : location(location), visitedCafe(nullptr) {
    if (!std::isfinite(location) || location < 0.0f || location > 10.0f) {
        throw std::out_of_range("Customer location must be between 0 and 10 km.");
    }
}

// Deep copy: create a brand-new Cafe and copy the values it points to.
Customer::Customer(const Customer& other)
    : location(other.location), visitedCafe(copyOf(other.visitedCafe)) {
}

// Deep copy assignment. The new copy is made BEFORE the old Cafe is deleted,
// so self-assignment and a failed allocation both leave this object valid.
Customer& Customer::operator=(const Customer& other) {
    if (this != &other) {
        Cafe* newCafe = copyOf(other.visitedCafe);
        delete visitedCafe;
        visitedCafe = newCafe;
        location = other.location;
    }
    return *this;
}

// The customer owns visitedCafe, so the destructor releases it.
Customer::~Customer() {
    delete visitedCafe;
    visitedCafe = nullptr;
}

float Customer::getLocation() const {
    return location;
}

const Cafe* Customer::getVisitedCafe() const {
    return visitedCafe;
}

float Customer::calculateScore(const Cafe& cafe) const {
    // fabs keeps the distance nonnegative when the customer is on either side.
    const float distance = std::fabs(location - cafe.getLocation());
    return 28.0f - distance - 3.0f * cafe.getCoffeeCost();
}

float Customer::calculateProbability(Cafe* cafes[], int cafeCount,
                                     int cafeIndex) const {
    if (cafes == nullptr || cafeCount <= 0 || cafeIndex < 0 || cafeIndex >= cafeCount ||
        cafes[cafeIndex] == nullptr) {
        return 0.0f;
    }

    float totalScore = 0.0f;
    for (int i = 0; i < cafeCount; ++i) {
        if (cafes[i] == nullptr) {
            return 0.0f;
        }
        totalScore += calculateScore(*cafes[i]);
    }

    return probabilityFromTotal(calculateScore(*cafes[cafeIndex]), totalScore);
}

int Customer::chooseCafe(Cafe* cafes[], int cafeCount) {
    if (cafes == nullptr || cafeCount <= 0) {
        throw std::invalid_argument("At least one cafe is required.");
    }
    // Reject the whole list before recording a sale.
    for (int i = 0; i < cafeCount; ++i) {
        if (cafes[i] == nullptr) {
            throw std::invalid_argument("The cafe list contains a null pointer.");
        }
    }

    float totalScore = 0.0f;
    for (int i = 0; i < cafeCount; ++i) {
        totalScore += calculateScore(*cafes[i]);
    }
    // An unusable denominator means no probability is defined: no sale.
    if (!std::isfinite(totalScore) || totalScore == 0.0f) {
        return -1;
    }

    // Find the highest probability and how many cafes share it.
    float bestProbability = probabilityFromTotal(calculateScore(*cafes[0]), totalScore);
    int tiedCount = 1;
    for (int i = 1; i < cafeCount; ++i) {
        const float probability =
            probabilityFromTotal(calculateScore(*cafes[i]), totalScore);
        if (probability > bestProbability) {
            bestProbability = probability;
            tiedCount = 1;
        } else if (probability == bestProbability) {
            ++tiedCount;
        }
    }

    // Pick one of the tied cafes at random (no randomness when there is no tie).
    int pick = 0;
    if (tiedCount > 1) {
        std::uniform_int_distribution<int> choice(0, tiedCount - 1);
        pick = choice(tieBreaker());
    }

    int chosenIndex = -1;
    for (int i = 0; i < cafeCount; ++i) {
        if (probabilityFromTotal(calculateScore(*cafes[i]), totalScore) == bestProbability) {
            if (pick == 0) {
                chosenIndex = i;
                break;
            }
            --pick;
        }
    }

    // Record the sale on the real cafe, then keep our own copy of it.
    cafes[chosenIndex]->recordSale();
    Cafe* newCafe = copyOf(cafes[chosenIndex]);
    delete visitedCafe;
    visitedCafe = newCafe;
    return chosenIndex;
}
