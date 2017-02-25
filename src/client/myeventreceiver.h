#ifndef MYEVENTRECEIVER_H
#define MYEVENTRECEIVER_H

#include "irrlichttypes_extrabloated.h"
#include "../keycode.h"
#include "../game.h"

class JoystickController;
class MyEventReceiver : public IEventReceiver
{
public:
	// This is the one method that we have to implement
	virtual bool OnEvent(const SEvent& event);

	bool IsKeyDown(const KeyPress &keyCode) const
	{
		return keyIsDown[keyCode];
	}

	// Checks whether a key was down and resets the state
	bool WasKeyDown(const KeyPress &keyCode)
	{
		bool b = keyWasDown[keyCode];
		if (b)
			keyWasDown.unset(keyCode);
		return b;
	}

	void listenForKey(const KeyPress &keyCode)
	{
		keysListenedFor.set(keyCode);
	}
	void dontListenForKeys()
	{
		keysListenedFor.clear();
	}

	s32 getMouseWheel()
	{
		s32 a = mouse_wheel;
		mouse_wheel = 0;
		return a;
	}

	void clearInput();

	MyEventReceiver()
	{
		clearInput();
#ifdef HAVE_TOUCHSCREENGUI
		m_touchscreengui = NULL;
#endif
	}

	bool leftclicked;
	bool rightclicked;
	bool leftreleased;
	bool rightreleased;

	bool left_active;
	bool middle_active;
	bool right_active;

	s32 mouse_wheel;

	JoystickController *joystick;
	int joystick_id = 0;

#ifdef HAVE_TOUCHSCREENGUI
	TouchScreenGUI* m_touchscreengui;
#endif

private:
	// The current state of keys
	KeyList keyIsDown;
	// Whether a key has been pressed or not
	KeyList keyWasDown;
	// List of keys we listen for
	// TODO perhaps the type of this is not really
	// performant as KeyList is designed for few but
	// often changing keys, and keysListenedFor is expected
	// to change seldomly but contain lots of keys.
	KeyList keysListenedFor;
};

#endif
