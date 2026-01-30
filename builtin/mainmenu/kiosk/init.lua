-- Luanti
-- Copyright (C) 2026 rubenwardy
-- SPDX-License-Identifier: LGPL-2.1-or-later

local path = core.get_mainmenu_path() .. DIR_DELIM .. "kiosk"
dofile(path .. DIR_DELIM .. "get_demos.lua")

local Kiosk = {}
Kiosk.__index = Kiosk

local function render_tile(demo, x, y, cell_w, cell_h)
	local text = core.colorize(mt_color_green, demo.title) ..
		core.colorize("#BFBFBF", " by " .. demo.author) .. "\n" ..
		demo.description
	local tags = core.colorize("#999", table.concat(demo.tags, ", "))
	local img_w = cell_h * 3 / 2

	local text_w = cell_w - img_w - 0.25 - 0.25
	local text_h = cell_h - 0.25 - 0.025 - 0.5

	local blank = core.formspec_escape(defaulttexturedir .. "blank.png")
	return {
		"container[", x, ",", y, "]",

		"box[0,0;", cell_w, ",", cell_h, ";#ffffff11]",

		-- image,
		"image[0,0;", img_w, ",", cell_h, ";",
		 	core.formspec_escape(demo.image or blank), "]",

		"label[", img_w + 0.25, ",0.25;", text_w, ",", text_h, ";",
			core.formspec_escape(text), "]",

		"label[", img_w + 0.25, ",", cell_h - 0.5 , ";", text_w, ",0.5;",
			core.formspec_escape(tags), "]",

		-- Add a tooltip in case the label overflows and the short description is cut off.
		"tooltip[", img_w + 0.25, ",0.25;", text_w, ",", text_h, ";",
			-- Text in tooltips doesn't wrap automatically, so we do it manually to
			-- avoid everything being one long line.
			core.formspec_escape(core.wrap_text(demo.description, 80)), "]",

		"style[view_", demo.title, ";border=false]",
		"style[view_", demo.title, ":hovered;bgimg=", core.formspec_escape(defaulttexturedir .. "button_hover_semitrans.png"), "]",
		"style[view_", demo.title, ":pressed;bgimg=", core.formspec_escape(defaulttexturedir .. "button_press_semitrans.png"), "]",
		"button[0,0;", cell_w, ",", cell_h, ";view_", demo.title, ";]",
		"container_end[]",
	}
end

function Kiosk:get_formspec()
	if self.hidden or (self.parent ~= nil and self.parent.hidden) then
		return ""
	end

	mm_game_theme.set_engine(true)

	local size = contentdb.get_formspec_size()
	local window_padding = contentdb.get_formspec_padding()
	local window = core.get_window_info()
	local demos = get_demos()

	local fs = {
		"formspec_version[7]",
		"size[", size.x, ",", size.y, "]",
		"padding[0,0]",
		"bgcolor[;true;#111]",

		"container[", window_padding.x, ",", window_padding.y, "]",

		"image[0,-0.05;0.8,0.8;", core.formspec_escape(path .. DIR_DELIM .. "luanti.png"), "]",

		"style_type[label;font_size=32]",
		"label[1.05,0.1;", size.x, ",1;Luanti showcase]",
		"style_type[label;font_size=]",

		-- "button[", size.x - window_padding.x * 2 - 4, ",0;4,0.8;open_menu;Open main menu]",
		"style[open_menu;border=false]",
		"tooltip[open_menu;Open main menu]",
		"image_button[", size.x - window_padding.x * 2 - 0.8, ",0;0.8,0.8;",
			core.formspec_escape(path .. DIR_DELIM .. "home.png"), ";open_menu;]",
	}

	local columns = math.max(1, math.floor(size.x / 8))
	local cell_spacing = 0.25
	local cell_w = (size.x - window_padding.x * 2 + cell_spacing) / (columns) - cell_spacing
	local cell_h = 3
	local y = 1
	for i = 1, #demos do
		local demo = demos[i]
		local x = (cell_w + cell_spacing) * ((i - 1) % columns)
		local y = (cell_h + cell_spacing) * math.floor((i - 1) / columns) + 1
		table.insert_all(fs, render_tile(demo, x, y, cell_w, cell_h))
		y = y + 1
	end

	table.insert_all(fs, {
		"container_end[]",
	})

	return table.concat(fs)
end

function Kiosk:handle_buttons(fields)
	if self.hidden then
		return false
	end

	if self.glb_btn_handler ~= nil and self.glb_btn_handler(self, fields) then
		return true
	end

	if fields.open_menu then
		local tabview = ui.find_by_name("maintab")
		self:hide()
		tabview:show()
		return true
	end

	local demos = get_demos()
	for i = 1, #demos do
		if fields["view_" .. demos[i].title] then
			demos[i]:start()
			return true
		end
	end
end

function Kiosk:handle_events(event)
	if self.hidden then
		return false
	end

	if self.glb_evt_handler ~= nil and self.glb_evt_handler(self, event) then
		return true
	end
end

function Kiosk:show()
	self.hidden = false
end

function Kiosk:hide()
	self.hidden = true
end

function Kiosk:delete()
	ui.delete(self)
end

function Kiosk:set_parent(parent)
	self.parent = parent
end

function Kiosk:set_global_button_handler(handler)
	self.glb_btn_handler = handler
end

function Kiosk:set_global_event_handler(handler)
	self.glb_evt_handler = handler
end

kiosk = setmetatable({
	name = "kiosk",
	type = "toplevel",
	width = 12,
	height = 5,
	header_x = nil,
	header_y = nil,
	fixed_size = false,
	hidden = true
}, Kiosk)
ui.add(kiosk)
