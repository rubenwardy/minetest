--[[
Minetest
Copyright (C) 2018-2020 SmallJoker

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

local minetest_url -- Filled by HTTP callback

local function version_info_formspec(data)
	return (
		"formspec_version[3]" ..
		"size[10,5.5,true]" ..
		"textarea[0.5,0.5;9.5,3;;" ..
		core.colorize("#0E0", fgettext("A new Minetest version is available")) .. ";" ..
		fgettext("Current version: $1\nNew version: $2\n\n" ..
			"Visit $3 to find out how to get the newest version to stay up to date" ..
			" with the features and bugfixes.",
			core.get_version().string, data.new_version,
			minetest_url) .. "]" ..
		"button[1.5,3.5;3,0.75;version_check_visit;" .. fgettext("Visit website") .. "]" ..
		"button[5.5,3.5;3,0.75;version_check_remind;" .. fgettext("Remind me later") .. "]" ..
		"button[2.5,4.5;5,0.75;version_check_never;" ..
			fgettext("Disable notifications") .. "]"
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
		core.settings:set("update_last_known", "all")
		this:delete()
		return true
	end
	if fields.version_check_visit then
		core.open_url(minetest_url)
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
	local url = core.settings:get("update_information_url")
	if core.settings:get("update_last_known") == "all" or
			url == "" then
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

	local http = core.get_http_api()
	local result = http.fetch_sync({url=url})

	local json = result.succeeded and core.parse_json(result.data)
	if type(json) ~= "table" or not json.latest then
		core.log("error", "Failed to read JSON output from " .. url ..
			", status code = " .. result.code)
		return
	end

	local known_update = tonumber(core.settings:get("update_last_known")) or 0
	-- Format: MMNNPPP (Major, Minor, Patch)
	local new_number = json.latest.version_code
	minetest_url = json.latest.url

	-- Format: Major.Minor.Patch
	-- Convert to MMMNNNPPP
	local cur_string = core.get_version().string
	local cur_major, cur_minor, cur_patch = cur_string:match("^(%d+).(%d+).(%d+)")

	if not new_number or not cur_patch then
		core.log("error", "Failed to read version numbers (invalid tag format?)")
		return
	end

	local cur_number = (cur_major * 1000 + cur_minor) * 1000 + cur_patch
	if new_number <= known_update or new_number < cur_number then
		return
	end
	-- Also consider updating from 1.2.3-dev to 1.2.3
	if new_number == cur_number and not core.get_version().is_dev then
		return
	end

	core.settings:set("update_last_known", tostring(new_number))
	return json.latest.version
end
