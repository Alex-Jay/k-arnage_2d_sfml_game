#ifndef BOOK_STATESTACK_HPP
#define BOOK_STATESTACK_HPP

#include "State/State.hpp"
#include "State/StateIdentifiers.hpp"
#include "Structural/ResourceIdentifiers.hpp"

#include <SFML/System/NonCopyable.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/Network/TcpSocket.hpp>

#include <vector>
#include <utility>
#include <functional>
#include <map>

namespace sf {
	class Event;
	class RenderWindow;
}

class StateStack : private sf::NonCopyable {
public:
	enum Action {
		Push,
		Pop,
		Clear,
	};

public:
	explicit StateStack(State::Context context);

	template <typename T>
	void registerState(States::ID stateID);
	template <typename T, typename Param1>
	void registerState(States::ID stateID, Param1 arg1);

	template<typename T, typename Param1, typename Param2>
	void registerState(States::ID stateID, Param1 arg1, Param2 arg2);



	void update(sf::Time dt);
	void draw();
	void handleEvent(const sf::Event& event);

	void pushState(States::ID stateID);
	void popState();
	void clearStates();

	bool isEmpty() const;

private:
	State::Ptr createState(States::ID stateID);
	void applyPendingChanges();

private:
	struct PendingChange {
		explicit PendingChange(Action action, States::ID stateID = States::None);

		Action action;
		States::ID stateID;
	};

private:
	std::vector<State::Ptr> mStack;
	std::vector<PendingChange> mPendingList;

	State::Context mContext;
	std::map<States::ID, std::function<State::Ptr()> > mFactories;
};

template <typename T>
void StateStack::registerState(States::ID stateID)
{
	mFactories[stateID] = [this]() {
		return State::Ptr(new T(*this, mContext));
	};
}

template <typename T, typename Param1>
void StateStack::registerState(States::ID stateID, Param1 arg1)
{
	mFactories[stateID] = [this, arg1]() {
		return State::Ptr(new T(*this, mContext, arg1));
	};
}

template <typename T, typename Param1, typename Param2>
void StateStack::registerState(States::ID stateID, Param1 arg1, Param2 arg2)
{
	mFactories[stateID] = [this, arg1, arg2]() {
		return State::Ptr(new T(*this, mContext, arg1, arg2));
	};
}

#endif // BOOK_STATESTACK_HPP
