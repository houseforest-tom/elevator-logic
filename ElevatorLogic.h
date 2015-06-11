/*
 * ElevatorLogic.h
 *
 *  Created on: 20.06.2014
 *      Author: STSJR
 */

#ifndef ELEVATORLOGIC_H_
#define ELEVATORLOGIC_H_

#define PRINTMSG(MSG) std::cout << "MESSAGE >>\t" << MSG << std::endl
#define PRINTERR(ERR) std::cerr << "ERROR >>\t" << ERR << std::endl

#include "EventHandler.h"

#include <list>
#include <map>
#include <set>
#include <vector>

class Elevator;
class Floor;
class Interface;

//Fallback test file to use if none specified on command line
static const std::string DEFAULT_TESTFILE 				= "tests/17.txt";

//Incoming events for interfaces
static const std::string EVENT_IN_INTERFACE_INTERACT 	= "Interface::Interact";

//Outgoing events from interfaces
static const std::string EVENT_OUT_INTERFACE_NOTIFY		= "Interface::Notify";

//Incoming events for elevators
static const std::string EVENT_IN_ELEVATOR_UP 			= "Elevator::Up";
static const std::string EVENT_IN_ELEVATOR_DOWN 		= "Elevator::Down";
static const std::string EVENT_IN_ELEVATOR_STOP 		= "Elevator::Stop";
static const std::string EVENT_IN_ELEVATOR_OPEN 		= "Elevator::Open";
static const std::string EVENT_IN_ELEVATOR_CLOSE 		= "Elevator::Close";
static const std::string EVENT_IN_ELEVATOR_BEEP 		= "Elevator::Beep";
static const std::string EVENT_IN_ELEVATOR_STOPBEEP 	= "Elevator::StopBeep";

//Outgoing events from elevators
static const std::string EVENT_OUT_ELEVATOR_MOVING 		= "Elevator::Moving";
static const std::string EVENT_OUT_ELEVATOR_STOPPED		= "Elevator::Stopped";
static const std::string EVENT_OUT_ELEVATOR_OPENING 	= "Elevator::Opening";
static const std::string EVENT_OUT_ELEVATOR_OPENED 		= "Elevator::Opened";
static const std::string EVENT_OUT_ELEVATOR_CLOSING 	= "Elevator::Closing";
static const std::string EVENT_OUT_ELEVATOR_CLOSED 		= "Elevator::Closed";
static const std::string EVENT_OUT_ELEVATOR_BEEPING 	= "Elevator::Beeping";
static const std::string EVENT_OUT_ELEVATOR_BEEPED 		= "Elevator::Beeped";
static const std::string EVENT_OUT_ELEVATOR_MALFUNCTION = "Elevator::Malfunction";
static const std::string EVENT_OUT_ELEVATOR_FIXED 		= "Elevator::Fixed";

//Outgoing events from persons
static const std::string EVENT_OUT_PERSON_ENTERING 		= "Person::Entering";
static const std::string EVENT_OUT_PERSON_ENTERED 		= "Person::Entered";
static const std::string EVENT_OUT_PERSON_EXITING 		= "Person::Exiting";
static const std::string EVENT_OUT_PERSON_EXITED 		= "Person::Exited";
static const std::string EVENT_OUT_PERSON_CANCELED 		= "Person::Canceled";

//Incoming events to up-down-buttons
static const std::string EVENT_IN_BUTTON_UP 			= "UpDownButton::Up";
static const std::string EVENT_IN_BUTTON_DOWN 			= "UpDownButton::Down";

//Outgoing events from up-down-buttons
static const std::string EVENT_OUT_BUTTON_DECIDE 		= "UpDownButton::Decide";

//Loadable type identifiers
static const std::string LOADABLE_TYPE_ELEVATOR 		= "Elevator";
static const std::string LOADABLE_TYPE_FLOOR	 		= "Floor";
static const std::string LOADABLE_TYPE_INTERFACE 		= "Interface";
static const std::string LOADABLE_TYPE_PERSON	 		= "Person";

class ElevatorLogic: public EventHandler
{
private:

	//Direction the elevator is currently heading towards
	enum class ElevatorDirection : unsigned char
	{
		UP,
		DOWN,
		NONE
	};

