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

#include "particles.h"
#include "constants.h"
#include "debug.h"
#include "settings.h"
#include "client/tile.h"
#include "gamedef.h"
#include "collision.h"
#include <stdlib.h>
#include "util/numeric.h"
#include "light.h"
#include "environment.h"
#include "clientmap.h"
#include "mapnode.h"
#include "client.h"

#include "nodedef.h"
class Map;
class IGameDef;
class Environment;

/*
	Utility
*/

v3f random_v3f(v3f min, v3f max)
{
	return v3f( rand()/(float)RAND_MAX*(max.X-min.X)+min.X,
			rand()/(float)RAND_MAX*(max.Y-min.Y)+min.Y,
			rand()/(float)RAND_MAX*(max.Z-min.Z)+min.Z);
}

class digParticleEmitter :  public irr::scene::IParticleEmitter {
private:
        core::array<irr::scene::SParticle> particles;

        u32 number;
        v3f pos;
        u32 emitted;
public:

        digParticleEmitter(v3f pos, int number) : number(number), pos(pos), emitted(0) {}

        s32 emitt(u32 now, u32 timeSinceLastCall, irr::scene::SParticle*& outArray)
        {
                if (emitted > 0) return 0;

                particles.set_used(0);

                irr::scene::SParticle p;
                for(u32 i=0; i<number; ++i)
                {
                        v3f particlepos = v3f(
                                                pos.X + rand() %100 /200. - 0.25,
                                                pos.Y + rand() %100 /200. - 0.25,
                                                pos.Z + rand() %100 /200. - 0.25);

                        p.pos.set(particlepos);

                        v3f velocity((rand() % 100 / 50. - 1) / 1.5,
                                     rand() % 100 / 35.,
                                     (rand() % 100 / 50. - 1) / 1.5);
                        p.vector = velocity/100;
                        p.startVector = p.vector;

                        p.startTime = now;
                        p.endTime = now + 500 + (rand() % (2000 - 500));;

                        p.color = video::SColor(255.0, 255.0, 255.0, 255.0);
                        p.startColor = p.color;
                        float size = rand() % 64 / 51.2;
                        p.startSize = core::dimension2d<f32>(size, size);
                        p.size = p.startSize;
                        particles.push_back(p);
                }
                outArray = particles.pointer();
                emitted += particles.size();

                return particles.size();
        }

        virtual void setDirection( const core::vector3df& newDirection ) {  }
        virtual void setMinParticlesPerSecond( u32 minPPS ) {  }
        virtual void setMaxParticlesPerSecond( u32 maxPPS ) {  }
        virtual void setMinStartColor( const video::SColor& color ) {  }
        virtual void setMaxStartColor( const video::SColor& color ) {  }
        virtual void setMaxLifeTime( const u32 t ) {  }
        virtual void setMinLifeTime( const u32 t ) { }
        virtual u32 getMaxLifeTime() const { return 1; }
        virtual u32 getMinLifeTime() const { return 1; }
        virtual void setMaxAngleDegrees(const s32 t ) {  }
        virtual s32 getMaxAngleDegrees() const { return 0; }
        virtual void setMaxStartSize( const core::dimension2df& size ) { }
        virtual void setMinStartSize( const core::dimension2df& size ) {  }
        virtual void setCenter( const core::vector3df& center ) {  }
        virtual void setRadius( f32 radius ) {  }
        virtual const core::vector3df& getDirection() const { return v3f(0.0, 0.0, 0.0); }
        virtual u32 getMinParticlesPerSecond() const { return 1; }
        virtual u32 getMaxParticlesPerSecond() const { return 1; }
        virtual const video::SColor& getMinStartColor() const { return video::SColor(1); }
        virtual const video::SColor& getMaxStartColor() const { return video::SColor(1);; }
        virtual const core::dimension2df& getMaxStartSize() const { return core::dimension2d<f32>(0.2, 0.2); }
        virtual const core::dimension2df& getMinStartSize() const { return core::dimension2d<f32>(0.2, 0.2); }
        virtual const core::vector3df& getCenter() const { return v3f(0.0, 0.0, 0.0); }
        virtual f32 getRadius() const { return 1.; }
        virtual irr::scene::E_PARTICLE_EMITTER_TYPE getType() const {
                return (irr::scene::E_PARTICLE_EMITTER_TYPE) 666;
        }

