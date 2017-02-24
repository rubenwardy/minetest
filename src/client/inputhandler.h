/*
Minetest
Copyright (C) 2010-2013 celeron55, Perttu Ahola <celeron55@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include "irrlichttypes_extrabloated.h"
#include "joystick_controller.h"
#include "../keycode.h"
#include "../game.h"
#include "../mainmenumanager.h"
#include "../util/basic_macros.h"
#include "../util/numeric.h"

class MyEventReceiver : public IEventReceiver
{
public:
	// This is the one method that we have to implement
	virtual bool OnEvent(const SEvent& event)
	{
		/*
			React to nothing here if a menu is active
		*/
		if (noMenuActive() == false) {
#ifdef HAVE_TOUCHSCREENGUI
			if (m_touchscreengui != 0) {
				m_touchscreengui->Toggle(false);
			}
#endif
			return g_menumgr.preprocessEvent(event);
		}

		switch (event.EventType) {

		// Remember whether each key is down or up
		case irr::EET_KEY_INPUT_EVENT: {
			const KeyPress &keyCode = event.KeyInput;
			if (keysListenedFor[keyCode]) {
				if (event.KeyInput.PressedDown) {
					keyIsDown.set(keyCode);
					keyWasDown.set(keyCode);
				} else {
					keyIsDown.unset(keyCode);
				}
				return true;
			}
			break;
		}

#ifdef HAVE_TOUCHSCREENGUI
		// case of touchscreengui we have to handle different events
		case irr::EET_TOUCH_INPUT_EVENT: {
			if (m_touchscreengui != 0) {
				m_touchscreengui->translateEvent(event);
				return true;
			}
			break;
		}
#endif

		case irr::EET_JOYSTICK_INPUT_EVENT: {
			if (event.JoystickEvent.Joystick == joystick_id)
				return joystick->handleEvent(event.JoystickEvent);

			break;
		}

		case irr::EET_MOUSE_INPUT_EVENT: {
			if (noMenuActive() == false) {
				left_active = false;
				middle_active = false;
				right_active = false;
			} else {
				left_active = event.MouseInput.isLeftPressed();
				middle_active = event.MouseInput.isMiddlePressed();
				right_active = event.MouseInput.isRightPressed();

				if (event.MouseInput.Event == EMIE_LMOUSE_PRESSED_DOWN) {
					leftclicked = true;
				}
				if (event.MouseInput.Event == EMIE_RMOUSE_PRESSED_DOWN) {
					rightclicked = true;
				}
				if (event.MouseInput.Event == EMIE_LMOUSE_LEFT_UP) {
					leftreleased = true;
				}
				if (event.MouseInput.Event == EMIE_RMOUSE_LEFT_UP) {
					rightreleased = true;
				}
				if (event.MouseInput.Event == EMIE_MOUSE_WHEEL) {
					mouse_wheel += event.MouseInput.Wheel;
				}
			}
			break;
		}

		case irr::EET_LOG_TEXT_EVENT: {
			static const LogLevel irr_loglev_conv[] = {
				LL_VERBOSE, // ELL_DEBUG
				LL_INFO,    // ELL_INFORMATION
				LL_WARNING, // ELL_WARNING
				LL_ERROR,   // ELL_ERROR
				LL_NONE,    // ELL_NONE
			};
			assert(event.LogEvent.Level < ARRLEN(irr_loglev_conv));
			g_logger.log(irr_loglev_conv[event.LogEvent.Level],
				std::string("Irrlicht: ") + (const char*) event.LogEvent.Text);

			return true;
		} }

		// Return false to keep processing results
		return false;
	}

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

	void clearInput()
	{
		keyIsDown.clear();
		keyWasDown.clear();

		leftclicked = false;
		rightclicked = false;
		leftreleased = false;
		rightreleased = false;

		left_active = false;
		middle_active = false;
		right_active = false;

		mouse_wheel = 0;
	}

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



