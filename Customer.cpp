#include "Customer.h"

#include <cmath>
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
}

Customer::Customer() : location(0.0f), visitedCafe(nullptr) {
    // Execution has entered the Customer default constructor.
}

Customer::Customer(float location) : location(location), visitedCafe(nullptr) {
    // Execution has entered the Customer parameterized constructor.
    if (!std::isfinite(location) || location < 0.0f || location > 10.0f) {
        throw std::out_of_range("Customer location must be between 0 and 10 km.");
    }
}

Customer::Customer(const Customer& other)
    : location(other.location), visitedCafe(other.visitedCafe) {
    // Execution has entered the Customer copy constructor.
    // visitedCafe is copied as a non-owning pointer; the cafe is not cloned.
}

Customer& Customer::operator=(const Customer& other) {
    // Execution has entered the Customer copy assignment operator.
    // Copy the address of the cafe. Never delete the previous cafe.
    if (this != &other) {
        location = other.location;
        visitedCafe = other.visitedCafe;
    }
    return *this;
}

Customer::~Customer() {
    // Execution has entered the Customer destructor.
    // The cafes are owned by the program, not by individual customers.
    // Never delete visitedCafe here.
    visitedCafe = nullptr;
}

float Customer::getLocation() const {
    return location;
}

Cafe* Customer::getVisitedCafe() const {
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

void Customer::chooseCafe(Cafe* cafes[], int cafeCount) {
    if (cafes == nullptr || cafeCount <= 0) {
        throw std::invalid_argument("At least one cafe is required.");
    }

    // Reject the whole list before recording a sale, including later nulls.
    for (int i = 0; i < cafeCount; ++i) {
        if (cafes[i] == nullptr) {
            throw std::invalid_argument("The cafe list contains a null pointer.");
        }
    }

    // This customer already selected a cafe and recorded one sale.
    if (visitedCafe != nullptr) {
        return;
    }

    float totalScore = 0.0f;
    for (int i = 0; i < cafeCount; ++i) {
        totalScore += calculateScore(*cafes[i]);
    }

    // An unusable denominator means no probability is defined.
    // Leave the customer unassigned and do not record a sale.
    if (!std::isfinite(totalScore) || totalScore == 0.0f) {
        visitedCafe = nullptr;
        return;
    }

    // Compare probabilities, not raw scores. Every cafe is considered.
    // An equal probability keeps the earlier cafe because the test is strict >.
    int bestIndex = 0;
    float bestProbability = probabilityFromTotal(calculateScore(*cafes[0]), totalScore);

    for (int i = 1; i < cafeCount; ++i) {
        const float probability =
            probabilityFromTotal(calculateScore(*cafes[i]), totalScore);
        if (probability > bestProbability) {
            bestProbability = probability;
            bestIndex = i;
        }
    }

    visitedCafe = cafes[bestIndex];
    visitedCafe->recordSale();
}
