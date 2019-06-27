#pragma once

#include "irrlichttypes_extrabloated.h"

#include <vector>
#include <functional>

#include "../client/tile.h"

class GUISidebar : public gui::IGUIElement {
public:
	GUISidebar(ISimpleTextureSource *tsrc, gui::IGUIEnvironment *environment, IGUIElement *parent, s32 id);

	void draw() override;
	bool OnEvent(const SEvent &event) override;

	int getWidth() const { return width; }

	void setIconPath(std::string path)
	{
		icon_path = path;
	}

	void setIconHeight(int height)
	{
		icon_height = height;
		rebuild();
	}

	void setCallback(const std::function<void(std::string)> &func) {
		onClick = func;
	}

protected:
	ISimpleTextureSource *tsrc;

	irr::video::SColor bgcolor { 0x8C000000 };
	irr::video::SColor selectedcolor { 0x6C000000 };
	irr::video::SColor hovercolor { 0x3C000000 };

	core::dimension2d<u32> window_size;
	int width = 200;
	std::string icon_path;
	int icon_height = 96;

	std::function<void(std::string)> onClick;

	struct Option {
		core::rect<s32> rect;
		std::string id;
		core::stringw caption;
	};

	std::vector<Option> options;
	int item_hover = -1;
	int item_selected = 2;

	void rebuild();
	void removeChildren();
};

