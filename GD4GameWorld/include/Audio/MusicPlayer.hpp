#pragma once
#include "ResourceHolder.hpp"
#include "ResourceIdentifiers.hpp"

#include "SFML/System/NonCopyable.hpp"
#include "SFML/Audio/Music.hpp"

#include <map>
#include <string>

class MusicPlayer : private sf::NonCopyable
{
public:
	MusicPlayer();

	void play(MusicIDs theme);
	void stop();

	void setPaused(bool paused);
	void setVolume(float volume);

private:
	sf::Music mMusic;
	std::map<MusicIDs, std::string> mFilenames;
	float mVolume;
};