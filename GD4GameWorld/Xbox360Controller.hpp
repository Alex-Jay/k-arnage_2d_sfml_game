#pragma once
#include <SFML/System/Vector2.hpp>

class Xbox360Controller
{
public:
	Xbox360Controller(int controllerIndex);

	//static Xbox360Controller GetController();
	bool IsConnected();

	// Buttons
	bool A();
	bool B();
	bool X();
	bool Y();
	bool RB();					// Right bumber
	bool LB();					// Left bumper
	bool L3();					// Left thumbstick click
	bool R3();					// Right thumbstick click
	bool Start();
	bool Back();
	bool XboxButton();
	bool LT();					// Left trigger
	bool RT();					// Right trigger

	// Axis & LT / RT & Analog sticks

	float TValue();				// Left and Right Trigger value
	sf::Vector2f LAnalog();		// X, Y coords of Left analog stick
	sf::Vector2f RAnalog();		// X, Y coords of Right analog stick
	sf::Vector2f DPad();		// X, Y coords of D-pad

	// D-pad & left thumbstick motions

	bool Up();
	bool Down();
	bool Left();
	bool Right();

	bool RAnalogLeft();
	bool RAnalogRight();

	int GetControllerIndex() const;
	float GetTriggerThreshold() const;
	float GetThumbstickThreshold() const;

	void SetTriggerThreshold(float threshold);
	void SetThumbstickThreshold(float threshold);

private:
	int mControllerIndex;
	float mTriggerThreshold;
	float mThumbstickThreshold;
};