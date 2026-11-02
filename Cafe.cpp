#include "Cafe.h"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <stdexcept>

namespace {
    // Making one coffee costs $2. Profit on a sale is coffeeCost - PRODUCTION_COST.
    const float PRODUCTION_COST = 2.0f;

    // All locations are measured in kilometers along the 0-10 km beach.
    void validateLocation(float value) {
        if (!std::isfinite(value) || value < 0.0f || value > 10.0f) {
            throw std::out_of_range("Cafe location must be between 0 and 10 km.");
        }
    }

    void validateCoffeeCost(float value) {
        if (!std::isfinite(value) || value < 0.0f) {
            throw std::invalid_argument("Coffee price must be a non-negative number.");
        }
    }
}

Cafe::Cafe()
    : location(0.0f), coffeeCost(0.0f), totalProfit(0.0f), customerCount(0) {
    // Execution has entered the Cafe default constructor.
}

Cafe::Cafe(float coffeeCost, float location)
    : location(location), coffeeCost(coffeeCost), totalProfit(0.0f),
      customerCount(0) {
    // Execution has entered the Cafe parameterized constructor.
    validateCoffeeCost(coffeeCost);
    validateLocation(location);
}

float Cafe::getLocation() const {
    return location;
}

float Cafe::getCoffeeCost() const {
    return coffeeCost;
}

float Cafe::getTotalProfit() const {
    return totalProfit;
}

int Cafe::getCustomerCount() const {
    return customerCount;
}

void Cafe::setLocation(float newLocation) {
    validateLocation(newLocation);
    location = newLocation;
}

void Cafe::setCoffeeCost(float newCoffeeCost) {
    validateCoffeeCost(newCoffeeCost);
    coffeeCost = newCoffeeCost;
}

void Cafe::recordSale() {
    // Making one coffee costs $2; profit can be negative below that price.
    totalProfit += coffeeCost - PRODUCTION_COST;
    ++customerCount;
}

void Cafe::resetStatistics() {
    totalProfit = 0.0f;
    customerCount = 0;
}

void Cafe::printSummary() const {
    // Save stream settings to avoid changing formatting in the caller.
    const std::ios::fmtflags oldFlags = std::cout.flags();
    const std::streamsize oldPrecision = std::cout.precision();

    std::cout << "Cafe at " << location << " km: "
              << customerCount << " customers, total profit $"
              << std::fixed << std::setprecision(2) << totalProfit << '\n';

    std::cout.flags(oldFlags);
    std::cout.precision(oldPrecision);
}
