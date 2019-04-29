#pragma once

typedef struct {
	int mList;
	int mNumber;
	int mNumber2;
} CollisionData;

void loadGameCollisions();
int getPlayerCollisionList();
int getPlayerCollectionCollisionList();
int getBackgroundCollisionList();
int getHurtingCollisionList();
int getSwitchCollisionList();
int getCollectableCollisionList();
int getFinalCollectableCollisionList();
