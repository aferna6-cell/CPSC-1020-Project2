#ifndef CAFE_H
#define CAFE_H

// Represents one cafe along a 10-km beach.
class Cafe {
private:
    float location;       // Cafe position on the beach (km)
    float coffeeCost;     // Price charged for one coffee (dollars)
    float totalProfit;    // Profit earned during the simulation (dollars)
    int customerCount;    // Number of customers served (no customer list needed)

public:
    // Create a cafe with a coffee price and beach location.
    Cafe(float coffeeCost, float location);

    // Access the cafe's information and simulation statistics.
    float getLocation() const;
    float getCoffeeCost() const;
    float getTotalProfit() const;
    int getCustomerCount() const;

    // Change a cafe's settings before a simulation.
    void setLocation(float newLocation);
    void setCoffeeCost(float newCoffeeCost);

    // Record one customer purchase: profit increases by (coffeeCost - $2).
    void recordSale();

    // Clear profit and customer count before a new simulation run.
    void resetStatistics();

    // Display this cafe's customer count and total profit.
    void printSummary() const;
};

#endif // CAFE_H
