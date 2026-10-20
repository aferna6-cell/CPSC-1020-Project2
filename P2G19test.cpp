// CPSC 1020 Project 2 - Group 19
// Unit tests and a 50,000-customer simulation check (no linked list).

#include "Cafe.h"
#include "Customer.h"

#include <cmath>
#include <iostream>
#include <limits>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {
    int passed = 0;
    int failed = 0;

    void check(bool condition, const std::string& description) {
        if (condition) {
            std::cout << "PASS: " << description << '\n';
            ++passed;
        } else {
            std::cerr << "FAIL: " << description << '\n';
            ++failed;
        }
    }

    bool near(float actual, float expected, float tolerance = 0.001f) {
        return std::fabs(actual - expected) <= tolerance;
    }

    template <typename Exception, typename Function>
    bool throwsAsExpected(Function action) {
        try {
            action();
        } catch (const Exception&) {
            return true;
        } catch (...) {
            return false;
        }
        return false;
    }
}

int main() {
    // 1. Constructors initialize fields and the cafe pointer.
    Cafe cafeA(5.0f, 3.0f);
    Customer customer(1.0f);
    check(near(cafeA.getLocation(), 3.0f) &&
          near(cafeA.getCoffeeCost(), 5.0f) &&
          near(cafeA.getTotalProfit(), 0.0f) &&
          cafeA.getCustomerCount() == 0,
          "Cafe constructor and getters");
    check(near(customer.getLocation(), 1.0f) && customer.getVisitedCafe() == nullptr,
          "Customer constructor initializes its pointer to nullptr");

    // 2. Setters and limits on the 10-km beach.
    cafeA.setLocation(4.0f);
    cafeA.setCoffeeCost(4.50f);
    check(near(cafeA.getLocation(), 4.0f) && near(cafeA.getCoffeeCost(), 4.50f),
          "Cafe setters update location and price");
    check(throwsAsExpected<std::out_of_range>([]() { Cafe wrong(5.0f, 11.0f); }),
          "Reject cafe outside 0-10 km");
    check(throwsAsExpected<std::out_of_range>([]() { Customer wrong(-0.1f); }),
          "Reject customer outside 0-10 km");
    check(throwsAsExpected<std::invalid_argument>([]() { Cafe wrong(-1.0f, 5.0f); }),
          "Reject a negative coffee price");
    check(throwsAsExpected<std::out_of_range>([&cafeA]() { cafeA.setLocation(12.0f); }) &&
          near(cafeA.getLocation(), 4.0f),
          "Invalid setter throws without modifying location");

    // 3. Scores depend on absolute distance and price.
    Cafe farCafe(5.0f, 8.0f);
    check(near(customer.calculateScore(farCafe), 6.0f),
          "Score is 28 - |1 - 8| - 3*5 = 6");
    check(near(customer.calculateScore(cafeA), 11.5f),
          "Score uses 4.50 price and absolute distance");

    // 4. The customer chooses the cafe with the greatest score.
    Cafe* choices[] = {&farCafe, &cafeA};
    customer.chooseCafe(choices, 2);
    check(customer.getVisitedCafe() == &cafeA &&
          cafeA.getCustomerCount() == 1 &&
          farCafe.getCustomerCount() == 0,
          "Choose higher-scoring cafe and update its customer count");
    check(near(cafeA.getTotalProfit(), 2.5f),
          "Coffee sale earns price minus $2 production cost");

    // 5. Tie behavior is deterministic (first cafe in array wins).
    Cafe tieA(5.0f, 5.0f);
    Cafe tieB(5.0f, 5.0f);
    Cafe* tied[] = {&tieB, &tieA};
    Customer tiedCustomer(5.0f);
    tiedCustomer.chooseCafe(tied, 2);
    check(tiedCustomer.getVisitedCafe() == &tieB && tieA.getCustomerCount() == 0,
          "Tie is resolved by original cafe array order");

    // 6. A missing cafe is a reported error, not a crash.
    check(throwsAsExpected<std::invalid_argument>([&customer]() {
              customer.chooseCafe(nullptr, 0);
          }),
          "Reject an empty cafe array");
    Cafe* invalid[] = {&cafeA, nullptr};
    const int oldCount = cafeA.getCustomerCount();
    check(throwsAsExpected<std::invalid_argument>([&customer, &invalid]() {
              customer.chooseCafe(invalid, 2);
          }) && cafeA.getCustomerCount() == oldCount,
          "Reject a null cafe element without recording a sale");

    // 7. Test print output and resetStatistics().
    std::ostringstream printed;
    std::streambuf* oldBuffer = std::cout.rdbuf(printed.rdbuf());
    cafeA.printSummary();
    std::cout.rdbuf(oldBuffer);
    check(printed.str().find("1 customers") != std::string::npos &&
          printed.str().find("$2.50") != std::string::npos,
          "Summary prints customers and two-decimal profit");
    cafeA.resetStatistics();
    check(cafeA.getCustomerCount() == 0 && near(cafeA.getTotalProfit(), 0.0f),
          "Reset clears profit and customer count");

    // 8. A reproducible larger simulation: one customer per trial.
    Cafe a1(5.0f, 3.0f);
    Cafe a2(5.0f, 7.0f);
    Cafe b1(5.0f, 3.5f);
    Cafe b2(5.0f, 6.5f);
    Cafe* fourCafes[] = {&a1, &a2, &b1, &b2};
    std::mt19937 generator(19);
    std::uniform_real_distribution<float> positions(0.0f, 10.0f);
    const int trialCount = 50000;
    for (int trial = 0; trial < trialCount; ++trial) {
        Customer arrivingCustomer(positions(generator));
        arrivingCustomer.chooseCafe(fourCafes, 4);
    }
    const int totalCustomers = a1.getCustomerCount() + a2.getCustomerCount() +
                               b1.getCustomerCount() + b2.getCustomerCount();
    const float totalProfit = a1.getTotalProfit() + a2.getTotalProfit() +
                              b1.getTotalProfit() + b2.getTotalProfit();
    check(totalCustomers == trialCount, "Every one of 50,000 customers visits exactly one cafe");
    check(near(totalProfit, trialCount * 3.0f),
          "Profit across all cafes equals 50,000 * ($5 - $2)");

    std::cout << "\nGroup 19 test results: " << passed << " passed, " << failed
              << " failed.\n";
    return (failed == 0) ? 0 : 1;
}
