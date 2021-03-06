#ifndef BOOK_STATEIDENTIFIERS_HPP
#define BOOK_STATEIDENTIFIERS_HPP


namespace States
{
	enum ID
	{
		None,
		Title,
		Menu,
		Game,
		Loading,
		Pause,
		NetworkPause,
		Settings,
		GameOver,
		Spectate,
		MissionSuccess,
		HostGame,
		JoinGame,
		HostLobby,
		JoinLobby
	};
}

#endif // BOOK_STATEIDENTIFIERS_HPP
