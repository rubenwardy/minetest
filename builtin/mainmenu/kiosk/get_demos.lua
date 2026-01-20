-- Luanti
-- Copyright (C) 2026 rubenwardy
-- SPDX-License-Identifier: LGPL-2.1-or-later

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
		}
	}
	for i = 1, #pkgmgr.games do
		local game = pkgmgr.games[i]
		if game.id ~= "devtest" and game.id ~= "minetest" then
			table.insert(retval, {
				type = "game",
				game = game,
				title = game.title or game.id,
				author = game.author,
				description = game.description,
			})
		end
	end
	return retval
end
