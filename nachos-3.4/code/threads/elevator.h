#ifndef ELEVATOR_H
#define ELEVATOR_H

#include "copyright.h"
#include "synch.h"

void Elevator(int numFloors);
void ArrivingGoingFromTo(int atFloor, int toFloor);

typedef struct Person {
    int id;
    int atFloor;
    int toFloor;
} Person;

class ELEVATOR {

public:
    ELEVATOR(int numFloors);
    ~ELEVATOR();
    void hailElevator(Person *p);
    void start();

private:
    void unload();
    void load();
    bool requestsAbove();
    bool requestsBelow();

    int numFloors;
    int currentFloor;
    int direction;
    int occupancy;
    int maxOccupancy;
    int totalWaiting;
    int *personsWaiting;
    int *personsGoingTo;
    int boardingSlots;
    bool doorsOpenForExit;

    Lock *elevatorLock;
    Condition **entering;
    Condition **leaving;
    Condition *idle;
    Condition *doorDone;
};

#endif
