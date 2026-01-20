local Kiosk = {}
Kiosk.__index = Kiosk

function Kiosk:get_formspec()
	if self.hidden or (self.parent ~= nil and self.parent.hidden) then
		return ""
	end

	mm_game_theme.set_engine(true)

	local size = contentdb.get_formspec_size()
	local window_padding = contentdb.get_formspec_padding()
	local window = core.get_window_info()

	local fs = {
		"formspec_version[7]",
		"size[", size.x, ",", size.y, "]",
		"padding[0,0]",
		"bgcolor[;true]",

		"container[", window_padding.x, ",", window_padding.y, "]",

		"label[0,0;", size.x, ",1;Luanti demo mode]",

		"container[", (size.x - 4) / 2,  ", ", size.y - window_padding.y * 2 - 0.8, "]",
		"button[0,0;4,0.8;open_menu;Open main menu]",
		"container_end[]",

		"container_end[]",
	}

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
