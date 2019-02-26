#include "Application.hpp"
#include "State.hpp"
#include "StateIdentifiers.hpp"
#include "TitleState.hpp"
#include "MenuState.hpp"
#include "GameState.hpp"
#include "PauseState.hpp"
#include "SettingsState.hpp"
#include "GameOverState.hpp"
#include "ScoreBoardState.hpp"

const sf::Time Application::TimePerFrame = sf::seconds(1.f / 60.f);

Application::Application()
	: mWindow(sf::VideoMode(1024, 768), "K-ARNAGE", sf::Style::Close)
	, mPlayerOne(0)
	//, mPlayerTwo(1)
	, mStateStack(State::Context(mWindow, mTextures, mFonts, mPlayerOne, /*mPlayerTwo,*/ mMusic, mSounds))
	, mStatisticsNumFrames(0)
{	
	mWindow.setVerticalSyncEnabled(true);
	mWindow.setKeyRepeatEnabled(false);

	mFonts.load(FontIDs::Main, "Media/Fonts/Sansation.ttf");

	mTextures.load(TextureIDs::TitleScreen, "Media/Textures/TitleScreen.png");
	mTextures.load(TextureIDs::Buttons, "Media/Textures/Buttons.png");

	mStatisticsText.setFont(mFonts.get(FontIDs::Main));
	mStatisticsText.setPosition(5.f, 5.f);
	mStatisticsText.setCharacterSize(20);

	registerStates();
	mStateStack.pushState(StateIDs::Title);

	mMusic.setVolume(25.f);
	//mMusic.setVolume(10.f);
}

void Application::run()
{
	sf::Clock clock;
	sf::Time timeSinceLastUpdate = sf::Time::Zero;

	while (mWindow.isOpen())
	{
		sf::Time elapsedTime = clock.restart();
		timeSinceLastUpdate += elapsedTime;
		while (timeSinceLastUpdate > TimePerFrame)
		{
			timeSinceLastUpdate -= TimePerFrame;
			processInput();
			update(TimePerFrame);
			if (mStateStack.isEmpty())
			{
				mWindow.close();
			}
		}
		updateStatistics(elapsedTime);
		render();
	}
}

void Application::processInput()
{
	sf::Event event;
	while (mWindow.pollEvent(event))
	{
		mStateStack.handleEvent(event);

		if (event.type == sf::Event::Closed)
		{
			mWindow.close();
		}
	}
}

void Application::update(sf::Time dt)
{
	mStateStack.update(dt);
}

void Application::render()
{
	mWindow.clear(sf::Color(0, 97, 255, 255));
	mStateStack.draw();
	mWindow.setView(mWindow.getDefaultView());
	mWindow.draw(mStatisticsText);
	mWindow.display();
}

void Application::updateStatistics(sf::Time elapsedTime)
{
	mStatisticsUpdateTime += elapsedTime;
	mStatisticsNumFrames += 1;
	if (mStatisticsUpdateTime >= sf::seconds(1.0f))
	{
		mStatisticsText.setString("FPS: " + std::to_string(mStatisticsNumFrames));

		mStatisticsUpdateTime -= sf::seconds(1.0f);
		mStatisticsNumFrames = 0;
	}
}

void Application::registerStates()
{
	mStateStack.registerState<TitleState>(StateIDs::Title);
	mStateStack.registerState<MenuState>(StateIDs::Menu);
	mStateStack.registerState<GameState>(StateIDs::Game);
	mStateStack.registerState<PauseState>(StateIDs::Pause);
	mStateStack.registerState<SettingsState>(StateIDs::Settings);
	mStateStack.registerState<GameOverState>(StateIDs::GameOver);
	mStateStack.registerState<ScoreBoardState>(StateIDs::ScoreBoard);
}