// CPSC 1020 Project 2 - Group 19
// Ordinary and extreme tests for the November 5 milestone.
// Diagnostic lines are printed only when a check fails.
// A single pass/fail summary is printed at the end.

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
            ++passed;
        } else {
            std::cerr << "FAIL: " << description << '\n';
            ++failed;
        }
    }

    bool near(float actual, float expected, float tolerance = 0.0001f) {
        return std::fabs(actual - expected) <= tolerance;
    }

    void checkNear(float actual, float expected, const std::string& description) {
        if (near(actual, expected)) {
            ++passed;
            return;
        }
        std::cerr << "FAIL: " << description << " (actual " << actual
                  << ", expected " << expected << ")\n";
        ++failed;
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

    // Independent copy of the slide formula, used to cross-check Customer.
    float specScore(float customerLocation, const Cafe& cafe) {
        const float distance = std::fabs(customerLocation - cafe.getLocation());
        return 28.0f - distance - 3.0f * cafe.getCoffeeCost();
    }

    // Index of the highest probability, or -1 when the denominator is unusable.
    // Equal probabilities keep the earliest cafe, matching Customer::chooseCafe.
    int indexOfHighestProbability(float customerLocation, Cafe* cafes[], int cafeCount) {
        float total = 0.0f;
        for (int i = 0; i < cafeCount; ++i) {
            total += specScore(customerLocation, *cafes[i]);
        }
        if (!std::isfinite(total) || total == 0.0f) {
            return -1;
        }

        int best = 0;
        float bestProbability = specScore(customerLocation, *cafes[0]) / total;
        for (int i = 1; i < cafeCount; ++i) {
            const float probability = specScore(customerLocation, *cafes[i]) / total;
            if (probability > bestProbability) {
                bestProbability = probability;
                best = i;
            }
        }
        return best;
    }
}

