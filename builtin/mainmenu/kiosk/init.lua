local Kiosk = {}
Kiosk.__index = Kiosk

function Kiosk:get_formspec()
	if self.hidden or (self.parent ~= nil and self.parent.hidden) then
		return ""
	end

	local fs = {
		"formspec_version[7]",
		"size[12,5]",
		"label[1,2;Hello world]",
		"button[1,3;2,0.8;open_menu;Open menu]",
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
