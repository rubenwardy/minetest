/*
Minetest
Copyright (C) 2019  rubenwardy <rw@rubenwardy.com>

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

#include "client/renderingengine.h"
#include "gui/guiEngine.h"
#include "test.h"
#include "gui/guiFormSpecMenu.h"


class TestFormspec : public TestBase
{
public:
	TestFormspec() { TestManager::registerTestModule(this); }
	const char *getName() { return "TestFormspec"; }

	void runTests(IGameDef *gamedef);
	void testRenderKernel();
};

static TestFormspec g_test_instance;

void TestFormspec::runTests(IGameDef *gamedef)
{
	TEST(testRenderKernel)
}

using namespace irr;

class DummyEventReceiver : public IEventReceiver {
public:
	bool OnEvent(const SEvent &event) override {
		return false;
	}
};

class DummyMenuMgr : public IMenuManager {
public:
	void createdMenu(gui::IGUIElement *menu) override {}
	void deletingMenu(gui::IGUIElement *menu) override {}
};

class DummyTextDest : public TextDest {
public:
	void gotText(const StringMap &fields) override {}
};

void TestFormspec::testRenderKernel()
{
//	auto device = createDevice( video::EDT_NULL, core::dimension2d<u32>(640, 480), 16,
//					false, false, false, 0);

	DummyEventReceiver evtrec;
	RenderingEngine engine{&evtrec};
	auto gui = RenderingEngine::get_gui_env();
	auto driver = engine.getVideoDriver();

	JoystickController joystick{};
	DummyMenuMgr menumgr;
	MenuTextureSource tsrc{driver};
	DummyTextDest txt_dst;

	auto menu = new GUIFormSpecMenu(&joystick, gui->getRootGUIElement(), -1, &menumgr, nullptr, &tsrc, nullptr, &txt_dst, "", false);
	menu->setFormSpec("size[3,3]button[1,1;1,1;h;Hello]", {});

	auto texture = driver->addRenderTargetTexture(core::dimension2d<u32>(256,256), "RTT1");

	driver->beginScene(true, true, 0);
	driver->setRenderTarget(texture, true, true, video::SColor(0,0,0,255));
	menu->draw();
	driver->endScene();

	video::IImage *image = driver->createImageFromData(texture->getColorFormat(),
			texture->getSize(), texture->lock(), false);
	texture->unlock();
	driver->writeImageToFile(image, "src/gui/unittest/test_formspec/1_out.png");
	image->drop();

	// Note: GUIFormSpecMenu is dropped by IGUIEnvironment
}
