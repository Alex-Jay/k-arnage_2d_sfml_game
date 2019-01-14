#include "SettingsState.hpp"
#include "Utility.hpp"
#include "ResourceHolder.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <iostream>

SettingsState::SettingsState(StateStack& stack, Context context)
	: State(stack, context)
{
	mBackgroundSprite.setTexture(context.textures->get(TextureIDs::TitleScreen));

	// Build key binding buttons and labels
	//addButtonLabel(Player::Action::MoveLeft, 150.f, "Move Left", context);
	//addButtonLabel(Player::Action::MoveRight, 200.f, "Move Right", context);
	//addButtonLabel(Player::Action::MoveUp, 250.f, "Move Up", context);
	//addButtonLabel(Player::Action::MoveDown, 300.f, "Move Down", context);
	//addButtonLabel(Player::Action::RotateLeft, 350.f, "Rotate Left", context);
	//addButtonLabel(Player::Action::RotateRight, 400.f, "Rotate Right", context);
	//addButtonLabel(Player::Action::Fire, 450.f, "Fire", context);
	//addButtonLabel(Player::Action::StartGrenade, 500.f, "Grenade", context);

	// Build key binding buttons and labels
	for (std::size_t x = 0; x < 2; ++x)
	{
		addButtonLabel((int)Player::Action::MoveLeft,		x, 0, "Move Left",		context);
		addButtonLabel((int)Player::Action::MoveRight,		x, 1, "Move Right",		context);
		addButtonLabel((int)Player::Action::MoveUp,			x, 2, "Move Up",		context);
		addButtonLabel((int)Player::Action::MoveDown,		x, 3, "Move Down",		context);
		addButtonLabel((int)Player::Action::RotateLeft,		x, 4, "Rotate Left",	context);
		addButtonLabel((int)Player::Action::RotateRight,	x, 5, "Rotate Right",	context);
		addButtonLabel((int)Player::Action::Fire,			x, 6, "Fire",			context);
		addButtonLabel((int)Player::Action::StartGrenade,	x, 7, "Grenade",		context);
	}

	updateLabels();

	auto backButton = std::make_shared<GUI::Button>(context);
	//backButton->setPosition(80.f, 575.f);
	backButton->setPosition(80.f, 620.f);
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
	for (std::size_t index = 0; index < 2 * static_cast<int>(Player::Action::ActionCount); ++index)
	{
		if (mBindingButtons[index]->isActive())
		{
			isKeyBinding = true;
			if (event.type == sf::Event::KeyReleased && !isReservedKey(event))
			{
				// Player One
				if (index < static_cast<size_t>(Player::Action::ActionCount))
				{
					getContext().playerManager->getPlayerOne()->assignKey(static_cast<Player::Action>(index), event.key.code);
				}
				// Player Two
				else
				{
					getContext().playerManager->getPlayerTwo()->assignKey(static_cast<Player::Action>(index - static_cast<size_t>(Player::Action::ActionCount)), event.key.code);
				}

				if (static_cast<Player::Action>(index) == Player::Action::StartGrenade)
				{
					getContext().playerManager->getPlayerOne()->assignReleaseKey(Player::Action::LaunchGrenade, event.key.code);
				}

				mBindingButtons[index]->deactivate();
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

// Alex - Test if the key pressed was a reserved key
bool SettingsState::isReservedKey(sf::Event event)
{
	int reservedKeyCodes[] = {
		sf::Keyboard::Enter,
		sf::Keyboard::Space,
		sf::Keyboard::Escape,
		sf::Keyboard::LSystem,
		sf::Keyboard::RSystem,
		sf::Keyboard::Tilde
	};

	for (int keyCode : reservedKeyCodes)
	{
		if (keyCode == event.key.code)
		{
			return true;
		}
	}

	return false;
}

void SettingsState::updateLabels()
{
	//Player& player = *getContext().playerManager->getPlayerOne();
	for (std::size_t i = 0; i < static_cast<size_t>(Player::Action::ActionCount); ++i)
	{
		Player::Action action = static_cast<Player::Action>(i);

		//Get key from both players
		sf::Keyboard::Key key1 = getContext().playerManager->getPlayerOne()->getAssignedKey(action);
		sf::Keyboard::Key key2 = getContext().playerManager->getPlayerTwo()->getAssignedKey(action);
	
		// Assign both strings to labels
		mBindingLabels[i]->setText(toString(key1));
		mBindingLabels[i + static_cast<int>(Player::Action::ActionCount)]->setText(toString(key2));
	}

	//for (std::size_t i = 0; i < static_cast<int>(Player::Action::ActionCount); ++i)
	//{
	//	sf::Keyboard::Key key = player.getAssignedKey(static_cast<Player::Action>(i));
	//	mBindingLabels[i]->setText(toString(key));
	//}
}


//void SettingsState::addButtonLabel(Player::Action action, float x, float y, const std::string& text, Context context)
void SettingsState::addButtonLabel(int index, float x, float y, const std::string& text, Context context)
{
	// For x==0, start at index 0, otherwise start at half of array
	index += static_cast<int>(Player::Action::ActionCount) * x;
	
	mBindingButtons[index] = std::make_shared<GUI::Button>(context);
	mBindingButtons[index]->setPosition(400.f*x + 80.f, 50.f*y + 100.f);
	mBindingButtons[index]->setText(text);
	mBindingButtons[index]->setToggle(true);

	mBindingLabels[index] = std::make_shared<GUI::Label>("", *context.fonts);
	mBindingLabels[index]->setPosition(400.f*x + 300.f, 50.f*y + 115.f);

	mGUIContainer.pack(mBindingButtons[index]);
	mGUIContainer.pack(mBindingLabels[index]);

	//mBindingButtons[static_cast<int>(action)] = std::make_shared<GUI::Button>(context);
	//mBindingButtons[static_cast<int>(action)]->setPosition(80.f, y);
	//mBindingButtons[static_cast<int>(action)]->setText(text);
	//mBindingButtons[static_cast<int>(action)]->setToggle(true);

	//mBindingLabels[static_cast<int>(action)] = std::make_shared<GUI::Label>("", *context.fonts);
	//mBindingLabels[static_cast<int>(action)]->setPosition(300.f, y + 15.f);

	//mGUIContainer.pack(mBindingButtons[static_cast<int>(action)]);
	//mGUIContainer.pack(mBindingLabels[static_cast<int>(action)]);
}
