#ifndef CUSTOMER_H
#define CUSTOMER_H

#include "Cafe.h"

// Represents one customer arriving at a position on the beach.
class Customer {
private:
    float location; // Customer position on the beach (km)

    // Non-owning pointer to the cafe this customer selected.
    // Customer does not own the Cafe object and must never delete it.
    // The surrounding program creates the cafes and destroys them.
    // Copying or assigning a Customer copies this address only, so both
    // customers refer to the same Cafe. Destroying a Customer does not
    // destroy that Cafe, and it does not change the Cafe's statistics.
    Cafe* visitedCafe;

public:
    // Create a customer at 0 km who has not selected a cafe.
    Customer();

    // Create a customer at the given location (initially visits no cafe).
    explicit Customer(float location);

    // Copy the location and the non-owning cafe pointer. The cafe is not cloned.
    Customer(const Customer& other);

    // Copy the location and the non-owning cafe pointer.
    // Self-assignment is safe. No cafe is deleted or cloned.
    Customer& operator=(const Customer& other);

    // Destructor: cafe pointer is non-owning, so the cafe is not deleted.
    ~Customer();

    // Access the customer's location and selected cafe.
    float getLocation() const;
    Cafe* getVisitedCafe() const;

    // Score a cafe using: 28 - distance - 3 * coffee price.
    // Distance is the absolute difference between cafe and customer locations.
    // The distance is never negative.
    float calculateScore(const Cafe& cafe) const;

    // Probability of visiting cafes[cafeIndex].
    // probability = score of that cafe / sum of the scores of every cafe.
    // Every cafe in the collection is included in the sum.
    // A negative score is left as the formula produces it.
    // If the list is unusable, or the sum is 0 or non-finite, the probability
    // is undefined and this function returns 0.
    float calculateProbability(Cafe* cafes[], int cafeCount, int cafeIndex) const;

    // Select the cafe with the highest probability from an array of cafe pointers.
    // The probability of each cafe is its score divided by the sum of all scores.
    // Stores a non-owning pointer to the selected cafe and records its sale.
    // A customer records at most one sale; a later call does not sell again.
    // Ties (equal probabilities) keep the earliest cafe in the array.
    // When the sum of scores is negative, probabilities are still compared
    // directly, which can choose a different cafe than the highest raw score.
    // An empty collection or a null pointer is rejected and does not record a sale.
    // If the sum of scores is 0 or non-finite, no cafe is selected and no sale
    // is recorded.
    void chooseCafe(Cafe* cafes[], int cafeCount);
};

#endif // CUSTOMER_H
