#include "copyright.h"
#include "system.h"
#include "synch.h"
#include "elevator.h"

void ElevatorTest(int numFloors, int numPersons) {

    Elevator(numFloors);

    for (int i = 0 ; i < numPersons; i++) {
        int atFloor = (Random() % numFloors) + 1;
        int toFloor = -1 ;
        do {
            toFloor = (Random() % numFloors) + 1;
        } while (atFloor == toFloor) ;

        ArrivingGoingFromTo(atFloor, toFloor);

        int gap = Random() % 100;
        for(int j =0 ; j< gap; j++) {
            currentThread->Yield();
        }
    }

}
