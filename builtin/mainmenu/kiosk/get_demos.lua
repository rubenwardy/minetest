-- Luanti
-- Copyright (C) 2026 rubenwardy
-- SPDX-License-Identifier: LGPL-2.1-or-later

local kiosk_path = core.get_mainmenu_path() .. DIR_DELIM .. "kiosk" .. DIR_DELIM
local fosdem_password = assert(core.settings:get("fosdem_password"))
local username = assert(core.settings:get("name"))

local function join_server(self)
	gamedata.selected_world = 0
	gamedata.address = self.server.address
	gamedata.port = self.server.port
	gamedata.playername = username
	gamedata.password = fosdem_password
	gamedata.servername = self.name
	gamedata.serverdescription = self.description
	core.start()
end

local function start_world(self)
	local worldname
	if self.persisted then
		worldname = "world_" .. self.game.id
		idx = menudata.worldlist:raw_index_by_uid(worldname)
		if idx == 0 then
			core.create_world(worldname, self.game.id, self.world_settings or {})
			menudata.worldlist:refresh()
		end
	else
		local timestamp = os.date("%Y-%m-%dT%H-%M-%S")
		worldname = "world_" .. timestamp .. "_" .. self.game.id
		core.create_world(worldname, self.game.id, self.world_settings or {})
		menudata.worldlist:refresh()
	end

	local idx = menudata.worldlist:raw_index_by_uid(worldname)
	gamedata.selected_world = idx
	gamedata.singleplayer = true
	core.start()
end

local aes = {
	type = "server",
	server = {
		address = "minetest.aes.land",
		post = 30010,
	},
	title = "A.E.S",
	author = "Zughy and Friends",
	description = "Arcade Emulation System, a minigames multiplayer server.\nDive into lots of minigames with friends: Murder, Skywars, Block League, Fantasy Brawl, Arcade and more!",
	image = kiosk_path .. "aes.png",
	start = join_server,
	tags = { "Minigames", "Online", "Arcade", "PvP" },
}

local games_info = {
	asuna = {
		description = "A vibrant world of beautiful biomes. Explore, discover, create.",
		image = kiosk_path .. "asuna.png",
		tags = { "Survival", "Sandbox", "Atmospheric", },
		persisted = true,
		world_settings = {
			mg_name = "v7",
		},
	},

	backroomtest = {
		description = "A game about exploring uncanny, vaguely unsettling, liminal spaces. The goal is to explore, be lost, wander. Can you find all the levels?",
		image = kiosk_path .. "backrooms.png",
		tags = { "Adventure", "Mystery", "Atmospheric"},
		persisted = true,
		world_settings = {
			mg_name = "v7",
		},
	},

	extra_ordinance = {
		description = "You have several weapons to choose from. You can dig through the earth. So can they.",
		image = kiosk_path .. "extra_ordinance.png",
		tags = { "Action", "Shooter", },
		world_settings = {
			mg_name = "singlenode",
		},
	},

	slide_space = {
		image = kiosk_path .. "slidespace.png",
		tags = { "Puzzle", "Atmospheric", },
		world_settings = {
			mg_name = "singlenode",
		},
	},

	prang = {
		description = "An unofficial port of PRANG!, a 2D arcade-style game.",
		tags = { "Arcade", "2D", },
		world_settings = {
			mg_name = "singlenode",
		},
	},
}

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
			table.insert(retval, demo)
		end
	end
	return retval
end
