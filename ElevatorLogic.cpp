/*
 * ElevatorLogic.cpp
 *
 *  Created on: 20.06.2014
 *      Author: STSJR
 */

#include "ElevatorLogic.h"

#include <iostream>
#include <algorithm>

#include "Interface.h"
#include "Person.h"
#include "Floor.h"
#include "Elevator.h"
#include "Event.h"
#include "Environment.h"
#include "UpDownButton.h"

ElevatorLogic::ElevatorLogic()
	: EventHandler("ElevatorLogic")
{
	std::cout << "ElevatorLogic(void)" << std::endl;
}

void ElevatorLogic::Initialize(Environment &environment)
{
	//Interface events
	environment.RegisterEventHandler(EVENT_OUT_INTERFACE_NOTIFY, 	 this, &ElevatorLogic::handleInterfaceNotify);

	//Elevator events
	environment.RegisterEventHandler(EVENT_OUT_ELEVATOR_MOVING,  	 this, &ElevatorLogic::handleElevatorMoving);
	environment.RegisterEventHandler(EVENT_OUT_ELEVATOR_STOPPED, 	 this, &ElevatorLogic::handleElevatorStopped);
	environment.RegisterEventHandler(EVENT_OUT_ELEVATOR_OPENING, 	 this, &ElevatorLogic::handleElevatorOpening);
	environment.RegisterEventHandler(EVENT_OUT_ELEVATOR_OPENED,	 	 this, &ElevatorLogic::handleElevatorOpened);
	environment.RegisterEventHandler(EVENT_OUT_ELEVATOR_CLOSING, 	 this, &ElevatorLogic::handleElevatorClosing);
	environment.RegisterEventHandler(EVENT_OUT_ELEVATOR_CLOSED,  	 this, &ElevatorLogic::handleElevatorClosed);
	environment.RegisterEventHandler(EVENT_OUT_ELEVATOR_BEEPING, 	 this, &ElevatorLogic::handleElevatorBeeping);
	environment.RegisterEventHandler(EVENT_OUT_ELEVATOR_BEEPED,  	 this, &ElevatorLogic::handleElevatorBeeped);
	environment.RegisterEventHandler(EVENT_OUT_ELEVATOR_MALFUNCTION, this, &ElevatorLogic::handleElevatorMalfunction);
	environment.RegisterEventHandler(EVENT_OUT_ELEVATOR_FIXED, 		 this, &ElevatorLogic::handleElevatorFixed);

	//Person events
	environment.RegisterEventHandler(EVENT_OUT_PERSON_ENTERING,  	 this, &ElevatorLogic::handlePersonEntering);
	environment.RegisterEventHandler(EVENT_OUT_PERSON_ENTERED, 	 	 this, &ElevatorLogic::handlePersonEntered);
	environment.RegisterEventHandler(EVENT_OUT_PERSON_EXITING, 	 	 this, &ElevatorLogic::handlePersonExiting);
	environment.RegisterEventHandler(EVENT_OUT_PERSON_EXITED, 	 	 this, &ElevatorLogic::handlePersonExited);
	environment.RegisterEventHandler(EVENT_OUT_PERSON_CANCELED,	 	 this, &ElevatorLogic::handlePersonCanceled);

	//Button events
	environment.RegisterEventHandler(EVENT_OUT_BUTTON_DECIDE, 	 	 this, &ElevatorLogic::handleButtonDecide);
}

void printEvent(const Event &event)
{
	std::cout << "~~~ Event Information ~~~" << std::endl;
	std::cout << "ID:\t\t" << event.GetId() << std::endl;
	std::cout << "Sender:\t\t" << (event.GetSender() ? event.GetSender()->GetName() : "nullptr") << std::endl;
	std::cout << "Handler:\t" << (event.GetEventHandler() ? event.GetEventHandler()->GetName() : "nullptr") << std::endl;
	std::cout << "Event:\t\t" << event.GetEvent() << std::endl;
	std::cout << "Data:\t\t" << event.GetData() << std::endl;
	std::cout << "Type:\t\t" << event.GetType() << std::endl;
	std::cout << "~~~ EOF Event Information ~~~" << std::endl;
}