        void restart() { emitted=0; }

};

class CollisionAffector : public irr::scene::IParticleAffector
{
public:
        CollisionAffector(IGameDef *gamedef, ClientEnvironment &env)
        {
                m_gamedef = gamedef;
                m_env = &env;
        }
        void affect(u32 now, irr::scene::SParticle* particlearray, u32 count)
        {
                if( LastTime == 0 )
                {
                        LastTime = now;
                        return;
                }
                f32 timeDelta = ( now - LastTime ) / 1000.0f;

                LastTime = now;
                if( !Enabled )
                        return;
                for(u32 i=0; i<count; ++i)
                {
                        irr::scene::SParticle p = particlearray[i];
                        float size = p.size.Width;
                        core::aabbox3d<f32> box = core::aabbox3d<f32>
                                        (-size/2,-size/2,-size/2,size/2,size/2,size/2);

                        v3f acc = v3f(0.0, 0.0, 0.0);

                        //to ensure particles collide with correct position
                        v3f off = intToFloat(m_env->getCameraOffset(), BS);

                        particlearray[i].pos += off;

                        collisionMoveSimple(m_env, m_gamedef,
                                            BS * 0.5, box,
                                            0, timeDelta,
                                            particlearray[i].pos,
                                            particlearray[i].vector,
                                            acc);

                        particlearray[i].pos -= off;

//                        try{
//                                v3s16 p = v3s16(
//                                                        floor(pos.X+0.5),
//                                                        floor(pos.Y+0.5),
//                                                        floor(pos.Z+0.5)
//                                                        );
//                                MapNode n = m_env->getClientMap().getNode(p);
//                                light = n.getLightBlend(m_env->getDayNightRatio(), m_gamedef->ndef());
//                        }
//                        catch(InvalidPositionException &e){
//                                light = blend_light(m_env->getDayNightRatio(), LIGHT_SUN, 0);
//                        }
//                        u8 plight = decode_light(light);
//                        particlearray[i].color = video::SColor(255, plight, plight, plight);
                }
        }
        irr::scene::E_PARTICLE_AFFECTOR_TYPE getType() const {
                return (irr::scene::E_PARTICLE_AFFECTOR_TYPE) 666;
        }
        irr::u32 LastTime;
        IGameDef *m_gamedef;
        ClientEnvironment *m_env;
};

class DeleteDoneAffector : public irr::scene::IParticleAffector
{
public:
        DeleteDoneAffector(irr::scene::IParticleSystemSceneNode* ps, irr::scene::ISceneManager* smgr)
        {
                ParticleSystem = ps;
                SceneManager = smgr;
        }

        void affect(u32 now, irr::scene::SParticle* particlearray, u32 count)
        {
                // if the particle count goes to zero delete the particle system
                if (count<=0) {
                        SceneManager->addToDeletionQueue(ParticleSystem);
                }
        }
        irr::scene::E_PARTICLE_AFFECTOR_TYPE getType() const {
                return (irr::scene::E_PARTICLE_AFFECTOR_TYPE) 667;
        }
private:
        irr::scene::IParticleSystemSceneNode* ParticleSystem;
        irr::scene::ISceneManager* SceneManager;
};

ParticleManager::ParticleManager(ClientEnvironment* env, irr::scene::ISceneManager* smgr) :
	m_env(env),
	m_smgr(smgr)
{}

ParticleManager::~ParticleManager()
{
	clearAll();
}

void ParticleManager::step(float dtime)
{
	// update position and handle expired spawners

	v3s16 offset = m_env->getCameraOffset();

	JMutexAutoLock lock(m_spawner_list_lock);
	for(std::list<s32>::iterator i =
			particlespawners.begin();
			i != particlespawners.end(); i++)
	{
		scene::ISceneNode *node = m_smgr->getSceneNodeFromId(i->second);
		v3f pos = node->getPosition();
		if(node)
			node->setPosition(pos * BS - intToFloat(offset, BS));
		// TODO: expired
		i++;
	}
	return;
}