/* This is faster than using getKeySetting with the tradeoff that functions
 * using it must make sure that it's initialised before using it and there is
 * no error handling (for example bounds checking). This is really intended for
 * use only in the main running loop of the client (the_game()) where the faster
 * (up to 10x faster) key lookup is an asset. Other parts of the codebase
 * (e.g. formspecs) should continue using getKeySetting().
 */
class KeyPress;
class KeyCache {
public:
	KeyCache()
	{
		handler = NULL;
		populate();
		populate_nonchanging();
	}

	void populate();

	// Keys that are not settings dependent
	void populate_nonchanging();

	KeyPress key[KeyType::INTERNAL_ENUM_COUNT];
	InputHandler *handler;
};



/*
	Separated input handler
*/

class RealInputHandler : public InputHandler
{
	KeyCache keycache;
public:
	RealInputHandler(IrrlichtDevice *device, MyEventReceiver *receiver):
		m_device(device),
		m_receiver(receiver),
		m_mousepos(0,0)
	{
		m_receiver->joystick = &joystick;
		keycache.handler = this;
		keycache.populate();
	}
	virtual bool isKeyDown(GameKeyType keyCode)
	{
		return m_receiver->IsKeyDown(keycache.key[keyCode]);
	}

	// TODO: remove
	virtual bool wasKeyDown(const KeyPress &keyCode)
	{
		return m_receiver->WasKeyDown(keyCode);
	}

	virtual bool wasKeyDown(GameKeyType keyCode)
	{
		return m_receiver->WasKeyDown(keycache.key[keyCode]);
	}
	virtual void listenForKey(const KeyPress &keyCode)
	{
		m_receiver->listenForKey(keyCode);
	}
	virtual void dontListenForKeys()
	{
		m_receiver->dontListenForKeys();
	}
	virtual v2s32 getMousePos()
	{
		if (m_device->getCursorControl()) {
			return m_device->getCursorControl()->getPosition();
		}
		else {
			return m_mousepos;
		}
	}
	virtual void setMousePos(s32 x, s32 y)
	{
		if (m_device->getCursorControl()) {
			m_device->getCursorControl()->setPosition(x, y);
		}
		else {
			m_mousepos = v2s32(x,y);
		}
	}

	virtual bool getLeftState()
	{
		return m_receiver->left_active || joystick.isKeyDown(KeyType::MOUSE_L);
	}
	virtual bool getRightState()
	{
		return m_receiver->right_active || joystick.isKeyDown(KeyType::MOUSE_R);
	}

	virtual bool getLeftClicked()
	{
		return m_receiver->leftclicked || joystick.getWasKeyDown(KeyType::MOUSE_L);
	}
	virtual bool getRightClicked()
	{
		return m_receiver->rightclicked || joystick.getWasKeyDown(KeyType::MOUSE_R);
	}
	virtual void resetLeftClicked()
	{
		m_receiver->leftclicked = false;
	}
	virtual void resetRightClicked()
	{
		m_receiver->rightclicked = false;
	}

	virtual bool getLeftReleased()
	{
		return m_receiver->leftreleased || joystick.wasKeyReleased(KeyType::MOUSE_L);
	}
	virtual bool getRightReleased()
	{
		return m_receiver->rightreleased;
	}
	virtual void resetLeftReleased()
	{
		m_receiver->leftreleased = false;
	}
	virtual void resetRightReleased()
	{
		m_receiver->rightreleased = false;
	}

	virtual s32 getMouseWheel()
	{
		return m_receiver->getMouseWheel();
	}

	virtual void notifyKeyConfigChanged()
	{
		keycache.populate(); // update the cache with new settings
	}

	void clear()
	{
		joystick.clear();
		m_receiver->clearInput();
	}
private:
	IrrlichtDevice  *m_device;
	MyEventReceiver *m_receiver;
	v2s32           m_mousepos;
};

