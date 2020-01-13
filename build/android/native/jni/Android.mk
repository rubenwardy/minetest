LOCAL_PATH := $(call my-dir)/..

#LOCAL_ADDRESS_SANITIZER:=true

include $(CLEAR_VARS)
LOCAL_MODULE := Curl
LOCAL_SRC_FILES := deps/Android/Curl/${NDK_TOOLCHAIN_VERSION}/$(APP_ABI)/libcurl.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := Freetype
LOCAL_SRC_FILES := deps/Android/Freetype/${NDK_TOOLCHAIN_VERSION}/$(APP_ABI)/libfreetype.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := Irrlicht
LOCAL_SRC_FILES := deps/Android/Irrlicht/${NDK_TOOLCHAIN_VERSION}/$(APP_ABI)/libIrrlicht.a
include $(PREBUILT_STATIC_LIBRARY)

#include $(CLEAR_VARS)
#LOCAL_MODULE := LevelDB
#LOCAL_SRC_FILES := deps/Android/LevelDB/${NDK_TOOLCHAIN_VERSION}/$(APP_ABI)/libleveldb.a
#include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := LuaJIT
LOCAL_SRC_FILES := deps/Android/LuaJIT/${NDK_TOOLCHAIN_VERSION}/$(APP_ABI)/libluajit.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := OpenAL
LOCAL_SRC_FILES := deps/Android/OpenAL-Soft/${NDK_TOOLCHAIN_VERSION}/$(APP_ABI)/libopenal.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := Vorbis
LOCAL_SRC_FILES := deps/Android/Vorbis/${NDK_TOOLCHAIN_VERSION}/$(APP_ABI)/libvorbis.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := Minetest

LOCAL_CFLAGS += \
	-DJSONCPP_NO_LOCALE_SUPPORT      \
	-DHAVE_TOUCHSCREENGUI            \
	-DENABLE_GLES=1                  \
	-DUSE_CURL=1                     \
	-DUSE_SOUND=1                    \
	-DUSE_FREETYPE=1                 \
	-DUSE_LEVELDB=0                  \
	-DUSE_LUAJIT=1                   \
	-DVERSION_MAJOR=${versionMajor}  \
	-DVERSION_MINOR=${versionMinor}  \
	-DVERSION_PATCH=${versionPatch}  \
	$(GPROF_DEF)

ifdef NDEBUG
	LOCAL_CFLAGS += -DNDEBUG=1
endif

ifdef GPROF
	GPROF_DEF := -DGPROF
	PROFILER_LIBS := android-ndk-profiler
	LOCAL_CFLAGS += -pg
endif

LOCAL_C_INCLUDES := \
	../../../src                                     \
	../../../src/script                              \
	../../../lib/gmp                                 \
	../../../lib/jsoncpp                             \
	deps/Android/Curl/include                        \
	deps/Android/Freetype/include                    \
	deps/Android/Irrlicht/include                    \
	deps/Android/LevelDB/include                     \
	deps/Android/libiconv/include                    \
	deps/Android/libiconv/libcharset/include         \
	deps/Android/LuaJIT/src                          \
	deps/Android/OpenAL-Soft/include                 \
	deps/Android/sqlite                              \
	deps/Android/Vorbis/include