void ElevatorLogic::handleInterfaceNotify(Environment &environment, const Event &event)
{
	//The interface (may be up-down-button) that was used by a person
	const Interface *pInterface = (Interface *)event.GetSender();

	//The elevator associated with the interface
	Elevator *pElevator = nullptr;

	//The floor to add to the elevator's destinations
	Floor *pDestinationFloor = nullptr;

	//Interface was outside the elevator:
	//----
	//Enqeue the floor the person is on in the elevator destinations
	//Check for the new fastest order of destinations
	//Move the elevator to the determined destination
	if(isOutdoorInterface(pInterface))
	{
		pDestinationFloor = getInterfaceFloor(event);
		PRINTMSG("Interface " << pInterface->GetId() << " (outdoor) sent notification.");
		Elevator *pElevMax = nullptr;

		//Add any unknown elevators to map
		for(int i=0; i<pInterface->GetLoadableCount(); i++)
		{
			pElevator = (Elevator *)pInterface->GetLoadable(i);
			addElevator(pElevator);
		}

		//Find the best elevator to assign the task to
		for (int i = 0; i < pInterface->GetLoadableCount(); i++)
		{
			//Check next elevator
			pElevator = (Elevator *)pInterface->GetLoadable(i);

			//Elevator can go to floor
			if(pDestinationFloor->HasElevator(pElevator))
			{
				//Found one that is already there
				if((pElevator->GetCurrentFloor() == pDestinationFloor) && (pElevator->GetState() != Elevator::Malfunction))
				{
					break;
				}

				//If no max set, use first working.
				if ((pElevMax == nullptr) && (pElevator->GetState() != Elevator::Malfunction))
				{
					pElevMax = pElevator;
				}

				//Search for the fastest elevator with no malfunction.
				else if((pElevator->GetSpeed() >= pElevMax->GetSpeed()) && (pElevator->GetState() != Elevator::Malfunction))
				{
					//Speed tie
					if(pElevator->GetSpeed() == pElevMax->GetSpeed())
					{
						//Use the one with less destinations enqueued
						if(m_elevatorInfo[pElevator->GetId()].destinations.size() <= m_elevatorInfo[pElevMax->GetId()].destinations.size())
						{
							//TODO: possibly check for proximity
							pElevMax = pElevator;
						}
					}

					//Found faster one
					else
					{
						//Use faster one
						pElevMax = pElevator;
					}
				}
			}

			//Reset selection.
			pElevator = nullptr;
		}

		//No elevator on the floor -> use the fastest one of those on other floors
		if(pElevator == nullptr)
		{
			//If no elevator is working, just use the first one in interface list. This should never happen.
			if(pElevMax == nullptr)
			{
				pElevator = (Elevator *)pInterface->GetLoadable(0);
			}
			else
			{
				//Use fastest elevator available
				pElevator = pElevMax;
			}
		}
	}

	//Interface was inside the elevator:
	//----
	//Enqueue the floor the person wants to go to
	//Check for the new fastest order of destinations
	//Move the elevator to the determined destination
	else
	{
		PRINTMSG("Interface " << pInterface->GetId() << " (indoor) sent notification.");
		pDestinationFloor = (Floor *)pInterface->GetLoadable(0);
		Person *pPerson = (Person *)event.GetEventHandler();
		pElevator = pPerson->GetCurrentElevator();
	}

	//Determine the new fastest order of destinations and start moving towards it
	addElevatorDestination(pElevator, pDestinationFloor);
	updateElevatorDestination(pElevator, environment);
}

