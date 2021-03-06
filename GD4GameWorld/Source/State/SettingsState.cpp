#include "State/SettingsState.hpp"
#include "Structural/Utility.hpp"
#include "Structural/ResourceHolder.hpp"

#include <SFML/Graphics/RenderWindow.hpp>


SettingsState::SettingsState(StateStack& stack, Context context)
: State(stack, context)
, mGUIContainer()
{
	mBackgroundSprite.setTexture(context.textures->get(Textures::TitleScreen));
	
	// Build key binding buttons and labels
	addButtonLabel(PlayerAction::MoveLeft, 0, 0, "Move Left", context);
	addButtonLabel(PlayerAction::MoveRight, 0, 1, "Move Right", context);
	addButtonLabel(PlayerAction::MoveUp, 0, 2, "Move Up", context);
	addButtonLabel(PlayerAction::MoveDown, 0, 3, "Move Down", context);
	addButtonLabel(PlayerAction::Fire, 0, 4, "Fire", context);
	addButtonLabel(PlayerAction::StartGrenade, 0, 5, "Granade", context);
	updateLabels();

	auto backButton = std::make_shared<GUI::Button>(context);
	backButton->setPosition(400.f, 620.f);
	backButton->setText("Back");
	backButton->setCallback(std::bind(&SettingsState::requestStackPop, this));

	mGUIContainer.pack(backButton);
}

void SettingsState::draw()
{
	sf::RenderWindow& window = *getContext().window;

	window.draw(mBackgroundSprite);
	window.draw(mGUIContainer);
}

bool SettingsState::update(sf::Time)
{
	return true;
}

bool SettingsState::handleEvent(const sf::Event& event)
{
	bool isKeyBinding = false;

	// Iterate through all key binding buttons to see if they are being pressed, waiting for the user to enter a key
	for (std::size_t i = 0; i < PlayerAction::Count; ++i)
	{
		if (mBindingButtons[i]->isActive())
		{
			isKeyBinding = true;
			if (event.type == sf::Event::KeyReleased)
			{
				// Player 1
				if (i < PlayerAction::Count)
					getContext().keys->assignKey(static_cast<PlayerAction::Type>(i), event.key.code);
				
				//// Player 2
				//else
				//	getContext().keys2->assignKey(static_cast<PlayerAction::Type>(i - PlayerAction::Count), event.key.code);

				if (static_cast<PlayerAction::Type>(i) == PlayerAction::Type::StartGrenade)
					getContext().keys->assignReleaseKey(PlayerAction::Type::LaunchGrenade, event.key.code);

				mBindingButtons[i]->deactivate();
			}
			break;
		}
	}

	// If pressed button changed key bindings, update labels; otherwise consider other buttons in container
	if (isKeyBinding)
		updateLabels();
	else
		mGUIContainer.handleEvent(event);

	return false;
}

void SettingsState::updateLabels()
{
	for (std::size_t i = 0; i < PlayerAction::Count; ++i)
	{
		auto action = static_cast<PlayerAction::Type>(i);

		// Get key bindings
		sf::Keyboard::Key key1 = getContext().keys->getAssignedKey(action);

		// Assign key strings to labels
		mBindingLabels[i]->setText(toString(key1));
	}
}

void SettingsState::addButtonLabel(std::size_t index, std::size_t x, std::size_t y, const std::string& text, Context context)
{
	// For x==0, start at index 0, otherwise start at half of array
	index += PlayerAction::Count * x;

	mBindingButtons[index] = std::make_shared<GUI::Button>(context);
	mBindingButtons[index]->setPosition(400.f*x + 400.f, 50.f*y + 200.f);
	mBindingButtons[index]->setText(text);
	mBindingButtons[index]->setToggle(true);

	mBindingLabels[index] = std::make_shared<GUI::Label>("", *context.fonts);
	mBindingLabels[index]->setPosition(400.f*x + 605.f, 50.f*y + 215.f);

	mGUIContainer.pack(mBindingButtons[index]);
	mGUIContainer.pack(mBindingLabels[index]);
}