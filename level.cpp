#include "level.h"

#include <prism/blitz.h>

#include "collision.h"
#include "collectablehandler.h"
#include "questgiver.h"
#include "gatehandler.h"

static struct {
	MugenSpriteFile mSprites;
	MugenAnimations mAnimations;
	int mCurrentLevel;

	int mWidth;
	int mHeight;
	int mLevelEntityIDs[100][100];
	int mShowDialogFlag;

	int mHasPlayerStartTile;
	Vector3DI mPlayerStartTile;

	CollisionData mBackgroundCollisionData;
	CollisionData mObstacleCollisionData;
	CollisionData mSwitchCollisionData[100][100];

	int mBackgroundID;
} gData;

static void loadAnimationFromLevelTile(int i, int* oAnimation, int* oHorizontalMirror, int* oVerticalMirror) {
	if (i == 10) {
		*oAnimation = 10;
		*oHorizontalMirror = 0;
		*oVerticalMirror = 0;
	}
	else if (i == 20) {
		*oAnimation = 20;
		*oHorizontalMirror = 0;
		*oVerticalMirror = 0;
	} 
	else {
			*oAnimation = 30;
			*oHorizontalMirror = 0;
			*oVerticalMirror = 0;
	}

}

static void setCollisionFromIndex(int tEntityID, Position tPos, int tIndex, int tHorizontalMirror, int tVerticalMirror, int x, int y, int tTrueIndex) {
	if (tIndex >= 204) return;

	addBlitzCollisionComponent(tEntityID);

	int collisionList;
	CollisionData* collisionData;
	int collisionID;
	if (tIndex < 20) {
		CollisionRect rect;
		if (tHorizontalMirror && tVerticalMirror) rect = makeCollisionRect(makePosition(-32, -32, 0), makePosition(0, 0, 0));
		if (!tHorizontalMirror && tVerticalMirror) rect = makeCollisionRect(makePosition(0, -32, 0), makePosition(32, 0, 0));
		if (tHorizontalMirror && !tVerticalMirror) rect = makeCollisionRect(makePosition(-32, 0, 0), makePosition(0, 32, 0));
		if (!tHorizontalMirror && !tVerticalMirror) rect = makeCollisionRect(makePosition(0, 0, 0), makePosition(32, 32, 0));
		collisionList = getBackgroundCollisionList();
		collisionData = &gData.mBackgroundCollisionData;
		collisionID = addBlitzCollisionRect(tEntityID, collisionList, rect);
		setBlitzCollisionSolid(tEntityID, collisionID, 0);
	}
	else if (tIndex < 30) {
		CollisionRect rect = makeCollisionRect(makePosition(5, 10, 0), makePosition(27, 27, 0));
		collisionList = getHurtingCollisionList();
		collisionData = &gData.mObstacleCollisionData;
		collisionID = addBlitzCollisionRect(tEntityID, collisionList, rect);
	}
	else if (tIndex < 40) {
		CollisionRect rect = makeCollisionRect(makePosition(5, 10, 0), makePosition(27, 27, 0));
		collisionList = getSwitchCollisionList();
		gData.mSwitchCollisionData[y][x].mList = collisionList;
		gData.mSwitchCollisionData[y][x].mNumber = y * 100 + x;
		gData.mSwitchCollisionData[y][x].mNumber2 = tTrueIndex - 30;
		collisionData = &gData.mSwitchCollisionData[y][x];
		collisionID = addBlitzCollisionRect(tEntityID, collisionList, rect);
	}
	else {
		logErrorFormat("Unimplemented index: %d.", tIndex);
		abortSystem();
		collisionID = -1;
		collisionData = &gData.mBackgroundCollisionData;
	}

	setBlitzCollisionCollisionData(tEntityID, collisionID, collisionData);
}

static vector<string> readText(BufferPointer* p) {
	vector<string> ret;
	int n = readIntegerFromTextStreamBufferPointer(p);
	readLineFromTextStreamBufferPointer(p);
	for (int i = 0; i < n; i++) {
		string line = readLineFromTextStreamBufferPointer(p);
		ret.push_back(line);
	}

	return ret;
}