LOCAL_SRC_FILES := \
	$(wildcard  ../../../src/client/*.cpp)           \
	$(wildcard  ../../../src/client/*/*.cpp)         \
	../../../src/content/content.cpp                 \
	../../../src/content_mapnode.cpp                 \
	../../../src/content/mods.cpp                    \
	../../../src/content_nodemeta.cpp                \
	../../../src/content/packages.cpp                \
	../../../src/content_sao.cpp                     \
	../../../src/content/subgames.cpp                \
	../../../src/database/database.cpp               \
	../../../src/database/database-dummy.cpp         \
	../../../src/database/database-files.cpp         \
	../../../src/database/database-sqlite3.cpp       \
	$(wildcard  ../../../src/gui/*.cpp)              \
	$(wildcard  ../../../src/irrlicht_changes/*.cpp) \
	../../../src/mapgen/cavegen.cpp                  \
	../../../src/mapgen/dungeongen.cpp               \
	../../../src/mapgen/mapgen_carpathian.cpp        \
	../../../src/mapgen/mapgen.cpp                   \
	../../../src/mapgen/mapgen_flat.cpp              \
	../../../src/mapgen/mapgen_fractal.cpp           \
	../../../src/mapgen/mapgen_singlenode.cpp        \
	../../../src/mapgen/mapgen_v5.cpp                \
	../../../src/mapgen/mapgen_v6.cpp                \
	../../../src/mapgen/mapgen_v7.cpp                \
	../../../src/mapgen/mapgen_valleys.cpp           \
	../../../src/mapgen/mg_biome.cpp                 \
	../../../src/mapgen/mg_decoration.cpp            \
	../../../src/mapgen/mg_ore.cpp                   \
	../../../src/mapgen/mg_schematic.cpp             \
	../../../src/mapgen/treegen.cpp                  \
	$(wildcard  ../../../src/network/*.cpp)          \
	$(wildcard  ../../../src/script/*.cpp)           \
	$(wildcard  ../../../src/script/*/*.cpp)         \
	$(wildcard  ../../../src/threading/*.cpp)        \
	$(wildcard  ../../../src/util/*.c)               \
	$(wildcard  ../../../src/util/*.cpp)             \
	../../../src/server/activeobjectmgr.cpp          \
	../../../src/server/mods.cpp                     \
	../../../src/ban.cpp                             \
	../../../src/chat.cpp                            \
	../../../src/clientiface.cpp                     \
	../../../src/collision.cpp                       \
	../../../src/convert_json.cpp                    \
	../../../src/craftdef.cpp                        \
	../../../src/debug.cpp                           \
	../../../src/defaultsettings.cpp                 \
	../../../src/emerge.cpp                          \
	../../../src/environment.cpp                     \
	../../../src/face_position_cache.cpp             \
	../../../src/filesys.cpp                         \
	../../../src/genericobject.cpp                   \
	../../../src/gettext.cpp                         \
	../../../src/httpfetch.cpp                       \
	../../../src/hud.cpp                             \
	../../../src/inventory.cpp                       \
	../../../src/inventorymanager.cpp                \
	../../../src/itemdef.cpp                         \
	../../../src/itemstackmetadata.cpp               \
	../../../src/light.cpp                           \
	../../../src/log.cpp                             \
	../../../src/main.cpp                            \
	../../../src/mapblock.cpp                        \
	../../../src/map.cpp                             \
	../../../src/mapnode.cpp                         \
	../../../src/mapsector.cpp                       \
	../../../src/map_settings_manager.cpp            \
	../../../src/metadata.cpp                        \
	../../../src/modchannels.cpp                     \
	../../../src/nameidmapping.cpp                   \
	../../../src/nodedef.cpp                         \
	../../../src/nodemetadata.cpp                    \
	../../../src/nodetimer.cpp                       \
	../../../src/noise.cpp                           \
	../../../src/objdef.cpp                          \
	../../../src/object_properties.cpp               \
	../../../src/pathfinder.cpp                      \
	../../../src/player.cpp                          \
	../../../src/porting_android.cpp                 \
	../../../src/porting.cpp                         \
	../../../src/profiler.cpp                        \
	../../../src/raycast.cpp                         \
	../../../src/reflowscan.cpp                      \
	../../../src/remoteplayer.cpp                    \
	../../../src/rollback.cpp                        \
	../../../src/rollback_interface.cpp              \
	../../../src/serialization.cpp                   \
	../../../src/server.cpp                          \
	../../../src/serverenvironment.cpp               \
	../../../src/serverlist.cpp                      \
	../../../src/serverobject.cpp                    \
	../../../src/settings.cpp                        \
	../../../src/staticobject.cpp                    \
	../../../src/tileanimation.cpp                   \
	../../../src/tool.cpp                            \
	../../../src/translation.cpp                     \
	../../../src/version.cpp                         \
	../../../src/voxelalgorithms.cpp                 \
	../../../src/voxel.cpp

# Build with LevelDB is disabled
#	../../../src/database/database-leveldb.cpp

# GMP
LOCAL_SRC_FILES += ../../../lib/gmp/mini-gmp.c

# JSONCPP
LOCAL_SRC_FILES += ../../../lib/jsoncpp/jsoncpp.cpp

# iconv
LOCAL_SRC_FILES += \
	deps/Android/libiconv/lib/iconv.c                \
	deps/Android/libiconv/libcharset/lib/localcharset.c

# SQLite3
LOCAL_SRC_FILES += deps/Android/sqlite/sqlite3.c

LOCAL_STATIC_LIBRARIES += Curl Freetype Irrlicht OpenAL Vorbis LuaJIT android_native_app_glue $(PROFILER_LIBS) #LevelDB

LOCAL_LDLIBS := -lEGL -lGLESv1_CM -lGLESv2 -landroid -lOpenSLES

include $(BUILD_SHARED_LIBRARY)

ifdef GPROF
$(call import-module,android-ndk-profiler)
endif
$(call import-module,android/native_app_glue)
