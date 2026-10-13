#include "copyright.h"
#include "system.h"
#include "synch.h"
#include "elevator.h"

#define ELEVATOR_CAPACITY 5
#define TICKS_PER_FLOOR 50

int nextPersonID = 1;
Lock *personIDLock = new Lock("PersonIDLock");

ELEVATOR *e = NULL;

ELEVATOR::ELEVATOR(int floors) {
    numFloors = floors;
    currentFloor = 1;
    direction = 1;
    occupancy = 0;
    maxOccupancy = ELEVATOR_CAPACITY;
    totalWaiting = 0;
    boardingSlots = 0;
    doorsOpenForExit = FALSE;

    elevatorLock = new Lock("Elevator Lock");
    idle = new Condition("Elevator Idle");
    doorDone = new Condition("Elevator Door Done");

    personsWaiting = new int[numFloors + 1];
    personsGoingTo = new int[numFloors + 1];
    entering = new Condition*[numFloors + 1];
    leaving = new Condition*[numFloors + 1];
    for (int i = 0; i <= numFloors; i++) {
        personsWaiting[i] = 0;
        personsGoingTo[i] = 0;
        entering[i] = new Condition("Entering");
        leaving[i] = new Condition("Leaving");
    }
}

ELEVATOR::~ELEVATOR() {
    for (int i = 0; i <= numFloors; i++) {
        delete entering[i];
        delete leaving[i];
    }
    delete [] entering;
    delete [] leaving;
    delete [] personsWaiting;
    delete [] personsGoingTo;
    delete idle;
    delete doorDone;
    delete elevatorLock;
}

bool ELEVATOR::requestsAbove() {
    for (int f = currentFloor + 1; f <= numFloors; f++)
        if (personsWaiting[f] > 0 || personsGoingTo[f] > 0)
            return TRUE;
    return FALSE;
}

bool ELEVATOR::requestsBelow() {
    for (int f = currentFloor - 1; f >= 1; f--)
        if (personsWaiting[f] > 0 || personsGoingTo[f] > 0)
            return TRUE;
    return FALSE;
}

void ELEVATOR::unload() {
    if (personsGoingTo[currentFloor] == 0)
        return;

    doorsOpenForExit = TRUE;
    leaving[currentFloor]->Broadcast(elevatorLock);
    while (personsGoingTo[currentFloor] > 0)
        doorDone->Wait(elevatorLock);
    doorsOpenForExit = FALSE;
}

void ELEVATOR::load() {
    while (occupancy < maxOccupancy && personsWaiting[currentFloor] > 0) {
        boardingSlots = 1;
        entering[currentFloor]->Signal(elevatorLock);
        while (boardingSlots > 0)
            doorDone->Wait(elevatorLock);
    }
}

void ELEVATOR::start() {

    elevatorLock->Acquire();

    while (1) {

        while (totalWaiting == 0 && occupancy == 0)
            idle->Wait(elevatorLock);

        unload();
        load();
        unload();

        if (direction == 1 && !requestsAbove() && requestsBelow())
            direction = -1;
        else if (direction == -1 && !requestsBelow() && requestsAbove())
            direction = 1;

        if (!requestsAbove() && !requestsBelow())
            continue;

        elevatorLock->Release();
        for (int j = 0; j < TICKS_PER_FLOOR; j++)
            currentThread->Yield();
        elevatorLock->Acquire();

        currentFloor += direction;
        printf("Elevator arrives on floor %d.\n", currentFloor);
    }
}

void ELEVATOR::hailElevator(Person *p) {

    elevatorLock->Acquire();

    printf("Person %d wants to go to floor %d from floor %d.\n",
           p->id, p->toFloor, p->atFloor);

    personsWaiting[p->atFloor]++;
    totalWaiting++;
    idle->Signal(elevatorLock);

    while (currentFloor != p->atFloor || boardingSlots == 0)
        entering[p->atFloor]->Wait(elevatorLock);

    boardingSlots--;
    personsWaiting[p->atFloor]--;
    totalWaiting--;
    occupancy++;
    personsGoingTo[p->toFloor]++;
    printf("Person %d got into the elevator.\n", p->id);
    doorDone->Signal(elevatorLock);

    while (currentFloor != p->toFloor || !doorsOpenForExit)
        leaving[p->toFloor]->Wait(elevatorLock);

    personsGoingTo[p->toFloor]--;
    occupancy--;
    printf("Person %d got out of the elevator.\n", p->id);
    doorDone->Signal(elevatorLock);

    elevatorLock->Release();
}

void ElevatorThread(int numFloors) {
    DEBUG('t', "Elevator with %d floors started\n", numFloors);
    e->start();
}

void Elevator(int numFloors) {
    e = new ELEVATOR(numFloors);

    Thread *t = new Thread("Elevator");
    t->Fork(ElevatorThread, numFloors);
}

void PersonThread(int person) {
    Person *p = (Person *)person;

    e->hailElevator(p);

    delete p;
}

int getNextPersonID() {
    personIDLock->Acquire();
    int personID = nextPersonID;
    nextPersonID = nextPersonID + 1;
    personIDLock->Release();
    return personID;
}

void ArrivingGoingFromTo(int atFloor, int toFloor) {

    Person *p = new Person;
    p->id = getNextPersonID();
    p->atFloor = atFloor;
    p->toFloor = toFloor;

    char *name = new char[20];
    sprintf(name, "Person %d", p->id);
    Thread *t = new Thread(name);
    t->Fork(PersonThread, (int)p);
}
