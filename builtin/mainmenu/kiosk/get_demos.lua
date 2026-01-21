-- Luanti
-- Copyright (C) 2026 rubenwardy
-- SPDX-License-Identifier: LGPL-2.1-or-later

local overrides = {
	asuna = {
		description = "A vibrant world of beautiful biomes. Explore, discover, create.",
	},

	backroomtest = {
		description = "A game about exploring uncanny, vaguely unsettling, liminal spaces. The goal is to explore, be lost, wander. Can you find all the levels?",
		image = core.get_mainmenu_path() .. DIR_DELIM .. "kiosk" .. DIR_DELIM .. "backrooms.png",
	},

	exile = {},

	extra_ordinance = {
		description = "You have several weapons to choose from. You can dig through the earth. So can they.",
		image = core.get_mainmenu_path() .. DIR_DELIM .. "kiosk" .. DIR_DELIM .. "extra_ordinance.png",
	},

	klots = {},

	nodecore = {},

	prang = {
		description = "An unofficial port of PRANG!, a 2D arcade-style game."
	},
}

function get_demos()
	local retval = {
		{
			type = "server",
			server = {
				address = "minetest.aes.land",
				post = 30010,
			},
			title = "A.E.S",
			author = "Zughy and Friends",
			description = "Arcade Emulation System, a Luanti server",
			image = core.get_mainmenu_path() .. DIR_DELIM .. "kiosk" .. DIR_DELIM .. "aes.png",
		}
	}
	for i = 1, #pkgmgr.games do
		local game = pkgmgr.games[i]
		if game.id ~= "devtest" and game.id ~= "minetest" then
			local demo = {
				type = "game",
				game = game,
				title = game.title or game.id,
				author = game.author,
				description = game.description,
				image = game.path .. DIR_DELIM .. "screenshot.png",
			}
			for key, value in pairs(overrides[game.id] or {}) do
				demo[key] = value
			end
			if demo.description:sub(-1) == "." then
				demo.description = demo.description:sub(1, #demo.description - 1)
			end
			table.insert(retval, demo)
		end
	end
	return retval
end
