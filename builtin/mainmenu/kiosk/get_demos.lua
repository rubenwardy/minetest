-- Luanti
-- Copyright (C) 2026 rubenwardy
-- SPDX-License-Identifier: LGPL-2.1-or-later

local aes = {
	type = "server",
	server = {
		address = "minetest.aes.land",
		post = 30010,
	},
	title = "A.E.S",
	author = "Zughy and Friends",
	description = "Arcade Emulation System, a minigames multiplayer server",
	image = core.get_mainmenu_path() .. DIR_DELIM .. "kiosk" .. DIR_DELIM .. "aes.png",
	start = join_server,
}

local games_info = {
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

	slide_space = {
		image = core.get_mainmenu_path() .. DIR_DELIM .. "kiosk" .. DIR_DELIM .. "slidespace.png",
	},

	prang = {
		description = "An unofficial port of PRANG!, a 2D arcade-style game."
	},
}

assert(core.settings:get("fosdem_password"))

local function join_server(self)
	gamedata.selected_world = 0
	gamedata.address = self.server.address
	gamedata.port = self.server.port
	gamedata.playername = "fosdem"
	gamedata.password = fosdem_password
	gamedata.servername = self.name
	gamedata.serverdescription = self.description
	core.start()
end

local function start_world(self)
	local timestamp = os.date("%Y-%m-%dT%H-%M-%S")
	local worldname = "world_" .. timestamp .. "_" .. self.game.id
	core.create_world(worldname, self.game.id, {})
	menudata.worldlist:refresh()

	local idx = menudata.worldlist:raw_index_by_uid(worldname)
	gamedata.selected_world = idx
	gamedata.singleplayer = true
	core.start()
end

function get_demos()
	local retval = {
		aes,
	}
	for i = 1, #pkgmgr.games do
		local game = pkgmgr.games[i]
		if games_info[game.id] then
			local demo = {
				type = "game",
				game = game,
				title = game.title or game.id,
				author = game.author,
				description = game.description,
				image = game.path .. DIR_DELIM .. "screenshot.png",
				start = start_world,
			}
			for key, value in pairs(games_info[game.id]) do
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
