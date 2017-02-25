#include "myeventreceiver.h"
#include "joystick_controller.h"
#include "../keycode.h"
#include "../game.h"
#include "../mainmenumanager.h"
#include "../util/basic_macros.h"
#include "../util/numeric.h"

bool MyEventReceiver::OnEvent(const SEvent& event)
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

void MyEventReceiver::clearInput()
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
