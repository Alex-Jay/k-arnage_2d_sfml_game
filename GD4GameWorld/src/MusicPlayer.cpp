#include "MusicPlayer.hpp"

//In SFML volume = 0 is mute and = 100 is max volume
MusicPlayer::MusicPlayer()
	:mMusic(), mFilenames(), mVolume(1.f)
{
	mFilenames[MusicIDs::MenuTheme] = "Media/Music/MenuTheme.ogg";
	mFilenames[MusicIDs::MissionTheme] = "Media/Music/MissionTheme.ogg";
}

void MusicPlayer::play(MusicIDs theme)
{
	std::string filename = mFilenames[theme];
	if (!mMusic.openFromFile(filename))
	{
		throw std::runtime_error("Music " + filename + " could not be opened");
	}
	mMusic.setVolume(mVolume);
	mMusic.setLoop(true);
	mMusic.play();
}

void MusicPlayer::stop()
{
	mMusic.stop();
}

void MusicPlayer::setPaused(bool paused)
{
	if (paused)
	{
		mMusic.pause();
	}
	else
	{
		mMusic.play();
	}
}

void MusicPlayer::setVolume(float volume)
{
	mVolume = volume;
}
