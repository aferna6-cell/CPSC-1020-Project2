#ifndef CUSTOMER_H
#define CUSTOMER_H

class Cafe; // Forward declaration: Customer stores a pointer to a Cafe.

// Represents one customer arriving at a position on the beach.
class Customer {
private:
    float location;    // Customer position on the beach (km)
    Cafe* visitedCafe; // Non-owning pointer; this class does not delete the cafe

public:
    // Create a customer at the given location (initially visits no cafe).
    explicit Customer(float location);

    // Access the customer's location and selected cafe.
    float getLocation() const;
    Cafe* getVisitedCafe() const;

    // Score a cafe using: 28 - distance - 3 * coffee price.
    // Distance is the absolute difference between cafe and customer locations.
    float calculateScore(const Cafe& cafe) const;

    // Select the cafe with the highest score from an array of cafe pointers.
    // Records the selected cafe and its sale.
    void chooseCafe(Cafe* cafes[], int cafeCount);
};

#endif // CUSTOMER_H
