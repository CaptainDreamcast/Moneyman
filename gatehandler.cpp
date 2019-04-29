#include "gatehandler.h"

#include <prism/blitz.h>

#include "level.h"
#include "collision.h"

typedef struct {
	int mEntityID;
	int mNumber;
	CollisionData mCollisionData;
} Gate;

static struct {
	list<Gate> mGates;

} gGateHandler;

void addGate(Position tPosition, int tIndex)
{
	gGateHandler.mGates.push_back(Gate());
	Gate& e = gGateHandler.mGates.back();
	e.mNumber = tIndex;
	e.mEntityID = addBlitzEntity(tPosition);
	addBlitzMugenAnimationComponent(e.mEntityID, getLevelSprites(), getLevelAnimations(), 40);
	e.mCollisionData.mList = getBackgroundCollisionList();
	addBlitzCollisionComponent(e.mEntityID);
	int collisionID = addBlitzCollisionRect(e.mEntityID, getBackgroundCollisionList(), makeCollisionRect(makePosition(0, 0, 0), makePosition(32, 32, 1)));
	setBlitzCollisionCollisionData(e.mEntityID, collisionID, &e.mCollisionData);
	setBlitzCollisionSolid(e.mEntityID, collisionID, 0);
}

typedef struct {
	int which;

} SearchCaller;

static int checkSingleGate(SearchCaller* caller, Gate& gate) {
	if (gate.mNumber == caller->which) {
		removeBlitzEntity(gate.mEntityID);
		return 1;
	}
	return 0;
}

void removeGates(int tIndex)
{
	SearchCaller which;
	which.which = tIndex;

	stl_list_remove_predicate(gGateHandler.mGates, checkSingleGate, &which);
}

static void loadGateHandler(void* tData) {
	(void)tData;
	gGateHandler.mGates.clear();
}

ActorBlueprint getGateHandler()
{
	return makeActorBlueprint(loadGateHandler);
}
