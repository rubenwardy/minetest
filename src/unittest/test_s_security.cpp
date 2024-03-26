/*
Minetest
Copyright (C) 2024 rubenwardy

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

#include "test.h"
#include "script/cpp_api/s_security.h"
#include "gamedef.h"
#include "content/mods.h"


#define SUBGAME_ID "devtest"

class TestSSecurity : public TestBase
{
public:
	TestSSecurity() { TestManager::registerTestModule(this); }
	const char *getName() { return "TestSSecurity"; }

	void runTests(IGameDef *gamedef);

	void testCheckPath();
};


class MockGameDef : public IGameDef {
public:
	std::vector<ModSpec> mods;
	SubgameSpec game;
	std::string worldpath;

	MockGameDef(): worldpath(porting::path_user + "/worlds/world1") {}

	ModSpec &addMod(const std::string &name, const std::string &path) {
		return mods.emplace_back(name, path, false, path);
	}

	IItemDefManager *getItemDefManager() override {
		throw std::runtime_error("Unimplemented");
	}

	const NodeDefManager *getNodeDefManager() override {
		throw std::runtime_error("Unimplemented");
	}

	ICraftDefManager *getCraftDefManager() override {
		throw std::runtime_error("Unimplemented");
	}

	u16 allocateUnknownNodeId(const std::string &name) override {
		throw std::runtime_error("Unimplemented");
	}

	IRollbackManager *getRollbackManager() override {
		throw std::runtime_error("Unimplemented");
	}

	const std::vector<ModSpec> &getMods() const override {
		return mods;
	}

	const ModSpec *getModSpec(const std::string &modname) const override {
		for (const auto &mod : mods)
			if (mod.name == modname)
				return &mod;

		return nullptr;
	}

	const SubgameSpec *getGameSpec() const override {
		return &game;
	}

	std::string getWorldPath() const override {
		return worldpath;
	}

	ModStorageDatabase *getModStorageDatabase() override {
		throw std::runtime_error("Unimplemented");
	}

	bool joinModChannel(const std::string &channel) override {
		throw std::runtime_error("Unimplemented");
	}

	bool leaveModChannel(const std::string &channel) override {
		throw std::runtime_error("Unimplemented");
	}

	bool sendModChannelMessage(const std::string &channel, const std::string &message) override {
		throw std::runtime_error("Unimplemented");
	}

	ModChannel *getModChannel(const std::string &channel) override {
		throw std::runtime_error("Unimplemented");
	}
};


static TestSSecurity g_test_instance;

void TestSSecurity::runTests(IGameDef *gamedef)
{
	TEST(testCheckPath);
}

void TestSSecurity::testCheckPath() {
	bool write_allowed;

#define canRead(path, modname) ScriptApiSecurity::checkPath(nullptr, path, false, nullptr, &gamedef, modname)
#define writeRequired(path, modname) ScriptApiSecurity::checkPath(nullptr, path, true, nullptr, &gamedef, modname)
#define writeAllowed(path, modname) (ScriptApiSecurity::checkPath(nullptr, path, true, &write_allowed, &gamedef, modname) && write_allowed)
#define canWrite(path, modname) (writeRequired(path, modname) || writeAllowed(path, modname))

	std::string global_mods = porting::path_user + "/mods/";
	std::string devtest_mods = porting::path_user + "/games/devtest/mods/";
	UASSERT(fs::IsDir(devtest_mods));

	MockGameDef gamedef;
	gamedef.game.path =  porting::path_user + "/games/devtest";
	auto modone = gamedef.addMod("soundstuff", devtest_mods + "soundstuff");

	UASSERT(!canRead(porting::path_user, "soundstuff"));
	UASSERT(!canRead(global_mods, "soundstuff"));
	UASSERT(canRead(gamedef.game.path, "soundstuff"));
	UASSERT(!canWrite(gamedef.game.path, "soundstuff"));
	UASSERT(canRead(devtest_mods, "soundstuff"));
	UASSERT(!canWrite(devtest_mods, "soundstuff"));
	UASSERT(canRead(modone.path, "soundstuff"));
	UASSERT(canWrite(modone.path, "soundstuff"));

	/*
	 not g_settings_path
	 removes nonexistent components
	 false on empty path
	 test with builtin (modname == "")
	 write with mods
	 read subgame

	 */
}
