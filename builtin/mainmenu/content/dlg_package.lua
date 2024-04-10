--Minetest
--Copyright (C) 2018-24 rubenwardy
--
--This program is free software; you can redistribute it and/or modify
--it under the terms of the GNU Lesser General Public License as published by
--the Free Software Foundation; either version 2.1 of the License, or
--(at your option) any later version.
--
--This program is distributed in the hope that it will be useful,
--but WITHOUT ANY WARRANTY; without even the implied warranty of
--MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--GNU Lesser General Public License for more details.
--
--You should have received a copy of the GNU Lesser General Public License along
--with this program; if not, write to the Free Software Foundation, Inc.,
--51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.


local function get_info_formspec(size, text)
	return table.concat({
		"formspec_version[6]",
		"size[", size.x, ",", size.y, "]",
		"padding[-0.01,-0.01]",

		"label[4,4.35;", text, "]",
		"container[0,", size.y - 0.8 - 0.375, "]",
		"button[0.375,0;2,0.8;back;", fgettext("Back"), "]",
		"container_end[]",
	})
end


--- Creates a scrollbaroptions for a scroll_container
--
-- @param visible_l the length of the scroll_container and scrollbar
-- @param total_l length of the scrollable area
-- @param scroll_factor as passed to scroll_container
local function make_scrollbaroptions_for_scroll_container(visible_l, total_l, scroll_factor)
	assert(total_l >= visible_l)
	local max = total_l - visible_l
	local thumb_size = (visible_l / total_l) * max
	return ("scrollbaroptions[min=0;max=%f;thumbsize=%f]"):format(max / scroll_factor, thumb_size / scroll_factor)
end


