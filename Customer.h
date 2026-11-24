#ifndef CUSTOMER_H
#define CUSTOMER_H

#include "Cafe.h"

// Represents one customer arriving at a position on the beach.
class Customer {
private:
    float location; // Customer position on the beach (km)

    // Pointer to this customer's own record of the cafe they visited.
    // The Customer OWNS this object: it is created with new when the customer
    // chooses a cafe, deep-copied by the copy constructor and copy assignment
    // operator, and deleted by the destructor. It is nullptr until a cafe is
    // chosen. The sale itself is recorded on the program's real cafe.
    Cafe* visitedCafe;

public:
    // Create a customer at 0 km who has not visited a cafe.
    Customer();

    // Create a customer at the given location (0 to 10 km) who has not
    // visited a cafe yet. Throws std::out_of_range for a bad location.
    explicit Customer(float location);

    // ---- Big Three ----

    // Copy constructor (deep copy): allocates a NEW Cafe and copies the
    // values of the other customer's visited cafe into it. The two customers
    // never share the same Cafe address.
    Customer(const Customer& other);

    // Copy assignment operator (deep copy): allocates a new Cafe holding a
    // copy of the other customer's visited cafe, deletes this customer's old
    // Cafe, then stores the new pointer. Self-assignment is safe.
    Customer& operator=(const Customer& other);

    // Destructor: deletes the Cafe this customer owns.
    ~Customer();

    // Access the customer's location and their record of the visited cafe
    // (nullptr if no cafe has been chosen).
    float getLocation() const;
    const Cafe* getVisitedCafe() const;

    // Score a cafe using: 28 - distance - 3 * coffee price.
    // Distance is the absolute difference between cafe and customer
    // locations, so it is never negative.
    float calculateScore(const Cafe& cafe) const;

    // Probability of visiting cafes[cafeIndex]:
    //   score of that cafe / sum of the scores of every cafe.
    // Returns 0 if the list or index is unusable, or the sum is 0 or
    // non-finite (the probability is undefined).
    float calculateProbability(Cafe* cafes[], int cafeCount, int cafeIndex) const;

    // Choose the cafe with the highest probability from an array of cafe
    // pointers, record one sale on that (program-owned) cafe, and store a
    // deep copy of it in visitedCafe (replacing any earlier visit).
    // Cafes with exactly equal probabilities split customers fairly: one of
    // the tied cafes is picked at random.
    // Returns the index of the chosen cafe, or -1 if the sum of scores is 0
    // or non-finite (no sale is recorded).
    // Throws std::invalid_argument for an empty list or a null pointer.
    int chooseCafe(Cafe* cafes[], int cafeCount);
};

#endif // CUSTOMER_H
