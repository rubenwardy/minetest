#include "../irrlicht_changes/static_text.h"
#include "../client/guiscalingfilter.h"
#include "guiSidebar.h"

using namespace gui;

GUISidebar::GUISidebar(ISimpleTextureSource *tsrc, IGUIEnvironment *environment, IGUIElement *parent, s32 id):
		IGUIElement(EGUIET_ELEMENT, environment, parent, id, core::rect<s32>(0, 0, 200, 100)),
		tsrc(tsrc)
{
	options = {
		Option{{}, "new", L"New Game"},
		Option{{}, "load", L"Load Game"},
		Option{{}, "online", L"Join Game"},
		Option{{}, "content", L"Content"},
		Option{{}, "settings", L"Settings"},
		Option{{}, "credits", L"Credits"},
	};
}

void GUISidebar::draw()
{
	auto driver = Environment->getVideoDriver();

	auto new_size = driver->getScreenSize();
	if (window_size != new_size) {
		window_size = new_size;
		rebuild();
	}

	Environment->getVideoDriver()->draw2DRectangle(bgcolor, DesiredRect, nullptr);

	video::ITexture *texture = tsrc->getTexture(icon_path);
	if (texture != 0) {
		const auto &orig = texture->getOriginalSize();

		float scale = (float)icon_height / (float)orig.Height;
		core::rect<s32> rect = core::rect<s32>(0, 0, orig.Width * scale, orig.Height * scale);
		rect += v2s32((width - rect.getWidth()) / 2, 24);

		// Image rectangle on screen
		const video::SColor color(255,255,255,255);
		const video::SColor colors[] = {color,color,color,color};
		draw2DImageFilterScaled(driver, texture, rect,
				core::rect<s32>(core::position2d<s32>(0,0),orig),
				NULL/*&AbsoluteClippingRect*/, colors, true);
	}

	if (item_selected >= 0)
		Environment->getVideoDriver()->draw2DRectangle(selectedcolor, options[item_selected].rect, nullptr);

	if (item_hover >= 0)
		Environment->getVideoDriver()->draw2DRectangle(hovercolor, options[item_hover].rect, nullptr);

	gui::IGUIElement::draw();
}

bool GUISidebar::OnEvent(const SEvent &event)
{
	if (event.EventType == EEVENT_TYPE::EET_MOUSE_INPUT_EVENT) {
		errorstream << "Mouse" << std::endl;
		if (event.MouseInput.Event == EMOUSE_INPUT_EVENT::EMIE_MOUSE_MOVED) {
			errorstream << "Move" << std::endl;
			item_hover = -1;
			auto mpos = v2s32(event.MouseInput.X, event.MouseInput.Y);
			for (int i = 0; i < options.size(); ++i) {
				const auto &opt = options[i];
				if (opt.rect.isPointInside(mpos)) {
					item_hover = i;
					break;
				}
			}
		} else if (event.MouseInput.Event == EMOUSE_INPUT_EVENT::EMIE_LMOUSE_LEFT_UP) {
			errorstream << "helo" << std::endl;
			auto mpos = v2s32(event.MouseInput.X, event.MouseInput.Y);
			for (const auto &opt : options) {
				if (opt.rect.isPointInside(mpos)) {
					onClick(opt.id);
				}
			}
		}
	}
	return false;
}

void GUISidebar::rebuild()
{
	DesiredRect = core::rect<s32>(0, 0, width, window_size.Height);
	AbsoluteClippingRect = DesiredRect;

	removeChildren();

	core::rect<s32> label_rect(24, 0, width - 24, 51);
	core::rect<s32> button_rect(0, 0, width, 51);
	label_rect += v2s32(0, 24 + icon_height + 16);
	button_rect += v2s32(0, 24 + icon_height + 16);

	for (auto &opt : options) {
		opt.rect = button_rect;

		gui::IGUIStaticText *e = gui::StaticText::add(Environment,
				opt.caption.c_str(), label_rect, false, false, this, 0);
		e->setTextAlignment(gui::EGUIA_UPPERLEFT, gui::EGUIA_CENTER);

		label_rect += v2s32(0, label_rect.getHeight());
		button_rect += v2s32(0, label_rect.getHeight());
	}
}

void GUISidebar::removeChildren()
{
	const core::list<gui::IGUIElement*> &children = getChildren();
	while (!children.empty()) {
		(*children.getLast())->remove();
	}
}