class RandomInputHandler : public InputHandler
{
	KeyCache keycache;
public:
	RandomInputHandler()
	{
		leftdown = false;
		rightdown = false;
		leftclicked = false;
		rightclicked = false;
		leftreleased = false;
		rightreleased = false;
		keydown.clear();

		keycache.handler = this;
		keycache.populate();
	}
	virtual bool isKeyDown(GameKeyType keyCode)
	{
		return keydown[keycache.key[keyCode]];
	}
	virtual bool wasKeyDown(GameKeyType keyCode)
	{
		return false;
	}
	virtual bool wasKeyDown(const KeyPress &keyCode)
	{
		return false;
	}
	virtual v2s32 getMousePos()
	{
		return mousepos;
	}
	virtual void setMousePos(s32 x, s32 y)
	{
		mousepos = v2s32(x, y);
	}

	virtual bool getLeftState()
	{
		return leftdown;
	}
	virtual bool getRightState()
	{
		return rightdown;
	}

	virtual bool getLeftClicked()
	{
		return leftclicked;
	}
	virtual bool getRightClicked()
	{
		return rightclicked;
	}
	virtual void resetLeftClicked()
	{
		leftclicked = false;
	}
	virtual void resetRightClicked()
	{
		rightclicked = false;
	}

	virtual bool getLeftReleased()
	{
		return leftreleased;
	}
	virtual bool getRightReleased()
	{
		return rightreleased;
	}
	virtual void resetLeftReleased()
	{
		leftreleased = false;
	}
	virtual void resetRightReleased()
	{
		rightreleased = false;
	}

	virtual s32 getMouseWheel()
	{
		return 0;
	}

	virtual void notifyKeyConfigChanged()
	{
		keycache.populate(); // update the cache with new settings
	}

	virtual void step(float dtime)
	{
		{
			static float counter1 = 0;
			counter1 -= dtime;
			if (counter1 < 0.0) {
				counter1 = 0.1 * Rand(1, 40);
				keydown.toggle(getKeySetting("keymap_jump"));
			}
		}
		{
			static float counter1 = 0;
			counter1 -= dtime;
			if (counter1 < 0.0) {
				counter1 = 0.1 * Rand(1, 40);
				keydown.toggle(getKeySetting("keymap_special1"));
			}
		}
		{
			static float counter1 = 0;
			counter1 -= dtime;
			if (counter1 < 0.0) {
				counter1 = 0.1 * Rand(1, 40);
				keydown.toggle(getKeySetting("keymap_forward"));
			}
		}
		{
			static float counter1 = 0;
			counter1 -= dtime;
			if (counter1 < 0.0) {
				counter1 = 0.1 * Rand(1, 40);
				keydown.toggle(getKeySetting("keymap_left"));
			}
		}
		{
			static float counter1 = 0;
			counter1 -= dtime;
			if (counter1 < 0.0) {
				counter1 = 0.1 * Rand(1, 20);
				mousespeed = v2s32(Rand(-20, 20), Rand(-15, 20));
			}
		}
		{
			static float counter1 = 0;
			counter1 -= dtime;
			if (counter1 < 0.0) {
				counter1 = 0.1 * Rand(1, 30);
				leftdown = !leftdown;
				if (leftdown)
					leftclicked = true;
				if (!leftdown)
					leftreleased = true;
			}
		}
		{
			static float counter1 = 0;
			counter1 -= dtime;
			if (counter1 < 0.0) {
				counter1 = 0.1 * Rand(1, 15);
				rightdown = !rightdown;
				if (rightdown)
					rightclicked = true;
				if (!rightdown)
					rightreleased = true;
			}
		}
		mousepos += mousespeed;
	}

	s32 Rand(s32 min, s32 max)
	{
		return (myrand()%(max-min+1))+min;
	}
private:
	KeyList keydown;
	v2s32 mousepos;
	v2s32 mousespeed;
	bool leftdown;
	bool rightdown;
	bool leftclicked;
	bool rightclicked;
	bool leftreleased;
	bool rightreleased;
};

#endif