	//Holds additional information about each elevator
	//This includes a list of its destinations, its current load
	//Whether it is moving / has stopped, etc.
	struct ElevatorInfo
	{
		//Pointer to actual elevator object
		Elevator *elevator = nullptr;

		//Current sum of person weights inside the elevator
		int currentLoad = 0;

		//Destination list and current selection
		std::vector<Floor *> destinations = { };
		Floor *currentDestination = nullptr;

		//Status flags
		bool closed = true;
		bool moving = false;
		bool stopped = false;
		bool beeping = false;

		//ID of stop event sent when the last movement was initiated
		int stopEventID = -1;

		//ID of close event sent after opening doors /
		int closeEventID = -1;

		//Number of persons that started entering this elevator
		int enteringPersons = 0;

		//Current direction
		ElevatorDirection direction = ElevatorDirection::NONE;
	};

	/**
	 * Contains all elevator information addressable by the
	 * corresponding elevator's ID.
	 */
	std::map<int, ElevatorInfo> m_elevatorInfo;

	/**
	 * TODO:
	 */
	void handleInterfaceNotify(Environment &environment, const Event &event);

	/**
	 * TODO:
	 */
	void handleElevatorMoving(Environment &environment, const Event &event);

	/**
	 * TODO:
	 */
	void handleElevatorStopped(Environment &environment, const Event &event);

	/**
	 * TODO:
	 */
	void handleElevatorOpening(Environment &environment, const Event &event);

	/**
	 * TODO:
	 */
	void handleElevatorOpened(Environment &environment, const Event &event);

	/**
	 * TODO:
	 */
	void handleElevatorClosing(Environment &environment, const Event &event);

	/**
	 * TODO:
	 */
	void handleElevatorClosed(Environment &environment, const Event &event);

	/**
	 * TODO:
	 */
	void handleElevatorBeeping(Environment &environment, const Event &event);

	/**
	 * TODO:
	 */
	void handleElevatorBeeped(Environment &environment, const Event &event);

	/**
	 * TODO:
	 */
	void handleElevatorMalfunction(Environment &environment, const Event &event);

	/**
	 * TODO:
	 */
	void handleElevatorFixed(Environment &environment, const Event &event);

	/**
	 * TODO:
	 */
	void handlePersonEntering(Environment &environment, const Event &event);

	/**
	 * TODO:
	 */
	void handlePersonEntered(Environment &environment, const Event &event);

	/**
	 * TODO:
	 */
	void handlePersonExiting(Environment &environment, const Event &event);

	/**
	 * TODO:
	 */
	void handlePersonExited(Environment &environment, const Event &event);

	/**
	 * TODO:
	 */
	void handlePersonCanceled(Environment &environment, const Event &event);

	/**
	 * TODO:
	 */
	void handleButtonDecide(Environment &environment, const Event &event);

public:
	/**
	 * Construct a new elevator logic object.
	 */
	ElevatorLogic(void);

	/**
	 * Initialize the elevator logic.
	 *
	 * @param environment
	 * The environment to use.
	 */
	void Initialize(Environment &environment);

	/**
	 * Virtual dtor.
	 */
	virtual ~ElevatorLogic(void);

private:
	void addElevator(Elevator *elevator);
	bool containsElevatorDestination(Elevator *elevator, Floor *floor);
	void addElevatorDestination(Elevator *elevator, Floor *floor);
	void removeElevatorDestination(Elevator *elevator, Floor *floor);
	bool isFloorAbove(Floor *a, Floor *b);
	void updateElevatorDestination(Elevator *elevator, Environment &environment);
	bool isOutdoorInterface(const Interface *interface);
	int calculateTravelTime(const Elevator *elevator, Floor *start, Floor *destination);
	bool isElevatorOverloaded(const Elevator *elevator);
	void increaseEnteringPersonCount(const Elevator *elevator);
	void decreaseEnteringPersonCount(const Elevator *elevator);
	Floor *getInterfaceFloor(const Event &event);

	template<typename T>
	bool insideRange(const T& value, std::vector<T> range)
	{
		for(auto iter = range.begin(); iter != range.end(); iter++)
		{
			if(*iter == value)
				return true;
		}

		return false;
	}
};

#endif /* ELEVATORLOGIC_H_ */
