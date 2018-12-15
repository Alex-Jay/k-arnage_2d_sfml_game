#include "Component.hpp"

namespace GUI
{
	GUI::Component::Component()
		: mIsSelected(false), mIsActive(false)
	{
	}

	GUI::Component::~Component()
	{
	}


	bool GUI::Component::isSelected() const
	{
		return mIsSelected;
	}

	void GUI::Component::select()
	{
		mIsSelected = true;
	}

	void GUI::Component::deselect()
	{
		mIsSelected = false;
	}

	bool GUI::Component::isActive() const
	{
		return mIsActive;
	}

	void GUI::Component::activate()
	{
		mIsActive = true;
	}

	void GUI::Component::deactivate()
	{
		mIsActive = false;
	}
}
