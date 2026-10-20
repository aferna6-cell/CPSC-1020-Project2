#include "Customer.h"
#include "Cafe.h"

#include <cmath>
#include <stdexcept>

Customer::Customer(float location) : location(location), visitedCafe(nullptr) {
    if (!std::isfinite(location) || location < 0.0f || location > 10.0f) {
        throw std::out_of_range("Customer location must be between 0 and 10 km.");
    }
}

Customer::~Customer() {
    // The cafes are owned by the program, not by individual customers.
    // Never delete visitedCafe here.
}

float Customer::getLocation() const {
    return location;
}

Cafe* Customer::getVisitedCafe() const {
    return visitedCafe;
}

float Customer::calculateScore(const Cafe& cafe) const {
    const float distance = std::fabs(location - cafe.getLocation());
    return 28.0f - distance - 3.0f * cafe.getCoffeeCost();
}

void Customer::chooseCafe(Cafe* cafes[], int cafeCount) {
    if (cafes == nullptr || cafeCount <= 0) {
        throw std::invalid_argument("At least one cafe is required.");
    }

    // Reject invalid arrays before recording a sale to avoid partial updates.
    for (int i = 0; i < cafeCount; ++i) {
        if (cafes[i] == nullptr) {
            throw std::invalid_argument("The cafe list contains a null pointer.");
        }
    }

    Cafe* bestCafe = cafes[0];
    float bestScore = calculateScore(*bestCafe);

    for (int i = 1; i < cafeCount; ++i) {
        const float score = calculateScore(*cafes[i]);
        if (score > bestScore) {
            bestScore = score;
            bestCafe = cafes[i];
        }
    }

    // Slide 8 specifies choosing the cafe with the highest probability.
    // With a common positive total score, this is the highest-scoring cafe.
    // Ties are resolved in favor of the earliest cafe in the array.
    visitedCafe = bestCafe;
    visitedCafe->recordSale();
}
