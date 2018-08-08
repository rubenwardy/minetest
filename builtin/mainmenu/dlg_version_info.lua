--[[
Minetest
Copyright (C) 2018 SmallJoker

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
]]


local function version_info_formspec(data)
	return (
		"size[9,4.5,true]" ..
		"textarea[0.5,0.5;8.5,3;;" ..
		fgettext("New Minetest version is available") .. ";" ..
		fgettext("Current version: $1\nNew version: $2\n\n" ..
			"Visit $3 to find out how to get the newest version to stay up to date" ..
			" with the features and bugfixes.",
			core.get_version().string, data.new_version,
			"https://www.minetest.net/downloads/") .. "]" ..
		"checkbox[1.5,3;version_check_never;" ..
			fgettext("Never notify me again") .. ";false]" ..
		"button[1.5,4;2,0.5;version_check_seen;" .. fgettext("OK") .. "]" ..
		"button[4.5,4;3,0.5;version_check_remind;" .. fgettext("Remind me later") .. "]"
	)
end

local function version_info_buttonhandler(this, fields)
	if fields.version_check_remind then
		-- Only wait for the check interval
		core.settings:set("update_last_known", "")
		this:delete()
		return true
	end
	if fields.version_check_never then
		if core.is_yes(fields.version_check_never) then
			core.settings:set("update_last_known", "all")
		end
	end
	if fields.version_check_seen then
		this:delete()
		return true
	end

	return false
end


function create_version_info_dlg(new_version)
	assert(type(new_version) == "string")

	local retval = dialog_create("version_info",
		version_info_formspec,
		version_info_buttonhandler,
		nil)
	retval.data.new_version = new_version

	return retval
end

function check_new_version()
	if core.settings:get("update_last_known") == "all" then
		-- Never show any updates
		return
	end

	local time_now = os.time()
	local time_checked = tonumber(core.settings:get("update_last_checked")) or 0
	if time_now - time_checked < 2 * 24 * 3600 then
		-- Check interval of 2 entire days
		return
	end
	core.settings:set("update_last_checked", tostring(time_now))

	local filename = os.tmpname()
	local get_url = ("%s?proto_version_min=%i&proto_version_max=%i"):format(
		core.settings:get("update_information_url"),
		core.get_min_supp_proto(),
		core.get_max_supp_proto()
	)
	if not core.download_file(get_url, filename) then
		os.remove(filename)
		return
	end

	local file = io.open(filename, "r")
	local json = core.parse_json(file:read("*all"))
	file:close()
	os.remove(filename)

	if type(json) ~= "table" then
		core.log("error", "Failed to read JSON output from " ..
			core.settings:get("update_information_url"))
		return
	end

	local known_update = tonumber(core.settings:get("update_last_known")) or 0
	local newest_version = json.tag_name or ""
	local installed_version = core.get_version().string

	-- Ensure it's the identical format
	local nv_major, nv_minor, nv_patch = newest_version:match("^(%d+).(%d+).(%d+)")
	local iv_major, iv_minor, iv_patch = installed_version:match("^(%d+).(%d+).(%d+)")
	if not nv_patch or not iv_patch then
		core.log("error", "Failed to read version numbers (invalid tag format?)")
		return
	end

	local nv_number = nv_major * 0x10000 + nv_minor * 0x100 + nv_patch
	local iv_number = iv_major * 0x10000 + iv_minor * 0x100 + iv_patch
	if nv_number <= known_update or nv_number < iv_number then
		return
	end
	-- Also consider updating from 1.2.3-dev to 1.2.3
	if nv_number == iv_number and not core.get_version().is_dev then
		return
	end

	core.settings:set("update_last_known", tostring(nv_number))
	return newest_version
end