void ElevatorLogic::handleElevatorMoving(Environment &environment, const Event &event)
{
	Elevator *pElevator = (Elevator *)event.GetSender();
	PRINTMSG("Elevator " << pElevator->GetId() << " started moving towards floor " << m_elevatorInfo[pElevator->GetId()].currentDestination->GetId());
}

void ElevatorLogic::handleElevatorStopped(Environment &environment, const Event &event)
{
	Elevator *pElevator = (Elevator *)event.GetSender();
	PRINTMSG("Elevator " << pElevator->GetId() << " stopped at floor " << pElevator->GetCurrentFloor()->GetId());

	//The elevator has stopped at one of its destination floors
	//Remove the destination from its list and open its doors
	//so passengers may enter.
	m_elevatorInfo[pElevator->GetId()].moving = false;
	removeElevatorDestination(pElevator, pElevator->GetCurrentFloor());

	if(m_elevatorInfo[pElevator->GetId()].closed)
	{
		environment.SendEvent(EVENT_IN_ELEVATOR_OPEN, 0, this, pElevator);
	}
}

void ElevatorLogic::handleElevatorOpening(Environment &environment, const Event &event)
{
	Elevator *pElevator = (Elevator *)event.GetSender();
	m_elevatorInfo[pElevator->GetId()].closed = false;
}

void ElevatorLogic::handleElevatorOpened(Environment &environment, const Event &event)
{
	Elevator *pElevator = (Elevator *)event.GetSender();
	PRINTMSG("Elevator " << pElevator->GetId() << " finished opening its doors.");

	//If not malfunctioning, close doors again. Else beep.
	if(pElevator->GetState() != Elevator::Malfunction && !isElevatorOverloaded(pElevator))
	{
		PRINTMSG("Elevator " << pElevator->GetId() << " will close its doors in 3s.");
		m_elevatorInfo[pElevator->GetId()].closeEventID = environment.SendEvent(EVENT_IN_ELEVATOR_CLOSE, 3, this, pElevator);
	}
}

void ElevatorLogic::handleElevatorClosing(Environment &environment, const Event &event)
{
	Elevator *pElevator = (Elevator *)event.GetSender();
	PRINTMSG("Elevator " << pElevator->GetId() << " started closing its doors.");
}

void ElevatorLogic::handleElevatorClosed(Environment &environment, const Event &event)
{
	Elevator *pElevator = (Elevator *)event.GetSender();
	PRINTMSG("Elevator " << pElevator->GetId() << " finished closing its doors.");
	m_elevatorInfo[pElevator->GetId()].closed = true;
	m_elevatorInfo[pElevator->GetId()].closeEventID = -1;

	//Start moving to destination
	updateElevatorDestination(pElevator, environment);
}

void ElevatorLogic::handleElevatorBeeping(Environment &environment, const Event &event)
{
	Elevator *pElevator = (Elevator *)event.GetSender();
	PRINTMSG("Elevator " << pElevator->GetId() << " started beeping.");

	if(m_elevatorInfo[pElevator->GetId()].closeEventID != -1)
	{
		//Cancel close event
		environment.CancelEvent(m_elevatorInfo[pElevator->GetId()].closeEventID);
		m_elevatorInfo[pElevator->GetId()].closeEventID = -1;
	}
}

void ElevatorLogic::handleElevatorBeeped(Environment &environment, const Event &event)
{
	//Elevator stopped beeping, so close its doors
	Elevator *pElevator = (Elevator *)event.GetSender();
	PRINTMSG("Elevator " << pElevator->GetId() << " stopped beeping. Closing doors.");

	if(m_elevatorInfo[pElevator->GetId()].closeEventID == -1)
	{
		m_elevatorInfo[pElevator->GetId()].closeEventID = environment.SendEvent(EVENT_IN_ELEVATOR_CLOSE, 0, this, pElevator);
	}
}

