#include "collectablehandler.h"

#include <prism/blitz.h>

#include "level.h"
#include "collision.h"

typedef struct {
	int mID;
	int mEntityID;
	CollisionData mCollisionData;
} Collectable;

static struct {
	map<int, Collectable> mCollectables;
} gCollectableHandler;

static void loadCollectableHandler(void* tData) {
	(void)tData;
	gCollectableHandler.mCollectables.clear();
}

static void unloadCollectableHandler(void* tData) {
	(void)tData;
	gCollectableHandler.mCollectables.clear();
}

ActorBlueprint getCollectableHandler()
{
	return makeActorBlueprint(loadCollectableHandler, unloadCollectableHandler);
}

static void collectableCB(void* tCaller, void* tCollisionData) {
	Collectable* e = (Collectable*)tCaller;
	(void)tCollisionData;
	removeBlitzEntity(e->mEntityID);
	gCollectableHandler.mCollectables.erase(e->mID);
}

static void addCollectableGeneral(Position tPosition, int tList, int tAnimation) {
	int id = stl_int_map_push_back(gCollectableHandler.mCollectables, Collectable());
	Collectable& e = gCollectableHandler.mCollectables[id];
	e.mID = id;
	e.mEntityID = addBlitzEntity(tPosition);
	addBlitzMugenAnimationComponent(e.mEntityID, getLevelSprites(), getLevelAnimations(), tAnimation);
	e.mCollisionData.mList = tList;
	addBlitzCollisionComponent(e.mEntityID);
	int collisionID = addBlitzCollisionRect(e.mEntityID, tList, makeCollisionRect(makePosition(-12, -12, 0), makePosition(12, 12, 1)));
	setBlitzCollisionCollisionData(e.mEntityID, collisionID, &e.mCollisionData);
	addBlitzCollisionCB(e.mEntityID, collisionID, collectableCB, &e);
}

void addCollectable(Position tPosition)
{
	addCollectableGeneral(tPosition, getCollectableCollisionList(), 1000);
}

void addFinalCollectable(Position tPosition)
{
	addCollectableGeneral(tPosition, getFinalCollectableCollisionList(), 1001);

}