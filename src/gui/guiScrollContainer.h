/*
Minetest
Copyright (C) 2013 celeron55, Perttu Ahola <celeron55@gmail.com>

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

#pragma once

#include "irrlichttypes_extrabloated.h"
#include "util/string.h"
#include "guiScrollBar.h"

class GUIScrollContainer : public gui::IGUIElement
{
public:
	GUIScrollContainer(gui::IGUIEnvironment *env, gui::IGUIElement *parent, s32 id,
			const core::rect<s32> &rectangle, const std::string &direction,
			f32 scrollfactor);

	virtual bool OnEvent(const SEvent &event) override;

	inline void onScrollEvent(gui::IGUIElement *caller)
	{
		if (caller == m_scrollbar)
			updateScrolling();
	}

	inline void setScrollBar(GUIScrollBar *scrollbar) { m_scrollbar = scrollbar; }

private:
	enum DirectionEnum
	{
		DOWN,
		RIGHT,
		UNDEFINED
	};

	GUIScrollBar *m_scrollbar;
	DirectionEnum m_direction;
	f32 m_scrollfactor;

	void updateScrolling();
};