void ElevatorLogic::handleElevatorMalfunction(Environment &environment, const Event &event)
{
	Elevator *pElevator = (Elevator *)event.GetSender();
	PRINTMSG("Elevator " << pElevator->GetId() << " malfunctions. [Floor: " << pElevator->GetCurrentFloor()->GetId() << "; Pos: " << pElevator->GetPosition() << "; Dir: " << (int)m_elevatorInfo[pElevator->GetId()].direction << "]");

	auto &info = m_elevatorInfo[pElevator->GetId()];

	//Cancel any enqueud stop action
	if(info.stopEventID != -1)
	{
		environment.CancelEvent(info.stopEventID);
		info.stopEventID = -1;
	}

	//Cancel any enqueud close action
	if(info.closeEventID != -1)
	{
		environment.CancelEvent(info.closeEventID);
		info.closeEventID = -1;
	}

	//Mark as not moving
	info.moving = false;

	//Copy assigned destinations to other elevators
	for(auto iter = info.destinations.begin(); iter != info.destinations.end(); iter++)
	{
		Floor *destination = *iter;

		//Select elevator to assign destination to
		for(auto elevatorInfo = m_elevatorInfo.begin(); elevatorInfo != m_elevatorInfo.end(); elevatorInfo++)
		{
			//Check if elevator is working and can reach destination
			Elevator *nextElevator = elevatorInfo->second.elevator;
			if(nextElevator != pElevator && nextElevator->GetState() != Elevator::Malfunction && destination->HasElevator(nextElevator))
			{
				PRINTMSG("Copying destination " << destination->GetId() << " to elevator " << nextElevator->GetId());
				addElevatorDestination(nextElevator, destination);
				updateElevatorDestination(nextElevator, environment);
				break;
			}
		}
	}
}

void ElevatorLogic::handleElevatorFixed(Environment &environment, const Event &event)
{
	Elevator *pElevator = (Elevator *)event.GetSender();
	PRINTMSG("Elevator " << pElevator->GetId() << " fixed. [Floor: " << pElevator->GetCurrentFloor()->GetId() << "; Pos: " << pElevator->GetPosition() << "; Dir: " << (int)m_elevatorInfo[pElevator->GetId()].direction << "]");

	//Continue movement
	updateElevatorDestination(pElevator, environment);
}

void ElevatorLogic::handlePersonEntering(Environment &environment, const Event &event)
{
	Elevator *pElevator = (Elevator *)event.GetEventHandler();
	increaseEnteringPersonCount(pElevator);

	environment.CancelEvent(m_elevatorInfo[pElevator->GetId()].closeEventID);
	m_elevatorInfo[pElevator->GetId()].closeEventID = -1;
}

void ElevatorLogic::handlePersonEntered(Environment &environment, const Event &event)
{
	//Was the elevator overloaded before the person entered
	Elevator *pElevator = (Elevator *)event.GetEventHandler();
	bool wasOverloaded = isElevatorOverloaded(pElevator);
	decreaseEnteringPersonCount(pElevator);

	//Add the person's weight to the elevator load
	Person *pPerson = (Person *)event.GetSender();
	PRINTMSG("Person " << pPerson->GetId() << " entered elevator " << pElevator->GetId() << ".");
	m_elevatorInfo[pElevator->GetId()].currentLoad = m_elevatorInfo[pElevator->GetId()].currentLoad + pPerson->GetWeight();
	PRINTMSG("Elevator load: " << m_elevatorInfo[pElevator->GetId()].currentLoad << " / " << pElevator->GetMaxLoad());

	//Check if the elevator is overloaded now
	if(!wasOverloaded && isElevatorOverloaded(pElevator))
	{
		environment.SendEvent(EVENT_IN_ELEVATOR_BEEP, 0, this, pElevator);
	}

	//Close if we are not overloaded and all persons finished entering
	if(!isElevatorOverloaded(pElevator) && m_elevatorInfo[pElevator->GetId()].enteringPersons == 0)
	{
		m_elevatorInfo[pElevator->GetId()].closeEventID = environment.SendEvent(EVENT_IN_ELEVATOR_CLOSE, 0, this, pElevator);
	}
}

