# CPSC 1020 Project 2 -- Group 19
# make       build the Group 19 test program
# make test  build it, run it, and return nonzero if any check fails
# make clean remove object files and the test program

CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -Wpedantic -O2
TARGET = P2G19test
OBJECTS = Cafe.o Customer.o P2G19test.o

.PHONY: all test clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJECTS)

Cafe.o: Cafe.cpp Cafe.h
	$(CXX) $(CXXFLAGS) -c Cafe.cpp

Customer.o: Customer.cpp Customer.h Cafe.h
	$(CXX) $(CXXFLAGS) -c Customer.cpp

P2G19test.o: P2G19test.cpp Cafe.h Customer.h
	$(CXX) $(CXXFLAGS) -c P2G19test.cpp

test: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJECTS) $(TARGET)