void ParticleManager::clearAll ()
{
	JMutexAutoLock lock(m_spawner_list_lock);
	for(std::list<s32>::iterator i =
			particlespawners.begin();
			i != particlespawners.end();)
	{
		scene::ISceneNode *node = m_smgr->getSceneNodeFromId(i->second);
		if(node)
			m_smgr->addToDeletionQueue(node);
		irrlicht_spawners.erase(i++);
	}
}

void ParticleManager::handleParticleEvent(ClientEvent *event, IGameDef *gamedef,
					  scene::ISceneManager* smgr, LocalPlayer *player)
{

	if (event->type == CE_DELETE_PARTICLESPAWNER) {
		JMutexAutoLock lock(m_spawner_list_lock);
		s32 id = event->delete_particlespawner.id;

		std::list<s32>::iterator find = std::find(particlespawners.begin(), particlespawners.end(), id);
		if (find != particlespawners.end())
		{
			scene::ISceneNode *node = smgr->getSceneNodeFromId(id);
			if(node)
				smgr->addToDeletionQueue(node);

			particlespawners.erase(find);
		}
		case CE_ADD_PARTICLESPAWNER: {
			{
				MutexAutoLock lock(m_spawner_list_lock);
				if (m_particle_spawners.find(event->add_particlespawner.id) !=
						m_particle_spawners.end()) {
					delete m_particle_spawners.find(event->add_particlespawner.id)->second;
					m_particle_spawners.erase(event->add_particlespawner.id);
				}
			}

			video::ITexture *texture =
				gamedef->tsrc()->getTextureForMesh(*(event->add_particlespawner.texture));

			ParticleSpawner* toadd = new ParticleSpawner(gamedef, smgr, player,
					event->add_particlespawner.amount,
					event->add_particlespawner.spawntime,
					*event->add_particlespawner.minpos,
					*event->add_particlespawner.maxpos,
					*event->add_particlespawner.minvel,
					*event->add_particlespawner.maxvel,
					*event->add_particlespawner.minacc,
					*event->add_particlespawner.maxacc,
					event->add_particlespawner.minexptime,
					event->add_particlespawner.maxexptime,
					event->add_particlespawner.minsize,
					event->add_particlespawner.maxsize,
					event->add_particlespawner.collisiondetection,
					event->add_particlespawner.collision_removal,
					event->add_particlespawner.attached_id,
					event->add_particlespawner.vertical,
					texture,
					event->add_particlespawner.id,
					this);

			/* delete allocated content of event */
			delete event->add_particlespawner.minpos;
			delete event->add_particlespawner.maxpos;
			delete event->add_particlespawner.minvel;
			delete event->add_particlespawner.maxvel;
			delete event->add_particlespawner.minacc;
			delete event->add_particlespawner.texture;
			delete event->add_particlespawner.maxacc;

		s32 id = event->add_particlespawner.id;
		std::list<s32>::iterator find = std::find(particlespawners.begin(), particlespawners.end(), id);
		if (find != particlespawners.end())
		{
			scene::ISceneNode *node = smgr->getSceneNodeFromId(id);
			if(node)
				smgr->addToDeletionQueue(node);

			particlespawners.erase(find);
		}

		video::ITexture *texture =
				gamedef->tsrc()->getTexture(*(event->add_particlespawner.texture));

		scene::IParticleSystemSceneNode * ps = smgr->addParticleSystemSceneNode(false);

		float pps = event->add_particlespawner.amount;
		float time = event->add_particlespawner.spawntime;

		if (time != 0)
			pps = pps / time;

		float minsize = event->add_particlespawner.minsize;
		float maxsize = event->add_particlespawner.maxsize;

		scene::IParticlePointEmitter * em = ps->createPointEmitter(
					random_v3f(v3f(0,0,0), v3f(1, 1, 1))/10,
					5,
					20,
					video::SColor(255.0, 255.0, 255.0, 255.0), //mincol,
					video::SColor(255.0, 255.0, 255.0, 255.0), //maxcol,
					event->add_particlespawner.minexptime*1000,
					event->add_particlespawner.maxexptime*1000,
					360, //maxdeg,
					core::dimension2d<f32>(minsize, minsize),
					core::dimension2d<f32>(maxsize, maxsize));


		ps->setEmitter(em);
		em->drop();

		ps->setMaterialTexture(0, texture);

		v3f pos = *event->add_particlespawner.minpos;
		pos = pos*BS - intToFloat(m_env->getCameraOffset(), BS);
		ps->setPosition(pos);

		ps->setMaterialFlag(video::EMF_LIGHTING, false);
		ps->setMaterialFlag(video::EMF_BACK_FACE_CULLING, false);
		ps->setMaterialFlag(video::EMF_BILINEAR_FILTER, false);
		ps->setMaterialFlag(video::EMF_FOG_ENABLE, true);
		ps->setMaterialType(video::EMT_TRANSPARENT_ALPHA_CHANNEL);

		if (time != 0) {

			scene::ISceneNodeAnimator* foo =  smgr->createDeleteAnimator(time * 1000);
			ps->addAnimator(foo);
			foo->drop();
		}

		{
			JMutexAutoLock lock(m_spawner_list_lock);
			particlespawners.push_back(ps->getID());
		}

			delete event->spawn_particle.pos;
			delete event->spawn_particle.vel;
			delete event->spawn_particle.acc;

	if (event->type == CE_SPAWN_PARTICLE) {
		//		video::ITexture *texture =
		//			gamedef->tsrc()->getTexture(*(event->spawn_particle.texture));

		//		Particle* toadd = new Particle(gamedef, smgr, player, m_env,
		//				*event->spawn_particle.pos,
		//				*event->spawn_particle.vel,
		//				*event->spawn_particle.acc,
		//				event->spawn_particle.expirationtime,
		//				event->spawn_particle.size,
		//				event->spawn_particle.collisiondetection,
		//				event->spawn_particle.vertical,
		//				texture,
		//				v2f(0.0, 0.0),
		//				v2f(1.0, 1.0));

		//		addParticle(toadd);

		//		delete event->spawn_particle.pos;
		//		delete event->spawn_particle.vel;
		//		delete event->spawn_particle.acc;

		return;
	}
}

void ParticleManager::addDiggingParticles(IGameDef* gamedef, scene::ISceneManager* smgr,
		LocalPlayer *player, v3s16 pos, const TileSpec tiles[])
{
		addNodeParticle(gamedef, smgr, player, pos, tiles, 32);
}

void ParticleManager::addPunchingParticles(IGameDef* gamedef, scene::ISceneManager* smgr,
		LocalPlayer *player, v3s16 pos, const TileSpec tiles[])
{
	addNodeParticle(gamedef, smgr, player, pos, tiles, 1);
}

void ParticleManager::addNodeParticle(IGameDef* gamedef, scene::ISceneManager* smgr,
		LocalPlayer *player, v3s16 pos, const TileSpec tiles[], int number)
{
	// Texture
	u8 texid = myrand_range(0, 5);
	video::ITexture *texture;

	//to ensure particles are at correct position
	v3s16 camera_offset = m_env->getCameraOffset();
	v3f particlepos = intToFloat(pos, BS) - intToFloat(camera_offset, BS);

	scene::IParticleSystemSceneNode* ps =
			smgr->addParticleSystemSceneNode(false);
	scene::IParticleEmitter* em;

	em = new digParticleEmitter(particlepos, number);
	ps->setEmitter(em);
	em->drop();

	irr::scene::IParticleAffector* fu = ps->createGravityAffector(v3f(0.0, -0.1, 0.0), 2000);
	ps->addAffector(fu);
	fu->drop();

	irr::scene::IParticleAffector* puf = new CollisionAffector(gamedef, *m_env);
	ps->addAffector(puf);
	puf->drop();

	scene::IParticleAffector* paf = new DeleteDoneAffector(ps, smgr);
	ps->addAffector(paf);
	paf->drop();

	ps->setMaterialFlag(video::EMF_LIGHTING, false);
	ps->setMaterialFlag(video::EMF_ZWRITE_ENABLE, true );
	ps->setMaterialTexture(0, texture);
}