local function get_formspec(data)
	local window = core.get_window_info()
	local size = { x = window.max_formspec_size.x, y = window.max_formspec_size.y }

	if not data.info then
		if not data.loading and not data.loading_error then
			data.loading = true

			contentdb.get_full_package_info(data.package, function(info)
				data.loading = false

				if info == nil then
					data.loading_error = true
					ui.update()
				elseif data.package.name == info.name then
					data.info = info
					ui.update()
				end
			end)
		end

		if data.loading_error then
			return get_info_formspec(size, fgettext("No packages could be retrieved"))
		else
			return get_info_formspec(size, fgettext("Loading..."))
		end
	else
		-- Check installation status
		contentdb.update_paths()

		local info = data.info

		local info_line =
				fgettext("by $1  —  $2 downloads  —  +$3 / $4 / -$5",
						info.author, info.downloads,
						info.reviews.positive, info.reviews.neutral, info.reviews.negative)

		local bottom_buttons_y = size.y - 0.8 - 0.375

		local formspec = {
			"formspec_version[7]",
			"size[", size.x, ",",  size.y, "]",
			"padding[-0.01,-0.01]",
			"bgcolor[#0000]",
			"box[0,0;", size.x, ",", size.y, ";#0000008C]",

			"button[0.375,", bottom_buttons_y, ";2,0.8;back;", fgettext("Back"), "]",
			"button[", size.x - 3.375, ",", bottom_buttons_y, ";3,0.8;open_contentdb;", fgettext("ContentDB page"), "]",

			"style_type[label;font_size=+24;font=bold]",
			"label[0.375,0.7;", core.formspec_escape(info.title), "]",
			"style_type[label;font_size=;font=]",

			"label[0.375,1.4;", core.formspec_escape(info_line), "]",
		}

		local x = size.x - 3.375
		local function add_link_button(label, name)
			if info[name] then
				x = x - 3.25
				table.insert_all(formspec, {
					"button[", x, ",", bottom_buttons_y, ";3,0.8;open_", name, ";", label, "]",
				})
			end
		end
		add_link_button(fgettext("Translate"), "translation_url")
		add_link_button(fgettext("Issue Tracker"), "issue_tracker")
		add_link_button(fgettext("Forums"), "forums")
		add_link_button(fgettext("Source"), "repo")
		add_link_button(fgettext("Website"), "website")

		table.insert_all(formspec, {
			"container[", size.x - 6.375, ",0.375]"
		})

		local left_button_rect = "0,0;2.875,1"
		local right_button_rect = "3.125,0;2.875,1"
		if data.package.downloading then
			formspec[#formspec + 1] = "animated_image[5,0;1,1;downloading;"
			formspec[#formspec + 1] = core.formspec_escape(defaulttexturedir)
			formspec[#formspec + 1] = "cdb_downloading.png;3;400;]"
		elseif data.package.queued then
			formspec[#formspec + 1] = "image_button[5,0;1,1;" .. core.formspec_escape(defaulttexturedir)
			formspec[#formspec + 1] = "cdb_queued.png;queued;]"
		elseif not data.package.path then
			formspec[#formspec + 1] = "style[install;bgcolor=green]"
			formspec[#formspec + 1] = "button["
			formspec[#formspec + 1] = right_button_rect
			formspec[#formspec + 1] =";install;"
			formspec[#formspec + 1] = fgettext("Install [$1]", info.download_size)
			formspec[#formspec + 1] = "]"
		else
			if data.package.installed_release < data.package.release then
				-- The install_ action also handles updating
				formspec[#formspec + 1] = "style[install;bgcolor=#28ccdf]"
				formspec[#formspec + 1] = "button["
				formspec[#formspec + 1] = left_button_rect
				formspec[#formspec + 1] = ";install;"
				formspec[#formspec + 1] = fgettext("Update")
				formspec[#formspec + 1] = "]"
			end

			formspec[#formspec + 1] = "style[uninstall;bgcolor=#a93b3b]"
			formspec[#formspec + 1] = "button["
			formspec[#formspec + 1] = right_button_rect
			formspec[#formspec + 1] = ";uninstall;"
			formspec[#formspec + 1] = fgettext("Uninstall")
			formspec[#formspec + 1] = "]"
		end

		local current_tab = data.current_tab or 1
		local tab_titles = {
			fgettext("Description"),
			fgettext("Information"),
		}

		local tab_body_height = bottom_buttons_y - 2.8

		table.insert_all(formspec, {
			"container_end[]",

			"tabheader[0.375,2.55;", size.x - 0.375*2, ",0.8;tabs;",
					table.concat(tab_titles, ","), ";", current_tab, ";true;true]",

			"container[0,2.8]",
		})

		if current_tab == 1 then
			-- Screenshots and description
			local hypertext = "<big><b>" .. core.hypertext_escape(info.short_description) .. "</b></big>\n"
			local winfo = core.get_window_info()
			local fs_to_px = winfo.size.x / winfo.max_formspec_size.x
			for i, ss in ipairs(info.screenshots) do
				local path = get_screenshot(data.package, ss.url, 2)
				hypertext = hypertext .. "<action name=ss_" .. i .. "><img name=" .. core.hypertext_escape(path) .. " width=" .. (3 * fs_to_px) .. " height=" .. (2 * fs_to_px) .. "></action>"
				if i ~= #info.screenshots then
					hypertext = hypertext .. "<img name=" .. core.hypertext_escape(defaulttexturedir) .. "blank.png width=" .. (0.25 * fs_to_px).. " height=" .. (2.25 * fs_to_px).. ">"
				end
			end
			hypertext = hypertext .. "\n" .. info.long_description.head ..
					info.long_description.body

			hypertext = hypertext:gsub("<img name=blank.png ",
					"<img name=" .. core.hypertext_escape(defaulttexturedir) .. "blank.png ")

			table.insert_all(formspec, {
				"hypertext[0.375,0;",
					size.x - 3*0.375, ",",
					tab_body_height - 0.375,
					";desc;", core.formspec_escape(hypertext), "]",

			})

		elseif current_tab == 2 then
			local hypertext = info.info_hypertext.head .. info.info_hypertext.body

			table.insert_all(formspec, {
				"hypertext[0.375,0;", size.x - 2*0.375, ",", tab_body_height - 0.375,
					";info;", core.formspec_escape(hypertext), "]",
			})
		else
			error("Unknown tab " .. current_tab)
		end

		formspec[#formspec + 1] = "container_end[]"

		return table.concat(formspec)
	end
end


local function handle_hypertext_event(this, event, hypertext_object)
	if not (event and event:sub(1, 7) == "action:") then
		return
	end

	for i, ss in ipairs(this.data.info.screenshots) do
		if event == "action:ss_" .. i then
			core.open_url(ss.url)
			return true
		end
	end

	-- TODO: escape base_url
	local base_url = core.settings:get("contentdb_url")
	for key, url in pairs(hypertext_object.links) do
		if event == "action:" .. key then
			local author, name = url:match("^" .. base_url .. "/?packages/([A-Za-z0-9 _-]+)/([a-z0-9_]+)/?$")
			if author and name then
				local package2 = contentdb.get_package_by_info(author, name)
				if package2 then
					local dlg = create_package_dialog(package2)
					dlg:set_parent(this)
					this:hide()
					dlg:show()
					return true
				end
			end

			core.open_url_dialog(url)
			return true
		end
	end
end


local function handle_submit(this, fields)
	local info = this.data.info
	local package = this.data.package

	if fields.back then
		this:delete()
		return true
	end

	if not info then
		return false
	end

	if fields.open_contentdb then
		local url = ("%s/packages/%s/?protocol_version=%d"):format(
			core.settings:get("contentdb_url"), package.url_part,
			core.get_max_supp_proto())
		core.open_url(url)
		return true
	end

	if fields.open_translation_url then
		core.open_url_dialog(info.translation_url)
		return true
	end

	if fields.open_issue_tracker then
		core.open_url_dialog(info.issue_tracker)
		return true
	end

	if fields.open_forums then
		core.open_url("https://forum.minetest.net/viewtopic.php?t=" .. info.forums)
		return true
	end

	if fields.open_repo then
		core.open_url_dialog(info.repo)
		return true
	end

	if fields.open_website then
		core.open_url_dialog(info.website)
		return true
	end

	if fields.install then
		install_or_update_package(this, package)
		return true
	end

	if fields.uninstall then
		local dlg = create_delete_content_dlg(package)
		dlg:set_parent(this)
		this:hide()
		dlg:show()
		return true
	end

	if fields.tabs then
		this.data.current_tab = tonumber(fields.tabs)
		return true
	end

	if handle_hypertext_event(this, fields.desc, info.long_description) or
			handle_hypertext_event(this, fields.info, info.info_hypertext) then
		return true
	end
end


function create_package_dialog(package)
	assert(package)

	local dlg = dialog_create("package_dialog_" .. package.id,
			get_formspec,
			handle_submit)
	local data = dlg.data

	data.package = package
	data.info = nil
	data.loading = false
	data.loading_error = nil
	data.current_tab = 1
	return dlg
end