void ElevatorLogic::handlePersonExiting(Environment &environment, const Event &event)
{
	//Person *pPerson = (Person *)event.GetSender();
}

void ElevatorLogic::handlePersonExited(Environment &environment, const Event &event)
{
	//Was the elevator overloaded before the person left
	Elevator *pElevator = (Elevator *)event.GetEventHandler();
	bool wasOverloaded = isElevatorOverloaded(pElevator);

	//Remove the persons weight from the elevator load
	Person *pPerson = (Person *)event.GetSender();
	PRINTMSG("Person " << pPerson->GetId() << " exited elevator " << pElevator->GetId() << ".");
	m_elevatorInfo[pElevator->GetId()].currentLoad = m_elevatorInfo[pElevator->GetId()].currentLoad - pPerson->GetWeight();
	PRINTMSG("Elevator load: " << m_elevatorInfo[pElevator->GetId()].currentLoad << " / " << pElevator->GetMaxLoad());

	//If it was overloaded before check whether the state
	//changed and stop beeping if neccessary
	if(wasOverloaded && !isElevatorOverloaded(pElevator))
	{
		environment.SendEvent(EVENT_IN_ELEVATOR_STOPBEEP, 0, this, pElevator);
	}
}

void ElevatorLogic::handlePersonCanceled(Environment &environment, const Event &event)
{
	//Person *pPerson = (Person *)event.GetSender();
}

void ElevatorLogic::handleButtonDecide(Environment &environment, const Event &event)
{
	//UpDownButton *pButton = (UpDownButton *)event.GetSender();
}

ElevatorLogic::~ElevatorLogic()
{
	std::cout << "~ElevatorLogic(void)" << std::endl;
}

void ElevatorLogic::addElevator(Elevator *elevator)
{
	//Add new elevator information to map
	if(m_elevatorInfo.count(elevator->GetId()) == 0)
	{
		ElevatorInfo info;
		info.elevator = elevator;
		info.currentLoad = 0;
		m_elevatorInfo.insert({elevator->GetId(), info});
		PRINTMSG("Adding elevator " << elevator->GetId() << " to map.");
	}
}

Floor *ElevatorLogic::getInterfaceFloor(const Event &event)
{
	return ((Person *)event.GetEventHandler())->GetCurrentFloor();
}

bool ElevatorLogic::containsElevatorDestination(Elevator *elevator, Floor *floor)
{
	const auto &dst = m_elevatorInfo[elevator->GetId()].destinations;
	return std::find(dst.begin(), dst.end(), floor) != dst.end();
}

void ElevatorLogic::addElevatorDestination(Elevator *elevator, Floor *floor)
{
	std::vector<Floor *> &destinations = m_elevatorInfo[elevator->GetId()].destinations;

	if(!containsElevatorDestination(elevator, floor) && elevator->HasFloor(floor))
	{
		PRINTMSG("Added destination " << floor->GetId() << " to elevator " << elevator->GetId() << ".");

		//No destination currently
		if(destinations.size() == 0)
		{
			destinations.push_back(floor);
		}

		//Search for correct position to insert destination
		else
		{
			auto iter = destinations.begin();
			for(; iter != destinations.end(); ++iter)
			{
				Floor *floorIter = *iter;
				if(isFloorAbove(floor, floorIter)) continue;
				else { destinations.insert(iter, floor); break; }
			}

			//Arrived at the end of the destination list -> push back
			if(iter == destinations.end())
			{
				destinations.push_back(floor);
			}
		}

		std::cout << "Elevator " << elevator->GetId() << " has destinations: ";
		for(auto iter = destinations.begin(); iter != destinations.end(); ++iter)
		{
			std::cout << (*iter)->GetId() << ", ";
		}
		std::cout << std::endl;
	}
}

