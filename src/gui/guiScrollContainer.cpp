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

#include "guiScrollContainer.h"

GUIScrollContainer::GUIScrollContainer(gui::IGUIEnvironment *env,
		gui::IGUIElement *parent, s32 id, const core::rect<s32> &rectangle,
		const std::string &direction, f32 scrollfactor) :
		gui::IGUIElement(gui::EGUIET_ELEMENT, env, parent, id, rectangle),
		m_scrollbar(nullptr), m_scrollfactor(scrollfactor)
{
	if (direction == "down")
		m_direction = DOWN;
	else if (direction == "right")
		m_direction = RIGHT;
	else
		m_direction = UNDEFINED;
}

bool GUIScrollContainer::OnEvent(const SEvent &event)
{
	if (event.EventType == EET_MOUSE_INPUT_EVENT &&
			event.MouseInput.Event == EMIE_MOUSE_WHEEL && m_scrollbar) {
		Environment->setFocus(m_scrollbar);
		return m_scrollbar->OnEvent(event);
	}

	return IGUIElement::OnEvent(event);
}

void GUIScrollContainer::updateScrolling()
{
	s32 pos = m_scrollbar->getPos();
	core::rect<s32> rect = getRelativePosition();

	if (m_direction == DOWN)
		rect.UpperLeftCorner.Y = pos * m_scrollfactor;
	else if (m_direction == RIGHT)
		rect.UpperLeftCorner.X = pos * m_scrollfactor;

	setRelativePosition(rect);
}