static void loadLevelTiles() {

	char path[1024];
	sprintf(path, "levels/%d.txt", gData.mCurrentLevel);
	Buffer b = fileToBuffer(path);
	BufferPointer p = getBufferPointer(b);

	string questGiverName = readLineFromTextStreamBufferPointer(&p);

	auto openingText = readText(&p);
	auto talkingText = readText(&p);
	auto endingText = readText(&p);

	gData.mWidth = readIntegerFromTextStreamBufferPointer(&p) * 32;
	gData.mHeight = readIntegerFromTextStreamBufferPointer(&p) * 32;

	Position startPosition = makePosition(0, 240 - gData.mHeight, 5);
	int tileSizeX = gData.mWidth / 32;
	int tileSizeY = gData.mHeight / 32;
	int y, x;
	for (y = 0; y < tileSizeY; y++) {
		for (x = 0; x < tileSizeX; x++) {
			int index = readIntegerFromTextStreamBufferPointer(&p);
			if (!index) continue;
			if (index == -1) {
				gData.mHasPlayerStartTile = 1;
				gData.mPlayerStartTile = makeVector3DI(x, y, 0);
				continue;
			}
			if (index == -2) {
				addCollectable(vecAdd(startPosition, makePosition(x * 32 + 16, y * 32 + 16, 11)));
				continue;
			}
			if (index == -3) {
				addQuestGiver(questGiverName, vecAdd(startPosition, makePosition(x * 32 + 16, y * 32 + 32, 4)), openingText, talkingText, endingText);
				continue;
			}
			if (index == -4) {
				addFinalCollectable(vecAdd(startPosition, makePosition(x * 32 + 16, y * 32 + 16, 11)));
				continue;
			}
			if (index < 0) {
				addGate(vecAdd(startPosition, makePosition(x * 32, y * 32, 11)), -index);
				continue;
			}

			int animation, horizontalMirror, verticalMirror;
			loadAnimationFromLevelTile(index, &animation, &horizontalMirror, &verticalMirror);
			Position pos = vecAdd(startPosition, makePosition((x + horizontalMirror) * 32, (y + verticalMirror) * 32, 0));
			gData.mLevelEntityIDs[y][x] = addBlitzEntity(pos);
			addBlitzMugenAnimationComponent(gData.mLevelEntityIDs[y][x], &gData.mSprites, &gData.mAnimations, animation);
			setMugenAnimationFaceDirection(getBlitzMugenAnimationID(gData.mLevelEntityIDs[y][x]), !horizontalMirror);
			setMugenAnimationVerticalFaceDirection(getBlitzMugenAnimationID(gData.mLevelEntityIDs[y][x]), !verticalMirror);
			setCollisionFromIndex(gData.mLevelEntityIDs[y][x], pos, animation, horizontalMirror, verticalMirror, x, y, index);
		}
	}

	freeBuffer(b);

	if (!gData.mHasPlayerStartTile) {
		logWarning("Unable to find player start tile.");
	}
}

static void loadLevelBackground() {
	auto animation = createOneFrameMugenAnimationForSprite(1, 0);
	auto backgroundSize = getAnimationFirstElementSpriteSize(animation, &gData.mSprites);
	gData.mBackgroundID = addMugenAnimation(animation, &gData.mSprites, makePosition(0, 0, 1));
}

static void loadCollisionData() {
	gData.mBackgroundCollisionData.mList = getBackgroundCollisionList();
	gData.mObstacleCollisionData.mList = getHurtingCollisionList();
}

static void loadLevelHandler(void* tData) {
	(void)tData;
	char path[1024];
	sprintf(path, "levels/%d.sff", gData.mCurrentLevel);
	gData.mSprites = loadMugenSpriteFileWithoutPalette(path);
	sprintf(path, "levels/%d.air", gData.mCurrentLevel);
	gData.mAnimations = loadMugenAnimationFile(path);

	loadCollisionData();
	loadLevelTiles();
	loadLevelBackground();

	setBlitzCameraHandlerRange(makeGeoRectangle(0, 240 - gData.mHeight, gData.mWidth, gData.mHeight));
}

ActorBlueprint getLevelHandler()
{
	return makeActorBlueprint(loadLevelHandler);
}

int getLevelHeight()
{
	return gData.mHeight;
}

Position getPlayerStartPosition()
{
	return makePosition(gData.mPlayerStartTile.x * 32 + 16, (240 - gData.mHeight) + gData.mPlayerStartTile.y * 32 + 31, 0);
}

MugenSpriteFile * getLevelSprites()
{
	return &gData.mSprites;
}

MugenAnimations * getLevelAnimations()
{
	return &gData.mAnimations;
}

void increaseLevel()
{
	gData.mCurrentLevel++;
	gData.mShowDialogFlag = 1;
}

void resetLevel()
{
	gData.mCurrentLevel = 0;
	gData.mShowDialogFlag = 1;
}

void removeLevelTile(int x, int y)
{
	removeBlitzEntity(gData.mLevelEntityIDs[y][x]);
}

int getShowDialog()
{
	return gData.mShowDialogFlag;
}

void resetShowDialog()
{
	gData.mShowDialogFlag = 0;
}
