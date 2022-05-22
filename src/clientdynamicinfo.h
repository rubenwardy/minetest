#pragma once

#include "irrTypes.h"
#include <bitset>

struct ClientDynamicInfo
{
	enum InputMethodId
	{
		IM_TOUCHSCREEN_CONTROLS = 0,
		IM_TOUCHSCREEN_GUI,
		IM_KEYBOARD,
		IM_MOUSE,
		IM_GAMEPAD,
	};

	v2u32 render_target_size;
	f32 real_gui_scaling;
	f32 real_hud_scaling;

	std::bitset<5> input_methods;

	inline bool hasInputMethod(InputMethodId method) const {
		return input_methods.test(method);
	}

	bool equal(const ClientDynamicInfo &other) const {
		return render_target_size == other.render_target_size &&
				abs(real_gui_scaling - other.real_gui_scaling) < 0.001f &&
				abs(real_hud_scaling - other.real_hud_scaling) < 0.001f &&
				input_methods == other.input_methods;
	}
};
