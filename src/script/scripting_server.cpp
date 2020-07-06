/*
Minetest
Copyright (C) 2013 celeron55, Perttu Ahola <celeron55@gmail.com>

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
*/

#include "util/make_unique.h"
#include "filesys.h"
#include "scripting_server.h"
#include "server.h"
#include "log.h"
#include "settings.h"
#include "cpp_api/s_internal.h"
#include "lua_api/l_areastore.h"
#include "lua_api/l_auth.h"
#include "lua_api/l_base.h"
#include "lua_api/l_craft.h"
#include "lua_api/l_env.h"
#include "lua_api/l_inventory.h"
#include "lua_api/l_item.h"
#include "lua_api/l_itemstackmeta.h"
#include "lua_api/l_mapgen.h"
#include "lua_api/l_modchannels.h"
#include "lua_api/l_nodemeta.h"
#include "lua_api/l_nodetimer.h"
#include "lua_api/l_noise.h"
#include "lua_api/l_object.h"
#include "lua_api/l_playermeta.h"
#include "lua_api/l_particles.h"
#include "lua_api/l_rollback.h"
#include "lua_api/l_server.h"
#include "lua_api/l_util.h"
#include "lua_api/l_vmanip.h"
#include "lua_api/l_settings.h"
#include "lua_api/l_http.h"
#include "lua_api/l_storage.h"

extern "C" {
#include "lualib.h"
}


class ServerSecurityPolicy : public ISecurityPolicy, LuaHelper
{
public:
	void initializeEnvironment(lua_State *L) override
	{

	}

	bool checkPath(lua_State *L, const std::string &abs_path, const std::string &cur_path, bool write_required, bool *write_allowed) override
	{
		std::string str;  // Transient

		// Get server from registry
		lua_rawgeti(L, LUA_REGISTRYINDEX, CUSTOM_RIDX_SCRIPTAPI);
		ScriptApiBase *script;
#if INDIRECT_SCRIPTAPI_RIDX
		script = (ScriptApiBase *) *(void**)(lua_touserdata(L, -1));
#else
		script = (ScriptApiBase *) lua_touserdata(L, -1);
#endif
		lua_pop(L, 1);
		const IGameDef *gamedef = script->getGameDef();
		if (!gamedef)
			return false;

		// Get mod name
		lua_rawgeti(L, LUA_REGISTRYINDEX, CUSTOM_RIDX_CURRENT_MOD_NAME);
		if (lua_isstring(L, -1)) {
			std::string mod_name = readParam<std::string>(L, -1);

			// Builtin can access anything
			if (mod_name == BUILTIN_MOD_NAME) {
				if (write_allowed) *write_allowed = true;
				return true;
			}

			// Allow paths in mod path
			// Don't bother if write access isn't important, since it will be handled later
			if (write_required || write_allowed != NULL) {
				const ModSpec *mod = gamedef->getModSpec(mod_name);
				if (mod) {
					str = fs::AbsolutePath(mod->path);
					if (!str.empty() && fs::PathStartsWith(abs_path, str)) {
						if (write_allowed) *write_allowed = true;
						return true;
					}
				}
			}
		}
		lua_pop(L, 1);  // Pop mod name

		// Allow read-only access to all mod directories
		if (!write_required) {
			const std::vector<ModSpec> &mods = gamedef->getMods();
			for (const ModSpec &mod : mods) {
				str = fs::AbsolutePath(mod.path);
				if (!str.empty() && fs::PathStartsWith(abs_path, str)) {
					return true;
				}
			}
		}

		str = fs::AbsolutePath(gamedef->getWorldPath());
		if (!str.empty()) {
			// Don't allow access to other paths in the world mod/game path.
			// These have to be blocked so you can't override a trusted mod
			// by creating a mod with the same name in a world mod directory.
			// We add to the absolute path of the world instead of getting
			// the absolute paths directly because that won't work if they
			// don't exist.
			if (fs::PathStartsWith(abs_path, str + DIR_DELIM + "worldmods") ||
				fs::PathStartsWith(abs_path, str + DIR_DELIM + "game")) {
				return false;
			}
			// Allow all other paths in world path
			if (fs::PathStartsWith(abs_path, str)) {
				if (write_allowed) *write_allowed = true;
				return true;
			}
		}

		// Default to disallowing
		return false;
	}
};


ServerScripting::ServerScripting(Server* server):
		ScriptApiBase(ScriptingType::Server)
{
	setGameDef(server);

	// setEnv(env) is called by ScriptApiEnv::initializeEnvironment()
	// once the environment has been created

	SCRIPTAPI_PRECHECKHEADER

	if (g_settings->getBool("secure.enable_security")) {
		initializeSecurity(std::make_unique<ServerSecurityPolicy>());
	} else {
		warningstream << "\\!/ Mod security should never be disabled, as it allows any mod to "
				<< "access the host machine."
				<< "Mods should use minetest.request_insecure_environment() instead \\!/" << std::endl;
	}

	lua_getglobal(L, "core");
	int top = lua_gettop(L);

	lua_newtable(L);
	lua_setfield(L, -2, "object_refs");

	lua_newtable(L);
	lua_setfield(L, -2, "luaentities");

	// Initialize our lua_api modules
	InitializeModApi(L, top);
	lua_pop(L, 1);

	// Push builtin initialization type
	lua_pushstring(L, "game");
	lua_setglobal(L, "INIT");

	infostream << "SCRIPTAPI: Initialized game modules" << std::endl;
}

void ServerScripting::InitializeModApi(lua_State *L, int top)
{
	// Register reference classes (userdata)
	InvRef::Register(L);
	ItemStackMetaRef::Register(L);
	LuaAreaStore::Register(L);
	LuaItemStack::Register(L);
	LuaPerlinNoise::Register(L);
	LuaPerlinNoiseMap::Register(L);
	LuaPseudoRandom::Register(L);
	LuaPcgRandom::Register(L);
	LuaRaycast::Register(L);
	LuaSecureRandom::Register(L);
	LuaVoxelManip::Register(L);
	NodeMetaRef::Register(L);
	NodeTimerRef::Register(L);
	ObjectRef::Register(L);
	PlayerMetaRef::Register(L);
	LuaSettings::Register(L);
	StorageRef::Register(L);
	ModChannelRef::Register(L);

	// Initialize mod api modules
	ModApiAuth::Initialize(L, top);
	ModApiCraft::Initialize(L, top);
	ModApiEnvMod::Initialize(L, top);
	ModApiInventory::Initialize(L, top);
	ModApiItemMod::Initialize(L, top);
	ModApiMapgen::Initialize(L, top);
	ModApiParticles::Initialize(L, top);
	ModApiRollback::Initialize(L, top);
	ModApiServer::Initialize(L, top);
	ModApiUtil::Initialize(L, top);
	ModApiHttp::Initialize(L, top);
	ModApiStorage::Initialize(L, top);
	ModApiChannels::Initialize(L, top);
}