int main() {
    // --- Constructors and safe pointer initialization ---
    {
        Cafe plain;
        check(near(plain.getLocation(), 0.0f) && near(plain.getCoffeeCost(), 0.0f) &&
                  near(plain.getTotalProfit(), 0.0f) && plain.getCustomerCount() == 0,
              "default Cafe is at 0 km, price $0, profit $0, and 0 customers");

        Cafe priced(5.0f, 3.0f);
        check(near(priced.getLocation(), 3.0f) && near(priced.getCoffeeCost(), 5.0f) &&
                  near(priced.getTotalProfit(), 0.0f) && priced.getCustomerCount() == 0,
              "parameterized Cafe stores price, location, and zero statistics");

        Customer unnamed;
        check(near(unnamed.getLocation(), 0.0f) && unnamed.getVisitedCafe() == nullptr,
              "default Customer is at 0 km and has not selected a cafe");

        Customer placed(1.0f);
        check(near(placed.getLocation(), 1.0f) && placed.getVisitedCafe() == nullptr,
              "parameterized Customer stores its location and a null cafe pointer");
    }

    // Boundaries are valid. Values outside the beach, and non-finite numbers, are not.
    {
        Cafe west(2.0f, 0.0f);
        Cafe east(2.0f, 10.0f);
        Customer atStart(0.0f);
        Customer atEnd(10.0f);
        check(near(west.getLocation(), 0.0f) && near(east.getLocation(), 10.0f) &&
                  near(atStart.getLocation(), 0.0f) && near(atEnd.getLocation(), 10.0f),
              "cafe and customer locations of 0 km and 10 km are accepted");

        check(throwsAsExpected<std::out_of_range>([]() { Cafe wrong(5.0f, 10.1f); }),
              "reject a cafe past 10 km");
        check(throwsAsExpected<std::out_of_range>([]() { Cafe wrong(5.0f, -0.1f); }),
              "reject a cafe before 0 km");
        check(throwsAsExpected<std::out_of_range>([]() { Customer wrong(10.1f); }),
              "reject a customer past 10 km");
        check(throwsAsExpected<std::out_of_range>([]() { Customer wrong(-0.1f); }),
              "reject a customer before 0 km");
        check(throwsAsExpected<std::invalid_argument>([]() { Cafe wrong(-1.0f, 5.0f); }),
              "reject a negative coffee price");

        const float nan = std::numeric_limits<float>::quiet_NaN();
        const float inf = std::numeric_limits<float>::infinity();
        check(throwsAsExpected<std::out_of_range>([nan]() { Cafe wrong(5.0f, nan); }) &&
                  throwsAsExpected<std::out_of_range>([inf]() { Customer wrong(inf); }),
              "reject non-finite locations");
        check(throwsAsExpected<std::invalid_argument>([nan]() { Cafe wrong(nan, 1.0f); }),
              "reject a non-finite coffee price");

        Cafe sample(5.0f, 4.0f);
        check(throwsAsExpected<std::out_of_range>([&sample]() { sample.setLocation(12.0f); }) &&
                  near(sample.getLocation(), 4.0f),
              "an invalid location setter throws and leaves the old location");
        check(throwsAsExpected<std::invalid_argument>([&sample]() { sample.setCoffeeCost(-3.0f); }) &&
                  near(sample.getCoffeeCost(), 5.0f),
              "an invalid price setter throws and leaves the old price");
        sample.setLocation(10.0f);
        sample.setCoffeeCost(4.5f);
        check(near(sample.getLocation(), 10.0f) && near(sample.getCoffeeCost(), 4.5f),
              "setters accept a beach endpoint and a new price");
    }

    // --- Copying, assignment, and non-owning destruction ---
    {
        Cafe* heapCafe = new Cafe(5.0f, 4.0f);
        Cafe other(2.0f, 9.0f);
        Customer original(4.0f);
        Cafe* onlyHeap[] = {heapCafe};
        original.chooseCafe(onlyHeap, 1);

        Customer copied(original);
        check(near(copied.getLocation(), 4.0f) && copied.getVisitedCafe() == heapCafe &&
                  heapCafe->getCustomerCount() == 1,
              "copy constructor copies the cafe pointer and does not record another sale");

        Customer assigned(9.0f);
        Cafe* onlyOther[] = {&other};
        assigned.chooseCafe(onlyOther, 1);
        assigned = original;
        check(assigned.getVisitedCafe() == heapCafe && near(assigned.getLocation(), 4.0f) &&
                  heapCafe->getCustomerCount() == 1 && other.getCustomerCount() == 1,
              "copy assignment replaces the pointer and does not record a sale");

        assigned = assigned;
        check(assigned.getVisitedCafe() == heapCafe && heapCafe->getCustomerCount() == 1,
              "self-assignment keeps the same non-owning cafe pointer");

        Customer first(1.0f);
        Customer second(2.0f);
        Customer third(3.0f);
        first = second = third;
        check(near(first.getLocation(), 3.0f) && near(second.getLocation(), 3.0f) &&
                  first.getVisitedCafe() == nullptr && second.getVisitedCafe() == nullptr,
              "chained assignment copies location and a null cafe pointer");

        {
            Customer temporary(original);
            Customer anotherCopy;
            anotherCopy = temporary;
            check(temporary.getVisitedCafe() == heapCafe &&
                      anotherCopy.getVisitedCafe() == heapCafe,
                  "copies share the cafe address while the cafe is still alive");
        }
        check(original.getVisitedCafe() == heapCafe && heapCafe->getCustomerCount() == 1 &&
                  near(heapCafe->getTotalProfit(), 3.0f),
              "destroying customer copies does not delete the cafe or change its sale");

        delete heapCafe;
    }

    // --- Scores: distance is nonnegative, endpoints, same position ---
    {
        Cafe atCustomer(5.0f, 4.0f);
        Customer sameSpot(4.0f);
        checkNear(sameSpot.calculateScore(atCustomer), 13.0f,
                  "score at the same position is 28 - 0 - 3*5 = 13");

        Cafe atZero(5.0f, 0.0f);
        Cafe atTen(5.0f, 10.0f);
        Customer fromZero(0.0f);
        Customer fromTen(10.0f);
        checkNear(fromZero.calculateScore(atZero), 13.0f,
                  "customer at 0 km scores 13 for a $5 cafe at 0 km");
        checkNear(fromZero.calculateScore(atTen), 3.0f,
                  "customer at 0 km scores 3 for a $5 cafe at 10 km");
        checkNear(fromTen.calculateScore(atTen), 13.0f,
                  "customer at 10 km scores 13 for a $5 cafe at 10 km");
        checkNear(fromTen.calculateScore(atZero), 3.0f,
                  "customer at 10 km scores 3 for a $5 cafe at 0 km");
        check(near(fromZero.calculateScore(atTen), fromTen.calculateScore(atZero)),
              "opposite ends produce the same nonnegative distance");
    }

    // --- Probabilities, ties, prices, and unusable denominators ---
    {
        Cafe nearCafe(5.0f, 0.0f);
        Cafe farCafe(5.0f, 10.0f);
        Customer atZero(0.0f);
        Cafe* identicalPrices[] = {&nearCafe, &farCafe};

        checkNear(atZero.calculateProbability(identicalPrices, 2, 0), 13.0f / 16.0f,
                  "identical $5 prices: probability of the cafe at 0 km is 13/16");
        checkNear(atZero.calculateProbability(identicalPrices, 2, 1), 3.0f / 16.0f,
                  "identical $5 prices: probability of the cafe at 10 km is 3/16");
        checkNear(atZero.calculateProbability(identicalPrices, 2, 0) +
                      atZero.calculateProbability(identicalPrices, 2, 1),
                  1.0f, "probabilities of the two $5 cafes add to 1");

        atZero.chooseCafe(identicalPrices, 2);
        check(atZero.getVisitedCafe() == &nearCafe && nearCafe.getCustomerCount() == 1 &&
                  farCafe.getCustomerCount() == 0,
              "highest probability selects the closer equally priced cafe and stores it");

        Customer atTen(10.0f);
        atTen.chooseCafe(identicalPrices, 2);
        check(atTen.getVisitedCafe() == &farCafe && farCafe.getCustomerCount() == 1,
              "customer at 10 km selects the cafe at 10 km");

        // Different prices. Customer at 5 km.
        // Cafe A at 5 km, $6: score = 28 - 0 - 18 = 10
        // Cafe B at 8 km, $2: score = 28 - 3 - 6 = 19
        Cafe expensiveClose(6.0f, 5.0f);
        Cafe cheapFar(2.0f, 8.0f);
        Customer middle(5.0f);
        Cafe* differentPrices[] = {&expensiveClose, &cheapFar};
        checkNear(middle.calculateScore(expensiveClose), 10.0f,
                  "different prices: nearby $6 cafe scores 10");
        checkNear(middle.calculateScore(cheapFar), 19.0f,
                  "different prices: farther $2 cafe scores 19");
        checkNear(middle.calculateProbability(differentPrices, 2, 0), 10.0f / 29.0f,
                  "probability of the $6 cafe is 10/29");
        checkNear(middle.calculateProbability(differentPrices, 2, 1), 19.0f / 29.0f,
                  "probability of the $2 cafe is 19/29");
        middle.chooseCafe(differentPrices, 2);
        check(middle.getVisitedCafe() == &cheapFar && cheapFar.getCustomerCount() == 1 &&
                  expensiveClose.getCustomerCount() == 0,
              "highest probability selects the cheaper cafe when its probability is larger");

        // Same score, so the same probability. The earlier array entry wins.
        Cafe tieLeft(4.0f, 4.0f);
        Cafe tieRight(4.0f, 6.0f);
        Customer centered(5.0f);
        checkNear(centered.calculateScore(tieLeft), 15.0f, "left cafe of a tie scores 15");
        checkNear(centered.calculateScore(tieRight), 15.0f, "right cafe of a tie scores 15");
        Cafe* tieFirstRight[] = {&tieRight, &tieLeft};
        centered.chooseCafe(tieFirstRight, 2);
        const float rightProb = centered.calculateProbability(tieFirstRight, 2, 0);
        const float leftProb = centered.calculateProbability(tieFirstRight, 2, 1);
        check(near(rightProb, leftProb) && centered.getVisitedCafe() == &tieRight &&
                  tieRight.getCustomerCount() == 1 && tieLeft.getCustomerCount() == 0,
              "equal probabilities keep the earliest cafe in the array");

        Cafe tieA(5.0f, 5.0f);
        Cafe tieB(5.0f, 5.0f);
        Cafe* reversed[] = {&tieB, &tieA};
        Customer again(5.0f);
        again.chooseCafe(reversed, 2);
        check(again.getVisitedCafe() == &tieB && tieA.getCustomerCount() == 0,
              "a second tie also keeps the first cafe rather than the later twin");

        // Three cafes, ordinary normalization. Customer at 2 km, every price $5.
        // Scores: dist 2 -> 11, dist 0 -> 13, dist 8 -> 5. Sum = 29.
        Cafe c0(5.0f, 0.0f);
        Cafe c1(5.0f, 2.0f);
        Cafe c2(5.0f, 10.0f);
        Customer atTwo(2.0f);
        Cafe* three[] = {&c0, &c1, &c2};
        const float p0 = atTwo.calculateProbability(three, 3, 0);
        const float p1 = atTwo.calculateProbability(three, 3, 1);
        const float p2 = atTwo.calculateProbability(three, 3, 2);
        checkNear(p0, 11.0f / 29.0f, "three-cafe probability of the cafe at 0 km is 11/29");
        checkNear(p1, 13.0f / 29.0f, "three-cafe probability of the cafe at 2 km is 13/29");
        checkNear(p2, 5.0f / 29.0f, "three-cafe probability of the cafe at 10 km is 5/29");
        checkNear(p0 + p1 + p2, 1.0f, "three valid probabilities add to 1");
        atTwo.chooseCafe(three, 3);
        check(atTwo.getVisitedCafe() == &c1 && c1.getCustomerCount() == 1 &&
                  c0.getCustomerCount() == 0 && c2.getCustomerCount() == 0,
              "the cafe with the highest of three probabilities is selected");

        // One cafe: its probability is 1 even though it is the only choice.
        Cafe alone(5.0f, 6.0f);
        Customer visitor(6.0f);
        Cafe* one[] = {&alone};
        checkNear(visitor.calculateProbability(one, 1, 0), 1.0f,
                  "a single cafe has probability 1");
        visitor.chooseCafe(one, 1);
        check(visitor.getVisitedCafe() == &alone, "a single cafe is stored as the selection");

        // Positive total with a negative score. Probabilities are not clamped.
        // Cafe P at 0 km, $4: score = 16. Cafe N at 0 km, $12: score = -8. Sum = 8.
        Cafe positiveScore(4.0f, 0.0f);
        Cafe negativeScore(12.0f, 0.0f);
        Customer origin(0.0f);
        Cafe* mixedSigns[] = {&positiveScore, &negativeScore};
        checkNear(origin.calculateProbability(mixedSigns, 2, 0), 2.0f,
                  "a negative competing score can make a probability larger than 1");
        checkNear(origin.calculateProbability(mixedSigns, 2, 1), -1.0f,
                  "a negative score over a positive total gives a negative probability");
        origin.chooseCafe(mixedSigns, 2);
        check(origin.getVisitedCafe() == &positiveScore && negativeScore.getCustomerCount() == 0,
              "the larger probability is selected when one probability is negative");

        // Negative total. Highest probability is not the highest score.
        // Cafe low at 0 km, $20: score = -32. Cafe high at 10 km, $12: score = -18.
        // Sum = -50. Probabilities: 0.64 and 0.36. The lower score wins.
        Cafe lowerScore(20.0f, 0.0f);
        Cafe higherScore(12.0f, 10.0f);
        Customer picker(0.0f);
        Cafe* negativeTotal[] = {&lowerScore, &higherScore};
        check(picker.calculateScore(lowerScore) < picker.calculateScore(higherScore),
              "setup check: the first cafe of the negative-total pair has the lower score");
        checkNear(picker.calculateProbability(negativeTotal, 2, 0), 0.64f,
                  "negative total: lower score has probability 0.64");
        checkNear(picker.calculateProbability(negativeTotal, 2, 1), 0.36f,
                  "negative total: higher score has probability 0.36");
        picker.chooseCafe(negativeTotal, 2);
        check(picker.getVisitedCafe() == &lowerScore && lowerScore.getCustomerCount() == 1 &&
                  higherScore.getCustomerCount() == 0,
              "selection uses probability, so a negative total can reject the higher score");

        // Score of exactly 0. Denominator is unusable, so nobody is selected.
        // Customer at 0, cafe at 1 km, price $9: 28 - 1 - 27 = 0.
        Cafe zeroScore(9.0f, 1.0f);
        Customer zeroCustomer(0.0f);
        Cafe* zeroList[] = {&zeroScore};
        checkNear(zeroCustomer.calculateScore(zeroScore), 0.0f, "constructed score is exactly 0");
        check(zeroCustomer.calculateProbability(zeroList, 1, 0) == 0.0f,
              "a zero total reports an undefined probability as 0");
        zeroCustomer.chooseCafe(zeroList, 1);
        check(zeroCustomer.getVisitedCafe() == nullptr && zeroScore.getCustomerCount() == 0,
              "a zero total selects no cafe and records no sale");

        // Scores 28 and -28 cancel. Still no sale, on either cafe.
        Cafe plus(0.0f, 5.0f);
        Cafe minus(18.0f, 7.0f);
        Customer cancelling(5.0f);
        Cafe* cancelList[] = {&plus, &minus};
        checkNear(cancelling.calculateScore(plus) + cancelling.calculateScore(minus), 0.0f,
                  "opposite scores sum to 0");
        cancelling.chooseCafe(cancelList, 2);
        check(cancelling.getVisitedCafe() == nullptr && plus.getCustomerCount() == 0 &&
                  minus.getCustomerCount() == 0,
              "scores that cancel select no cafe and record no sale");

        // A later valid choice still works after an undefined probability.
        Cafe recovery(5.0f, 0.0f);
        Cafe* recoveryList[] = {&recovery};
        zeroCustomer.chooseCafe(recoveryList, 1);
        check(zeroCustomer.getVisitedCafe() == &recovery && recovery.getCustomerCount() == 1,
              "a customer who could not choose can still select a later valid cafe");

        // Non-finite score from an enormous legal price. Must not crash or sell.
        Cafe huge(std::numeric_limits<float>::max(), 0.0f);
        Cafe ordinary(5.0f, 5.0f);
        Customer careful(0.0f);
        Cafe* hugeList[] = {&ordinary, &huge};
        check(!std::isfinite(careful.calculateScore(huge)),
              "an enormous coffee price produces a non-finite score");
        check(careful.calculateProbability(hugeList, 2, 0) == 0.0f &&
                  careful.calculateProbability(hugeList, 2, 1) == 0.0f,
              "a non-finite total reports undefined probabilities as 0");
        careful.chooseCafe(hugeList, 2);
        check(careful.getVisitedCafe() == nullptr && ordinary.getCustomerCount() == 0 &&
                  huge.getCustomerCount() == 0,
              "a non-finite total selects no cafe and does not crash");

        check(careful.calculateProbability(nullptr, 0, 0) == 0.0f, "null cafe list returns probability 0");
        check(careful.calculateProbability(hugeList, 2, 5) == 0.0f,
              "an out-of-range cafe index returns probability 0");
        check(careful.calculateProbability(hugeList, 2, -1) == 0.0f,
              "a negative cafe index returns probability 0");
    }

    // --- Empty collections, null pointers, and one sale per customer ---
    {
        Cafe sold(5.0f, 3.0f);
        Customer buyer(3.0f);
        check(throwsAsExpected<std::invalid_argument>([&buyer]() { buyer.chooseCafe(nullptr, 0); }),
              "an empty cafe collection is rejected");
        check(buyer.getVisitedCafe() == nullptr && sold.getCustomerCount() == 0,
              "a rejected empty collection leaves the customer and cafe unchanged");

        Cafe* present[] = {&sold};
        check(throwsAsExpected<std::invalid_argument>([&buyer, &present]() {
                  buyer.chooseCafe(present, 0);
              }),
              "a cafe count of 0 is rejected");
        check(throwsAsExpected<std::invalid_argument>([&buyer, &present]() {
                  buyer.chooseCafe(present, -1);
              }),
              "a negative cafe count is rejected");

        buyer.chooseCafe(present, 1);
        const int salesBeforeNull = sold.getCustomerCount();
        Cafe* hasNull[] = {&sold, nullptr};
        check(throwsAsExpected<std::invalid_argument>([&buyer, &hasNull]() {
                  buyer.chooseCafe(hasNull, 2);
              }) && sold.getCustomerCount() == salesBeforeNull &&
                  buyer.getVisitedCafe() == &sold,
              "a null cafe pointer throws, does not crash, and does not record a sale");

        Cafe* startsNull[] = {nullptr, &sold};
        Customer fresh(3.0f);
        check(throwsAsExpected<std::invalid_argument>([&fresh, &startsNull]() {
                  fresh.chooseCafe(startsNull, 2);
              }) && fresh.getVisitedCafe() == nullptr &&
                  sold.getCustomerCount() == salesBeforeNull,
              "a null pointer earlier in the list also records no sale");

        buyer.chooseCafe(present, 1);
        check(sold.getCustomerCount() == 1 && near(sold.getTotalProfit(), 3.0f) &&
                  buyer.getVisitedCafe() == &sold,
              "one customer records exactly one sale even if chooseCafe is called twice");

        Customer secondBuyer(3.0f);
        secondBuyer.chooseCafe(present, 1);
        check(secondBuyer.getVisitedCafe() == &sold && sold.getCustomerCount() == 2 &&
                  near(sold.getTotalProfit(), 6.0f),
              "a second customer records a second sale on the same cafe");
    }

    // --- Profit: $5 -> $3, $2 -> $0, cumulative totals, counts, reset ---
    {
        Cafe fiveDollar(5.0f, 1.0f);
        Cafe* fiveList[] = {&fiveDollar};
        Customer first(1.0f);
        first.chooseCafe(fiveList, 1);
        check(fiveDollar.getCustomerCount() == 1 && near(fiveDollar.getTotalProfit(), 3.0f),
              "a $5 coffee produces $3 profit on one sale");

        Customer second(1.0f);
        Customer third(2.0f);
        second.chooseCafe(fiveList, 1);
        third.chooseCafe(fiveList, 1);
        check(fiveDollar.getCustomerCount() == 3 && near(fiveDollar.getTotalProfit(), 9.0f),
              "three $5 sales accumulate $9 profit and 3 customers");

        Cafe twoDollar(2.0f, 8.0f);
        Cafe* twoList[] = {&twoDollar};
        Customer breakEven(8.0f);
        breakEven.chooseCafe(twoList, 1);
        check(twoDollar.getCustomerCount() == 1 && near(twoDollar.getTotalProfit(), 0.0f),
              "a $2 coffee produces $0 profit per sale");
        Customer another(8.0f);
        another.chooseCafe(twoList, 1);
        check(twoDollar.getCustomerCount() == 2 && near(twoDollar.getTotalProfit(), 0.0f),
              "repeated $2 sales keep a cumulative profit of $0 and increase the count");

        Cafe freeCoffee(0.0f, 0.0f);
        Cafe* freeList[] = {&freeCoffee};
        Customer deal(0.0f);
        deal.chooseCafe(freeList, 1);
        check(freeCoffee.getCustomerCount() == 1 && near(freeCoffee.getTotalProfit(), -2.0f),
              "a $0 coffee produces -$2 profit because production still costs $2");

        fiveDollar.setCoffeeCost(2.0f);
        Customer afterPriceChange(1.0f);
        afterPriceChange.chooseCafe(fiveList, 1);
        check(fiveDollar.getCustomerCount() == 4 && near(fiveDollar.getTotalProfit(), 9.0f),
              "a later $2 sale adds $0 and does not rewrite earlier profit");

        fiveDollar.resetStatistics();
        check(fiveDollar.getCustomerCount() == 0 && near(fiveDollar.getTotalProfit(), 0.0f),
              "reset clears profit and customer count");
        check(first.getVisitedCafe() == &fiveDollar,
              "reset does not destroy the cafe or clear a customer's non-owning pointer");

        Customer afterReset(1.0f);
        afterReset.chooseCafe(fiveList, 1);
        check(fiveDollar.getCustomerCount() == 1 && near(fiveDollar.getTotalProfit(), 0.0f),
              "the first sale after reset uses the current $2 price");

        std::ostringstream printed;
        const std::ios::fmtflags oldFlags = std::cout.flags();
        std::cout.precision(8);
        const std::streamsize callerPrecision = std::cout.precision();
        std::streambuf* oldBuffer = std::cout.rdbuf(printed.rdbuf());
        fiveDollar.printSummary();
        std::cout.rdbuf(oldBuffer);
        const std::streamsize precisionAfterSummary = std::cout.precision();
        std::cout.flags(oldFlags);
        std::cout.precision(6);
        check(printed.str().find("1 customers") != std::string::npos &&
                  printed.str().find("$0.00") != std::string::npos,
              "summary prints the customer count and profit to two decimals");
        check(precisionAfterSummary == callerPrecision,
              "summary restores the caller's stream precision");
    }

    // --- Fixed mixed-price sweep, including both ends of the beach ---
    // $4 at 0 km, $5 at 5 km, and $4 at 10 km each win a stretch of the beach.
    // A $2 cafe is covered by the profit checks above; on this beach it would
    // outscore a $5 cafe everywhere, so it would not leave the others a region.
    {
        Cafe west(4.0f, 0.0f);
        Cafe center(5.0f, 5.0f);
        Cafe east(4.0f, 10.0f);
        Cafe* row[] = {&west, &center, &east};

        int selectionMismatches = 0;
        float predictedProfit = 0.0f;
        const int steps = 101;
        for (int step = 0; step < steps; ++step) {
            const float spot = step / 10.0f;
            const int expected = indexOfHighestProbability(spot, row, 3);
            Customer person(spot);
            person.chooseCafe(row, 3);
            const bool matches = (expected < 0) ? person.getVisitedCafe() == nullptr
                                                : person.getVisitedCafe() == row[expected];
            if (!matches) {
                ++selectionMismatches;
            }
            if (expected >= 0) {
                predictedProfit += row[expected]->getCoffeeCost() - 2.0f;
            }
        }

        const int totalCustomers =
            west.getCustomerCount() + center.getCustomerCount() + east.getCustomerCount();
        const float actualProfit =
            west.getTotalProfit() + center.getTotalProfit() + east.getTotalProfit();
        check(selectionMismatches == 0,
              "every customer from 0 km through 10 km selected the highest probability");
        check(totalCustomers == steps,
              "each customer in the mixed-price sweep was recorded once");
        checkNear(actualProfit, predictedProfit,
                  "mixed-price profit matches price minus $2 for each independent selection");
        check(west.getCustomerCount() > 0 && center.getCustomerCount() > 0 &&
                  east.getCustomerCount() > 0,
              "west, center, and east cafes each win part of the beach");
    }

    // --- 50,000 reproducible trials with four competing cafes ---
    {
        Cafe simA(5.0f, 1.0f);
        Cafe simB(5.0f, 4.0f);
        Cafe simC(5.0f, 7.0f);
        Cafe simD(5.0f, 9.0f);
        Cafe* simCafes[] = {&simA, &simB, &simC, &simD};

        std::mt19937 generator(19);
        std::uniform_real_distribution<float> positions(0.0f, 10.0f);
        const int trialCount = 50000;
        int mismatches = 0;

        for (int trial = 0; trial < trialCount; ++trial) {
            const float spot = positions(generator);
            const int expected = indexOfHighestProbability(spot, simCafes, 4);
            Customer arriving(spot);
            arriving.chooseCafe(simCafes, 4);
            if (arriving.getVisitedCafe() != simCafes[expected]) {
                ++mismatches;
            }
        }

        const int totalCustomers = simA.getCustomerCount() + simB.getCustomerCount() +
                                   simC.getCustomerCount() + simD.getCustomerCount();
        const float totalProfit = simA.getTotalProfit() + simB.getTotalProfit() +
                                  simC.getTotalProfit() + simD.getTotalProfit();

        std::ostringstream mismatchDetail;
        mismatchDetail << mismatches << " of " << trialCount
                       << " customers selected a cafe other than the highest probability";
        check(mismatches == 0, mismatchDetail.str());
        check(totalCustomers == trialCount,
              "all 50,000 customers were counted exactly once across the four cafes");
        checkNear(totalProfit, trialCount * 3.0f,
                  "aggregate profit of 50,000 sales at $5 is 50,000 * ($5 - $2)");
        check(simA.getCustomerCount() > 0 && simB.getCustomerCount() > 0 &&
                  simC.getCustomerCount() > 0 && simD.getCustomerCount() > 0,
              "each of the four competing cafes served at least one customer");
    }

    std::cout << "Group 19 test results: " << (failed == 0 ? "passed" : "failed")
              << " (" << passed << " passed, " << failed << " failed).\n";
    return (failed == 0) ? 0 : 1;
}
