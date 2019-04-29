#include "collision.h"

#include <prism/blitz.h>

static struct {
	int mPlayerList;
	int mPlayerCollectionList;
	int mBackgroundList;
	int mHurtingList;
	int mSwitchList;
	int mCollectableList;
	int mFinalCollectableList;
} gCollision;

void loadGameCollisions()
{
	gCollision.mPlayerList = addCollisionListToHandler();
	gCollision.mPlayerCollectionList = addCollisionListToHandler();
	gCollision.mBackgroundList = addCollisionListToHandler();
	gCollision.mHurtingList = addCollisionListToHandler();
	gCollision.mSwitchList = addCollisionListToHandler();
	gCollision.mCollectableList = addCollisionListToHandler();
	gCollision.mFinalCollectableList = addCollisionListToHandler();
	addCollisionHandlerCheck(gCollision.mPlayerList, gCollision.mBackgroundList);
	addCollisionHandlerCheck(gCollision.mPlayerList, gCollision.mSwitchList);
	addCollisionHandlerCheck(gCollision.mPlayerCollectionList, gCollision.mCollectableList);
	addCollisionHandlerCheck(gCollision.mPlayerCollectionList, gCollision.mFinalCollectableList);
	addCollisionHandlerCheck(gCollision.mPlayerList, gCollision.mHurtingList);
}

int getPlayerCollisionList()
{
	return gCollision.mPlayerList;
}

int getPlayerCollectionCollisionList()
{
	return gCollision.mPlayerCollectionList;
}

int getBackgroundCollisionList()
{
	return gCollision.mBackgroundList;
}

int getHurtingCollisionList()
{
	return gCollision.mHurtingList;
}

int getSwitchCollisionList()
{
	return gCollision.mSwitchList;
}

int getCollectableCollisionList()
{
	return gCollision.mCollectableList;
}

int getFinalCollectableCollisionList()
{
	return gCollision.mFinalCollectableList;
}
