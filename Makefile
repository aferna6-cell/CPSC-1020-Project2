# CPSC 1020 Project 2 -- Group 19
# Builds ONE executable: P2G19main (the cafe simulation).
#   make        build P2G19main
#   make run    build and run the simulation
#   make clean  remove object files and the executable

CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -pedantic
TARGET = P2G19main
OBJECTS = P2G19main.o Cafe.o Customer.o

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS)

P2G19main.o: P2G19main.cpp Cafe.h Customer.h
	$(CXX) $(CXXFLAGS) -c P2G19main.cpp

Cafe.o: Cafe.cpp Cafe.h
	$(CXX) $(CXXFLAGS) -c Cafe.cpp

Customer.o: Customer.cpp Customer.h Cafe.h
	$(CXX) $(CXXFLAGS) -c Customer.cpp

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all run clean