void ElevatorLogic::removeElevatorDestination(Elevator *elevator, Floor *floor)
{
	auto &dst = m_elevatorInfo[elevator->GetId()].destinations;
	auto iter = std::find(dst.begin(), dst.end(), floor);
	if(iter != dst.end())
	{
		PRINTMSG("Removed destination " << (*iter)->GetId() << " from elevator " << elevator->GetId() << ".");
		dst.erase(iter);
	}
}

//Checks for the next destination this elevator should move to
//and initiate its movement towards the selected floor.
void ElevatorLogic::updateElevatorDestination(Elevator *elevator, Environment &environment)
{
	auto &info = m_elevatorInfo[elevator->GetId()];

	//Already has destinations enqueued
	if(info.destinations.size() >= 1)
	{
		const std::vector<Floor *> &destinations = info.destinations;
		Floor *dest = nullptr;

		//No direction
		switch(info.direction)
		{
			case ElevatorDirection::NONE:
			{
				//Select only destination in list
				//Else it would have a direction
				dest = destinations[0];
				break;
			}

			//Moving up
			case ElevatorDirection::UP:
			{
				//Look for floors above us
				for(auto iter = destinations.begin(); iter != destinations.end(); iter++)
				{
					Floor *floor = *iter;

					if(floor == elevator->GetCurrentFloor())
					{
						if(elevator->GetPosition() <= 0.5)
						{
							dest = floor;
							break;
						}
					}

					else if(isFloorAbove(floor, elevator->GetCurrentFloor()))
					{
						dest = floor;
						break;
					}
				}

				//None found above us
				if(dest == nullptr)
				{
					//Use highest floor below us
					dest = destinations.back();
				}

				break;
			}

			case ElevatorDirection::DOWN:
			{
				//Look for floors below us
				for(auto iter = destinations.rbegin(); iter != destinations.rend(); iter++)
				{
					Floor *floor = *iter;

					//Destination is our current floor
					if(floor == elevator->GetCurrentFloor())
					{
						//Check height inside floor
						if(elevator->GetPosition() >= 0.5)
						{
							dest = floor;
							break;
						}
					}

					else if(!isFloorAbove(floor, elevator->GetCurrentFloor())) //floor below us
					{
						dest = floor;
						break;
					}
				}

				//None found below us
				if(dest == nullptr)
				{
					//Use lowest floor above us
					dest = destinations[0];
				}

				break;
			}
		}

		info.currentDestination = dest;
		PRINTMSG("Selected destination for elevator " << elevator->GetId() << ": " << info.currentDestination->GetId() << ".");
	}

	//No destination currently
	else
	{
		info.currentDestination = nullptr;
		PRINTMSG("Selected destination for elevator " << elevator->GetId() << ": none.");
	}

	//Determined next destination
	if(info.currentDestination != nullptr)
	{
		Floor *destination = info.currentDestination;
		Floor *current = elevator->GetCurrentFloor();

		//Elevator working, not moving and doors closed
		if(!info.moving && info.closed && elevator->GetState() != Elevator::Malfunction)
		{
			//We need to go upwards
			if(isFloorAbove(destination, current))
			{
				info.direction = ElevatorDirection::UP;
			}

			//We need to go downwards
			else if(destination != current)
			{
				info.direction = ElevatorDirection::DOWN;
			}

			//We are inside the destination floor, check height
			else
			{
				double position = elevator->GetPosition();
				if(position < 0.5)
				{
					info.direction = ElevatorDirection::UP;
				}
				else
				{
					info.direction = ElevatorDirection::DOWN;
				}
			}

			//Start moving
			if(info.direction == ElevatorDirection::UP)
			{
				environment.SendEvent(EVENT_IN_ELEVATOR_UP, 0, this, elevator);
			}
			else
			{
				environment.SendEvent(EVENT_IN_ELEVATOR_DOWN, 0, this, elevator);
			}

			//Calculate the time we need to travel
			int duration = calculateTravelTime(elevator, elevator->GetCurrentFloor(), info.currentDestination);

			PRINTMSG("Calculated travel duration: " << duration);
			info.stopEventID = environment.SendEvent(EVENT_IN_ELEVATOR_STOP, duration, this, elevator);
			info.stopped = false;
			info.moving = true;
		}

		//If we are moving
		else if(info.moving && insideRange(elevator->GetState(), {Elevator::Up, Elevator::Down}))
		{
			//Cancel current stop event and fire new one to stop at intermediary floor
			environment.CancelEvent(info.stopEventID);

			//Calculate the time we need to travel (not taking into account position inside floor)
			int duration = calculateTravelTime(elevator, elevator->GetCurrentFloor(), info.currentDestination);

			PRINTMSG("Calculated travel duration: " << duration);
			info.stopEventID = environment.SendEvent(EVENT_IN_ELEVATOR_STOP, duration, this, elevator);
			info.stopped = false;
			info.moving = true;
		}
	}

	//No destination left and not broken
	else if(elevator->GetState() != Elevator::Malfunction)
	{
		PRINTMSG("Elevator " << elevator->GetId() << " has no destination.");
		info.direction = ElevatorDirection::NONE;
	}
}

