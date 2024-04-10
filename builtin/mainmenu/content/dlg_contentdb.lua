--Minetest
--Copyright (C) 2018-20 rubenwardy
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

if not core.get_http_api then
	function create_contentdb_dlg()
		return messagebox("contentdb",
				fgettext("ContentDB is not available when Minetest was compiled without cURL"))
	end
	return
end

-- Filter
local search_string = ""
local cur_page = 1
local filter_type

-- Automatic package installation
local auto_install_spec = nil


local filter_type_names = {
	{ "type_all", nil },
	{ "type_game", "game" },
	{ "type_mod", "mod" },
	{ "type_txp", "txp" },
}


function install_or_update_package(parent, package)
	local install_parent
	if package.type == "mod" then
		install_parent = core.get_modpath()
	elseif package.type == "game" then
		install_parent = core.get_gamepath()
	elseif package.type == "txp" then
		install_parent = core.get_texturepath()
	else
		error("Unknown package type: " .. package.type)
	end

	if package.queued or package.downloading then
		return
	end

	local function on_confirm()
		local has_hard_deps = contentdb.has_hard_deps(package)
		if has_hard_deps then
			local dlg = create_install_dialog(package)
			dlg:set_parent(parent)
			parent:hide()
			dlg:show()
		elseif has_hard_deps == nil then
			local dlg = messagebox("error_checking_deps",
					fgettext("Error getting dependencies for package"))
			dlg:set_parent(parent)
			parent:hide()
			dlg:show()
		else
			contentdb.queue_download(package, package.path and contentdb.REASON_UPDATE or contentdb.REASON_NEW)
		end
	end

	if package.type == "mod" and #pkgmgr.games == 0 then
		local dlg = messagebox("install_game",
			fgettext("You need to install a game before you can install a mod"))
		dlg:set_parent(parent)
		parent:hide()
		dlg:show()
	elseif not package.path and core.is_dir(install_parent .. DIR_DELIM .. package.name) then
		local dlg = create_confirm_overwrite(package, on_confirm)
		dlg:set_parent(parent)
		parent:hide()
		dlg:show()
	else
		on_confirm()
	end
end


-- Resolves the package specification stored in auto_install_spec into an actual package.
-- May only be called after the package list has been loaded successfully.
local function resolve_auto_install_spec()
	assert(contentdb.load_ok)

	if not auto_install_spec then
		return nil
	end

	local spec = contentdb.aliases[auto_install_spec] or auto_install_spec
	local resolved = nil

	for _, pkg in ipairs(contentdb.packages_full_unordered) do
		if pkg.id == spec then
			resolved = pkg
			break
		end
	end

	if not resolved then
		gamedata.errormessage = fgettext("The package $1 was not found.", auto_install_spec)
		ui.update()

		auto_install_spec = nil
	end

	return resolved
end


-- Installs the package specified by auto_install_spec.
-- Only does something if:
-- a. The package list has been loaded successfully.
-- b. The ContentDB dialog is currently visible.
local function do_auto_install()
	if not contentdb.load_ok then
		return
	end

	local pkg = resolve_auto_install_spec()
	if not pkg then
		return
	end

	local contentdb_dlg = ui.find_by_name("contentdb")
	if not contentdb_dlg or contentdb_dlg.hidden then
		return
	end

	install_or_update_package(contentdb_dlg, pkg)
	auto_install_spec = nil
end


local function sort_and_filter_pkgs()
	contentdb.update_paths()
	contentdb.sort_packages()
	contentdb.filter_packages(search_string, filter_type)

	local auto_install_pkg = resolve_auto_install_spec()
	if auto_install_pkg then
		local idx = table.indexof(contentdb.packages, auto_install_pkg)
		if idx ~= -1 then
			table.remove(contentdb.packages, idx)
			table.insert(contentdb.packages, 1, auto_install_pkg)
		end
	end
end


local function load()
	if contentdb.load_ok then
		sort_and_filter_pkgs()
		return
	end
	if contentdb.loading then
		return
	end
	contentdb.fetch_pkgs(function(result)
		if result then
			sort_and_filter_pkgs()
			do_auto_install()
		end
		ui.update()
	end)
end


