#include "inputhandler.h"

void KeyCache::populate_nonchanging()
{
	key[KeyType::ESC] = KeyPress("KEY_ESCAPE");
	key[KeyType::CANCEL] = KeyPress("KEY_CANCEL");

	// const KeyPress NumberKey[] = {
	// 	KeyPress("0"), KeyPress("1"), KeyPress("2"), KeyPress("3"), KeyPress("4"),
	// 	KeyPress("5"), KeyPress("6"), KeyPress("7"), KeyPress("8"), KeyPress("9")
	// };
}

void KeyCache::populate()
{
	key[KeyType::FORWARD]      = getKeySetting("keymap_forward");
	key[KeyType::BACKWARD]     = getKeySetting("keymap_backward");
	key[KeyType::LEFT]         = getKeySetting("keymap_left");
	key[KeyType::RIGHT]        = getKeySetting("keymap_right");
	key[KeyType::JUMP]         = getKeySetting("keymap_jump");
	key[KeyType::SPECIAL1]     = getKeySetting("keymap_special1");
	key[KeyType::SNEAK]        = getKeySetting("keymap_sneak");

	key[KeyType::AUTORUN]      = getKeySetting("keymap_autorun");

	key[KeyType::DROP]         = getKeySetting("keymap_drop");
	key[KeyType::INVENTORY]    = getKeySetting("keymap_inventory");
	key[KeyType::CHAT]         = getKeySetting("keymap_chat");
	key[KeyType::CMD]          = getKeySetting("keymap_cmd");
	key[KeyType::CONSOLE]      = getKeySetting("keymap_console");
	key[KeyType::MINIMAP]      = getKeySetting("keymap_minimap");
	key[KeyType::FREEMOVE]     = getKeySetting("keymap_freemove");
	key[KeyType::FASTMOVE]     = getKeySetting("keymap_fastmove");
	key[KeyType::NOCLIP]       = getKeySetting("keymap_noclip");
	key[KeyType::CINEMATIC]    = getKeySetting("keymap_cinematic");
	key[KeyType::SCREENSHOT]   = getKeySetting("keymap_screenshot");
	key[KeyType::TOGGLE_HUD]   = getKeySetting("keymap_toggle_hud");
	key[KeyType::TOGGLE_CHAT]  = getKeySetting("keymap_toggle_chat");
	key[KeyType::TOGGLE_FORCE_FOG_OFF]
			= getKeySetting("keymap_toggle_force_fog_off");
	key[KeyType::TOGGLE_UPDATE_CAMERA]
			= getKeySetting("keymap_toggle_update_camera");
	key[KeyType::TOGGLE_DEBUG]
			= getKeySetting("keymap_toggle_debug");
	key[KeyType::TOGGLE_PROFILER]
			= getKeySetting("keymap_toggle_profiler");
	key[KeyType::CAMERA_MODE]
			= getKeySetting("keymap_camera_mode");
	key[KeyType::INCREASE_VIEWING_RANGE]
			= getKeySetting("keymap_increase_viewing_range_min");
	key[KeyType::DECREASE_VIEWING_RANGE]
			= getKeySetting("keymap_decrease_viewing_range_min");
	key[KeyType::RANGESELECT]
			= getKeySetting("keymap_rangeselect");
	key[KeyType::ZOOM] = getKeySetting("keymap_zoom");

	key[KeyType::QUICKTUNE_NEXT] = getKeySetting("keymap_quicktune_next");
	key[KeyType::QUICKTUNE_PREV] = getKeySetting("keymap_quicktune_prev");
	key[KeyType::QUICKTUNE_INC]  = getKeySetting("keymap_quicktune_inc");
	key[KeyType::QUICKTUNE_DEC]  = getKeySetting("keymap_quicktune_dec");

	key[KeyType::DEBUG_STACKS]   = getKeySetting("keymap_print_debug_stacks");

	if (handler) {
		// First clear all keys, then re-add the ones we listen for
		handler->dontListenForKeys();
		for (size_t i = 0; i < KeyType::INTERNAL_ENUM_COUNT; i++) {
			handler->listenForKey(key[i]);
		}
		for (size_t i = 0; i < 10; i++) {
			handler->listenForKey(NumberKey[i]);
		}
	}
}