bool ElevatorLogic::isOutdoorInterface(const Interface *interface)
{
	return (interface->GetLoadableCount() > 0 && interface->GetLoadable(0)->GetType() == LOADABLE_TYPE_ELEVATOR);
}

bool ElevatorLogic::isFloorAbove(Floor *a, Floor *b)
{
	if(a == nullptr || b == nullptr) return false;
	else return a->IsBelow(b);
}

int ElevatorLogic::calculateTravelTime(const Elevator *elevator, Floor *start, Floor *destination)
{
	//Resulting duration
	int duration = 0;
	double position = elevator->GetPosition();
	double floorHeight = (double)elevator->GetCurrentFloor()->GetHeight();
	double speed = (double)elevator->GetSpeed();
	auto &info = m_elevatorInfo[elevator->GetId()];

	//Travelling upwards -> subtract height already travelled inside current floor
	if(info.direction == ElevatorDirection::UP)
	{
		//In lower half of floor
		if(position <= 0.5)
		{
			duration += round((0.5 - position) * floorHeight / speed);
		}
		//In upper half of floor
		else
		{
			duration -= round((position - 0.5) * floorHeight / speed);
		}
	}

	//Travelling downwards -> add height already travelled inside current floor
	else if(info.direction == ElevatorDirection::DOWN)
	{
		//In lower half of floor
		if(elevator->GetPosition() <= 0.5)
		{
			//SOMETHING FUCKS UP HERE
			duration -= round((0.5 - position) * floorHeight / speed);
		}
		//In upper half of floor
		else
		{
			duration += round((position - 0.5) * floorHeight / speed);
		}
	}

	if(isFloorAbove(destination, start))
	{
		for(Floor *current = start; current != destination; current = current->GetAbove())
		{
			duration += current->GetHeight() / speed;
		}
	}
	else
	{
		for(Floor *current = start; current != destination; current = current->GetBelow())
		{
			duration += current->GetHeight() / speed;
		}
	}

	return duration;
}

bool ElevatorLogic::isElevatorOverloaded(const Elevator *elevator)
{
	return m_elevatorInfo[elevator->GetId()].currentLoad > elevator->GetMaxLoad();
}

void ElevatorLogic::increaseEnteringPersonCount(const Elevator *elevator)
{
	m_elevatorInfo[elevator->GetId()].enteringPersons++;
}

void ElevatorLogic::decreaseEnteringPersonCount(const Elevator *elevator)
{
	m_elevatorInfo[elevator->GetId()].enteringPersons--;
}