local function get_info_formspec(size, text)
	return table.concat({
		"formspec_version[6]",
		"size[", size.x, ",", size.y, "]",
		"padding[-0.01,-0.01]",

		"label[4,4.35;", text, "]",
		"container[0,", size.y - 0.8 - 0.375, "]",
		"button[0.375,0;4,0.8;back;", fgettext("Back"), "]",
		"container_end[]",
	})
end


-- Determines how to fit `num_per_page` into `size` space
local function fit_cells(num_per_page, size)
	local cell_spacing = 0.25
	local desired_size = 4.5
	local row_cells = math.min(5, math.floor(size.x / desired_size))
	local cell_w, cell_h
	-- Fit cells into the available height
	while true do
		cell_w = (size.x - (row_cells-1)*cell_spacing) / row_cells
		cell_h = cell_w * 2 / 3

		local required_height = math.ceil(num_per_page / row_cells) * (cell_h + cell_spacing) - cell_spacing
		if required_height <= size.y then
			break
		end

		row_cells = row_cells + 1
	end

	return cell_spacing, row_cells, cell_w, cell_h
end


local function get_formspec(dlgdata)
	local window = core.get_window_info()
	local size = { x = window.max_formspec_size.x, y = window.max_formspec_size.y }

	if contentdb.loading then
		return get_info_formspec(size, fgettext("Loading..."))
	end
	if contentdb.load_error then
		return get_info_formspec(size, fgettext("No packages could be retrieved"))
	end
	assert(contentdb.load_ok)

	contentdb.update_paths()

	local num_per_page = dlgdata.num_per_page
	dlgdata.pagemax = math.max(math.ceil(#contentdb.packages / num_per_page), 1)
	if cur_page > dlgdata.pagemax then
		cur_page = 1
	end

	local W = size.x
	local H = size.y

	local category_x = 0
	local number_category_buttons = 4
	local max_button_w = (size.x - 0.375*2 - 0.25 - 7) / number_category_buttons
	local category_button_w = math.min(max_button_w, 3)
	local function make_category_button(name, label, selected)
		category_x = category_x + 1
		local color = selected and mt_color_green or ""
		return ("style[%s;bgcolor=%s]button[%f,0;%f,0.8;%s;%s]"):format(name, color,
				(category_x - 1) * category_button_w, category_button_w, name, label)
	end

	local selected_type = filter_type

	local search_box_width = size.x - 0.375*2 - 0.25 - 2*0.8
			- number_category_buttons * category_button_w
	local formspec = {
		"formspec_version[7]",
		"size[", size.x, ",", size.y, "]",
		"padding[-0.01,-0.01]",

		-- Top-left: categories
		"container[0.375,0.375]",
		make_category_button("type_all", fgettext("All"), selected_type == nil),
		make_category_button("type_game", fgettext("Games"), selected_type == "game"),
		make_category_button("type_mod", fgettext("Mods"), selected_type == "mod"),
		make_category_button("type_txp", fgettext("Texture Packs"), selected_type == "txp"),
		"container_end[]",

		-- Top-right: Search
		"container[", size.x - 0.375 - search_box_width - 0.8*2, ",0.375]",
		"field[0,0;", search_box_width, ",0.8;search_string;;", core.formspec_escape(search_string), "]",
		"field_enter_after_edit[search_string;true]",
		"image_button[", search_box_width, ",0;0.8,0.8;",
			core.formspec_escape(defaulttexturedir .. "search.png"), ";search;]",
		"image_button[", search_box_width + 0.8, ",0;0.8,0.8;",
			core.formspec_escape(defaulttexturedir .. "clear.png"), ";clear;]",
		"container_end[]",

		-- Bottom strip start
		"container[0,", H - 0.8 - 0.375, "]",
		"button[0.375,0;4,0.8;back;", fgettext("Back to Main Menu"), "]",

		-- Bottom-center: Page nav buttons
		"container[", (W - 1*4 - 2) / 2,  ",0]",
		"image_button[0,0;1,0.8;", core.formspec_escape(defaulttexturedir), "start_icon.png;pstart;]",
		"image_button[1,0;1,0.8;", core.formspec_escape(defaulttexturedir), "prev_icon.png;pback;]",
		"style[pagenum;border=false]",
		"button[2,0;2,0.8;pagenum;", tonumber(cur_page), " / ", tonumber(dlgdata.pagemax), "]",
		"image_button[4,0;1,0.8;", core.formspec_escape(defaulttexturedir), "next_icon.png;pnext;]",
		"image_button[5,0;1,0.8;", core.formspec_escape(defaulttexturedir), "end_icon.png;pend;]",
		"container_end[]", -- page nav end

		-- Bottom-right: updating
		"container[", W - 0.375 - 3,  ",0]",
		"style[status,downloading,queued;border=false]",
	}

	if contentdb.number_downloading > 0 then
		formspec[#formspec + 1] = "button[0,0;3,0.8;downloading;"
		if #contentdb.download_queue > 0 then
			formspec[#formspec + 1] = fgettext("$1 downloading,\n$2 queued",
					contentdb.number_downloading, #contentdb.download_queue)
		else
			formspec[#formspec + 1] = fgettext("$1 downloading...", contentdb.number_downloading)
		end
		formspec[#formspec + 1] = "]"
	else
		local num_avail_updates = 0
		for i=1, #contentdb.packages_full do
			local package = contentdb.packages_full[i]
			if package.path and package.installed_release < package.release and
					not (package.downloading or package.queued) then
				num_avail_updates = num_avail_updates + 1
			end
		end

		if num_avail_updates == 0 then
			formspec[#formspec + 1] = "button[0,0;3,0.8;status;"
			formspec[#formspec + 1] = fgettext("No updates")
			formspec[#formspec + 1] = "]"
		else
			formspec[#formspec + 1] = "button[0,0;3,0.8;update_all;"
			formspec[#formspec + 1] = fgettext("Update All [$1]", num_avail_updates)
			formspec[#formspec + 1] = "]"
		end
	end

	formspec[#formspec + 1] = "container_end[]" -- updating end
	formspec[#formspec + 1] = "container_end[]" -- bottom strip end

	if #contentdb.packages == 0 then
		formspec[#formspec + 1] = "label[4,4.75;"
		formspec[#formspec + 1] = fgettext("No results")
		formspec[#formspec + 1] = "]"
	end

	-- download/queued tooltips always have the same message
	local tooltip_colors = ";#dff6f5;#302c2e]"
	formspec[#formspec + 1] = "tooltip[downloading;" .. fgettext("Downloading...") .. tooltip_colors
	formspec[#formspec + 1] = "tooltip[queued;" .. fgettext("Queued") .. tooltip_colors

	formspec[#formspec + 1] = "container[0.375,1.425]"

	local cell_spacing, row_cells, cell_w, cell_h = fit_cells(num_per_page, {
		x = size.x - 0.375 * 2,
		y = size.y - 1.425 - 0.25 - 0.8 - 0.375
	})

	local start_idx = (cur_page - 1) * num_per_page + 1
	for i=start_idx, math.min(#contentdb.packages, start_idx+num_per_page-1) do
		local package = contentdb.packages[i]

		table.insert_all(formspec, {
			"container[",
			(cell_w + cell_spacing) * ((i - start_idx) % row_cells),
			",",
			(cell_h + cell_spacing) * math.floor((i - start_idx) / row_cells),
			"]",

			-- image,
			"image_button[0,0;", cell_w, ",", cell_h, ";",
				core.formspec_escape(get_screenshot(package, package.thumbnail, 2)),
			";view_", i, ";;;false]",

			--"style[title_", i, ";border=false]",
			-- The 0.01 here fixes a single line of image pixels appearing below the box
			"box[0,", cell_h - 0.8 + 0.01, ";", cell_w, ",0.8;#0009]",

			"style_type[button;border=false;textcolor=", mt_color_green, "]",
			"button[0.25,", cell_h - 0.8, ";", cell_w - 0.5, ",0.5;title_", i ,";",
				core.formspec_escape(core.colorize(mt_color_green, package.title)), "]",
			"style_type[button;font_size=*0.9;textcolor=#fff9]",
			"button[0.25,", cell_h - 0.3, ";",  cell_w - 0.5, ",0.3;author_", i, ";",
				core.formspec_escape(core.colorize("#fff9", package.author)), "]",
			"style_type[button;font_size=;border=;textcolor=]",
		})

		if package.featured or package.short_description:sub(1, 8) == "Featured" then
			table.insert_all(formspec, {
				"tooltip[0,0;0.8,0.8;", fgettext("Featured"), "]",
				"image[0.2,0.2;0.4,0.4;", defaulttexturedir, "server_favorite.png]",
			})
		end

		if package.downloading then
			table.insert_all(formspec, {
				"animated_image[", cell_w - 0.8, ",0;0.8,0.8;", defaulttexturedir, "cdb_downloading.png;3;400;]",
			})
		elseif package.queued then
			table.insert_all(formspec, {
				"image[", cell_w - 0.8, ",0;0.8,0.8;", defaulttexturedir, "cdb_queued.png]",
			})
		elseif package.path then
			if package.installed_release < package.release then
				table.insert_all(formspec, {
					"image[", cell_w - 0.8, ",0;0.8,0.8;", defaulttexturedir, "cdb_update.png]",
				})
			else
				table.insert_all(formspec, {
					"image[", cell_w - 0.6, ",0.2;0.4,0.4;", defaulttexturedir, "checkbox_64.png]",
				})
			end
		end

		local tooltip = package.short_description

		table.insert_all(formspec, {
			"tooltip[0,0;", cell_w, ",", cell_h, ";", core.formspec_escape(tooltip), "]",
			"container_end[]",
		})
	end

	formspec[#formspec + 1] = "container_end[]"

	return table.concat(formspec)
end


local function handle_submit(this, fields)
	if fields.search or fields.key_enter_field == "search_string" then
		search_string = fields.search_string:trim()
		cur_page = 1
		contentdb.filter_packages(search_string, filter_type)
		return true
	end

	if fields.clear then
		search_string = ""
		cur_page = 1
		contentdb.filter_packages("", filter_type)
		return true
	end

	if fields.back then
		this:delete()
		return true
	end

	if fields.pstart then
		cur_page = 1
		return true
	end

	if fields.pend then
		cur_page = this.data.pagemax
		return true
	end

	if fields.pnext then
		cur_page = cur_page + 1
		if cur_page > this.data.pagemax then
			cur_page = 1
		end
		return true
	end

	if fields.pback then
		if cur_page == 1 then
			cur_page = this.data.pagemax
		else
			cur_page = cur_page - 1
		end
		return true
	end

	for _, pair in ipairs(filter_type_names) do
		if fields[pair[1]] then
			filter_type = pair[2]
			cur_page = 1
			contentdb.filter_packages(search_string, filter_type)
			return true
		end
	end

	if fields.update_all then
		for i=1, #contentdb.packages_full do
			local package = contentdb.packages_full[i]
			if package.path and package.installed_release < package.release and
					not (package.downloading or package.queued) then
				contentdb.queue_download(package, contentdb.REASON_UPDATE)
			end
		end
		return true
	end

	local num_per_page = this.data.num_per_page
	local start_idx = (cur_page - 1) * num_per_page + 1
	assert(start_idx ~= nil)
	for i=start_idx, math.min(#contentdb.packages, start_idx+num_per_page-1) do
		local package = contentdb.packages[i]
		assert(package)

		if fields["view_" .. i] or fields["title_" .. i] or fields["author_" .. i] then
			local dlg = create_package_dialog(package)
			dlg:set_parent(this)
			this:hide()
			dlg:show()
			return true
		end
	end

	return false
end


local function handle_events(event)
	if event == "DialogShow" then
		-- On touchscreen, don't show the "MINETEST" header behind the dialog.
		mm_game_theme.set_engine(core.settings:get_bool("enable_touch"))

		-- If ContentDB is already loaded, auto-install packages here.
		do_auto_install()

		return true
	end

	return false
end


--- Creates a ContentDB dialog.
---
--- @param type string | nil
--- Sets initial package filter. "game", "mod", "txp" or nil (no filter).
--- @param install_spec table | nil
--- ContentDB ID of package as returned by pkgmgr.get_contentdb_id().
--- Sets package to install or update automatically.
function create_contentdb_dlg(type, install_spec)
	search_string = ""
	cur_page = 1
	filter_type = type

	-- Keep the old auto_install_spec if the caller doesn't specify one.
	if install_spec then
		auto_install_spec = install_spec
	end

	load()

	local dlg = dialog_create("contentdb",
			get_formspec,
			handle_submit,
			handle_events)
	dlg.data.num_per_page = core.settings:get_bool("enable_touch") and 8 or 15
	return dlg
end
