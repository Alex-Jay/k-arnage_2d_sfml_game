#include "Xbox360Controller.hpp"

#include <SFML/Window/Joystick.hpp>

Xbox360Controller::Xbox360Controller(int controllerIndex)
	: mControllerIndex(controllerIndex), mTriggerThreshold(70.f), mThumbstickThreshold(50.f)
{
}

//Xbox360Controller Xbox360Controller::GetController()
//{
//	for (unsigned int i = 0; i < sf::Joystick::Count; i++)
//	{
//		if (sf::Joystick::isConnected(i))
//		{
//			Xbox360Controller controller(i);
//			return controller;
//		}
//	}
//
//	// Return default controller
//	Xbox360Controller controller(0);
//	return controller;
//}

bool Xbox360Controller::IsConnected()
{
	if (mControllerIndex < 0 || mControllerIndex > sf::Joystick::Count)
	{
		return false;
	}
	else
	{
		return sf::Joystick::isConnected(mControllerIndex);
	}
}

bool Xbox360Controller::A()
{
	return sf::Joystick::isButtonPressed(mControllerIndex, 0);
}

bool Xbox360Controller::B()
{
	return sf::Joystick::isButtonPressed(mControllerIndex, 1);
}

bool Xbox360Controller::X()
{
	return sf::Joystick::isButtonPressed(mControllerIndex, 2);
}

bool Xbox360Controller::Y()
{
	return sf::Joystick::isButtonPressed(mControllerIndex, 3);
}

bool Xbox360Controller::LB()
{
	return sf::Joystick::isButtonPressed(mControllerIndex, 4);
}

bool Xbox360Controller::RB()
{
	return sf::Joystick::isButtonPressed(mControllerIndex, 5);
}

bool Xbox360Controller::Back()
{
	return sf::Joystick::isButtonPressed(mControllerIndex, 6);
}

bool Xbox360Controller::Start()
{
	return sf::Joystick::isButtonPressed(mControllerIndex, 7);
}

bool Xbox360Controller::L3()
{
	return sf::Joystick::isButtonPressed(mControllerIndex, 8);
}

bool Xbox360Controller::R3()
{
	return sf::Joystick::isButtonPressed(mControllerIndex, 9);
}

// Button id doesn't work, may be reserved for something else
bool Xbox360Controller::XboxButton()
{
	return sf::Joystick::isButtonPressed(mControllerIndex, 10);
}

bool Xbox360Controller::LT()
{
	return TValue() > mTriggerThreshold;
}

bool Xbox360Controller::RT()
{
	return TValue() < -mTriggerThreshold;
}

float Xbox360Controller::TValue()
{
	//<summary>Positive = Left Trigger. Negative = Right Trigger</summary>
	return sf::Joystick::getAxisPosition(mControllerIndex, sf::Joystick::Z);
}

sf::Vector2f Xbox360Controller::LAnalog()
{
	//<summary>Returns X,Y coordinates of the Left Analog Stick</summary>
	// Flip signs on Y-value to fit our game coordinate system
	/*
		Original Layout:
		LEFT - [X: -100, Y: 0]
		RIGHT - [X: -100, Y: 0]
		UP - [X: 0, Y: -100]
		DOWN - [X: 0, Y: 100]

		After Flipped Signs:
		LEFT - [X: -100, Y: 0]
		RIGHT - [X: -100, Y: 0]
		UP - [X: 0, Y: 100]
		DOWN - [X: 0, Y: -100]
	*/
	return sf::Vector2f(sf::Joystick::getAxisPosition(mControllerIndex, sf::Joystick::X), -sf::Joystick::getAxisPosition(mControllerIndex, sf::Joystick::Y));
}

sf::Vector2f Xbox360Controller::RAnalog()
{
	//<summary>Returns X,Y coordinates of the Right Analog Stick</summary>
	return sf::Vector2f(sf::Joystick::getAxisPosition(mControllerIndex, sf::Joystick::U), sf::Joystick::getAxisPosition(mControllerIndex, sf::Joystick::R));
}

sf::Vector2f Xbox360Controller::DPad()
{
	//<summary>Returns X,Y coordinates of the D-pad</summary>
	// D-pad doesn't need a sign flip on Y-axis, it's originally negative
	/*
		Original Layout:
		LEFT - [X: -100, Y: 0]
		RIGHT - [X: -100, Y: 0]
		UP - [X: 0, Y: 100]
		DOWN - [X: 0, Y: -100]
	*/
	return sf::Vector2f(sf::Joystick::getAxisPosition(mControllerIndex, sf::Joystick::PovX), sf::Joystick::getAxisPosition(mControllerIndex, sf::Joystick::PovY));
}

bool Xbox360Controller::Up()
{
	return DPad().y > mThumbstickThreshold || LAnalog().y > mThumbstickThreshold;
}

bool Xbox360Controller::Down()
{
	return DPad().y < -mThumbstickThreshold || LAnalog().y < -mThumbstickThreshold;
}

bool Xbox360Controller::Left()
{
	return DPad().x < -mThumbstickThreshold || LAnalog().x < -mThumbstickThreshold;
}

bool Xbox360Controller::Right()
{
	return DPad().x > mThumbstickThreshold || LAnalog().x > mThumbstickThreshold;
}

bool Xbox360Controller::RAnalogLeft()
{
	return RAnalog().x < -mThumbstickThreshold;
}

bool Xbox360Controller::RAnalogRight()
{
	return RAnalog().x > mThumbstickThreshold;
}

// ------------------ MUTATORS ----------------------------

int Xbox360Controller::GetControllerIndex() const
{
	return mControllerIndex;
}

float Xbox360Controller::GetTriggerThreshold() const
{
	return mTriggerThreshold;
}

float Xbox360Controller::GetThumbstickThreshold() const
{
	return mThumbstickThreshold;
}

void Xbox360Controller::SetTriggerThreshold(float threshold)
{
	mTriggerThreshold = threshold;
}

void Xbox360Controller::SetThumbstickThreshold(float threshold)
{
	mThumbstickThreshold = threshold;
}
