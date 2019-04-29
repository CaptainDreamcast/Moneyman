#include "player.h"

#include <algorithm>

#include <prism/blitz.h>

#include "level.h"
#include "collision.h"
#include "questgiver.h"
#include "gamescreen.h"
#include "gatehandler.h"

static struct {
	MugenSpriteFile mSprites;
	MugenAnimations mAnimations;
	MugenSounds mSounds;
	int mCurrentState;
	int mEntityID;
	int mCollisionID;
	int mCollisionCollectID;
	CollisionData mCollisionData;
	CollisionData mCollectionCollisionData;

	int mIsFinished;

	int mIsDead;
	int mIsInvincible;
	int mInvincibleNow;
} gPlayer;

static void playerHitCB(void* tCaller, void* tCollisionData);
static void playerCollectedCB(void* tCaller, void* tCollisionData);

static void loadPlayer(void* tData) {
	(void)tData;
	gPlayer.mSprites = loadMugenSpriteFileWithoutPalette("levels/PLAYER.sff");
	gPlayer.mAnimations = loadMugenAnimationFile("levels/PLAYER.air");
	gPlayer.mSounds = loadMugenSoundFile("levels/SOUND.snd");

	gPlayer.mCurrentState = 1;
	Position pos = getPlayerStartPosition();
	pos.z = 10;
	gPlayer.mEntityID = addBlitzEntity(pos);
	addBlitzMugenAnimationComponent(gPlayer.mEntityID, &gPlayer.mSprites, &gPlayer.mAnimations, 11);
	addBlitzPhysicsComponent(gPlayer.mEntityID);
	setBlitzPhysicsGravity(gPlayer.mEntityID, makePosition(0, 0.2, 0));
	setBlitzPhysicsDragFactorOnCollision(gPlayer.mEntityID, makePosition(0.2, 0, 0));


	gPlayer.mCollisionData.mList = getPlayerCollisionList();
	addBlitzCollisionComponent(gPlayer.mEntityID);
	gPlayer.mCollisionID = addBlitzCollisionRect(gPlayer.mEntityID, getPlayerCollisionList(), makeCollisionRect(makePosition(-8, -45, 0), makePosition(8, 0, 0)));
	setBlitzCollisionSolid(gPlayer.mEntityID, gPlayer.mCollisionID, 1);
	setBlitzCollisionCollisionData(gPlayer.mEntityID, gPlayer.mCollisionID, &gPlayer.mCollisionData);
	addBlitzCollisionCB(gPlayer.mEntityID, gPlayer.mCollisionID, playerHitCB, NULL);

	gPlayer.mCollectionCollisionData.mList = getPlayerCollectionCollisionList();
	gPlayer.mCollisionCollectID = addBlitzCollisionRect(gPlayer.mEntityID, getPlayerCollectionCollisionList(), makeCollisionRect(makePosition(-12, -55, 0), makePosition(12, 0, 0)));
	setBlitzCollisionCollisionData(gPlayer.mEntityID, gPlayer.mCollisionCollectID, &gPlayer.mCollectionCollisionData);
	addBlitzCollisionCB(gPlayer.mEntityID, gPlayer.mCollisionCollectID, playerCollectedCB, NULL);

	gPlayer.mIsFinished = 0;
	gPlayer.mIsInvincible = 0;
	gPlayer.mIsDead = 0;
}

static int getCurrentAnimationBase() {
	return gPlayer.mCurrentState * 10;
}

static void changeState(int tDelta) {
	int currentAnimation = getBlitzMugenAnimationAnimationNumber(gPlayer.mEntityID);
	currentAnimation -= getCurrentAnimationBase();
	gPlayer.mCurrentState = clamp(gPlayer.mCurrentState + tDelta, 0, 2);

	if (gPlayer.mCurrentState == 2) {
		currentAnimation = 1;
	}

	changeBlitzMugenAnimationIfDifferent(gPlayer.mEntityID, getCurrentAnimationBase() + currentAnimation);

	if (gPlayer.mCurrentState && tDelta == 1) {
		changeBlitzCollisionRect(gPlayer.mEntityID, gPlayer.mCollisionID, makeCollisionRect(makePosition(-8, -45, 0), makePosition(8, 0, 0)));
		changeBlitzCollisionRect(gPlayer.mEntityID, gPlayer.mCollisionCollectID, makeCollisionRect(makePosition(-12, -55, 0), makePosition(12, 0, 0)));
	} else if (!gPlayer.mCurrentState && tDelta == -1) {
		changeBlitzCollisionRect(gPlayer.mEntityID, gPlayer.mCollisionID, makeCollisionRect(makePosition(-8, -22, 0), makePosition(8, 0, 0)));
		changeBlitzCollisionRect(gPlayer.mEntityID, gPlayer.mCollisionCollectID, makeCollisionRect(makePosition(-12, -27, 0), makePosition(12, 0, 0)));
	}
}

static void restartAfterDeath(void* tCaller) {
	(void)tCaller;
	setNewScreen(getGameScreen());
}

static void dieForReal() {
	setBlitzMugenAnimationTransparency(gPlayer.mEntityID, 0);
	int deathEntity = addBlitzEntity(getBlitzEntityPosition(gPlayer.mEntityID));
	addBlitzMugenAnimationComponent(deathEntity, getPlayerSprites(), getPlayerAnimations(), 30);
	gPlayer.mIsDead = 1;
	addTimerCB(180, restartAfterDeath, NULL);
}

static void die() {
	if (gPlayer.mIsInvincible || gPlayer.mIsDead) return;
	if (gPlayer.mCurrentState > 0) {
		tryPlayMugenSound(&gPlayer.mSounds, 1, 3);
		changeState(-1);
		gPlayer.mIsInvincible = 1;
		gPlayer.mInvincibleNow = 0;
		setBlitzMugenAnimationTransparency(gPlayer.mEntityID, 0.7);
	}
	else {
		tryPlayMugenSound(&gPlayer.mSounds, 1, 3);
		dieForReal();
	}
}

static void activateSwitch(int mLocation, int mIndex) {
	int y = mLocation / 100;
	int x = mLocation % 100;

	removeLevelTile(x, y);
	removeGates(mIndex);
}

static void playerHitCB(void* tCaller, void* tCollisionData) {
	(void)tCaller;

	CollisionData* collisionData = (CollisionData*)tCollisionData;
	int listID = collisionData->mList;
	if (listID == getHurtingCollisionList()) {
		die();
	}
	else if (listID == getSwitchCollisionList() && gPlayer.mCurrentState == 2) {
		activateSwitch(collisionData->mNumber, collisionData->mNumber2);
	}

}

static void collect() {
	changeState(1);
}

static void playerCollectedCB(void* tCaller, void* tCollisionData) {
	(void)tCaller;

	tryPlayMugenSound(&gPlayer.mSounds, 1, 0);

	CollisionData* collisionData = (CollisionData*)tCollisionData;
	int listID = collisionData->mList;
	if (listID == getCollectableCollisionList()) {
		collect();
	}
	else if (listID == getFinalCollectableCollisionList()) {
		gPlayer.mIsFinished = 1;
	}
}

static void updatePlayerMovementGeneral(double tSpeed, double tJumpForce) {

	if (hasPressedLeft()) {
		if (getMugenAnimationAnimationNumber(getBlitzMugenAnimationID(gPlayer.mEntityID)) == getCurrentAnimationBase() + 1) changeBlitzMugenAnimation(gPlayer.mEntityID, getCurrentAnimationBase() + 2);
		setMugenAnimationFaceDirection(getBlitzMugenAnimationID(gPlayer.mEntityID), 0);
		setBlitzPhysicsVelocityX(gPlayer.mEntityID, -tSpeed);
	}
	if (hasPressedRight()) {
		if (getMugenAnimationAnimationNumber(getBlitzMugenAnimationID(gPlayer.mEntityID)) == getCurrentAnimationBase() + 1) changeBlitzMugenAnimation(gPlayer.mEntityID, getCurrentAnimationBase() + 2);
		setMugenAnimationFaceDirection(getBlitzMugenAnimationID(gPlayer.mEntityID), 1);
		setBlitzPhysicsVelocityX(gPlayer.mEntityID, tSpeed);
	}

	if (!hasPressedLeft() && !hasPressedRight()) {
		if (getMugenAnimationAnimationNumber(getBlitzMugenAnimationID(gPlayer.mEntityID)) == getCurrentAnimationBase() + 2) changeBlitzMugenAnimation(gPlayer.mEntityID, getCurrentAnimationBase() + 1);
		setBlitzPhysicsVelocityX(gPlayer.mEntityID, 0);
	}

	if (!isInSpeakingRange() && hasPressedAFlank() && getMugenAnimationAnimationNumber(getBlitzMugenAnimationID(gPlayer.mEntityID)) != getCurrentAnimationBase() + 3 && tJumpForce) {
		tryPlayMugenSound(&gPlayer.mSounds, 1, 1);
		changeBlitzMugenAnimation(gPlayer.mEntityID, getCurrentAnimationBase() + 3);
		setBlitzPhysicsVelocityY(gPlayer.mEntityID, -tJumpForce);
	}
}

static void updatePlayerMovement0() {
	updatePlayerMovementGeneral(3, 3);
}

static void updatePlayerMovement1() {
	updatePlayerMovementGeneral(3, 3.6);
}

static void updatePlayerMovement2() {
	updatePlayerMovementGeneral(2.5, 0);
}

static void constraintPlayerPosition() {
	Position* p = getBlitzEntityPositionReference(gPlayer.mEntityID);
	p->x = fmax(0.0, p->x);
	p->y = fmax(240 - getLevelHeight() + 16, p->y);
}

static void updatePlayerGravity() {
	Position vel = getBlitzPhysicsVelocity(gPlayer.mEntityID);
	if (vel.y > 0) setBlitzPhysicsGravity(gPlayer.mEntityID, makePosition(0, 0.2, 0));
	else setBlitzPhysicsGravity(gPlayer.mEntityID, makePosition(0, 0.1, 0));
}

static void updatePlayerCamera() {
	Position pos = getBlitzEntityPosition(gPlayer.mEntityID);
	setBlitzCameraPositionBasedOnCenterPoint(vecSub(pos, makePosition(0, 16, 0)));
}

static void updatePlayerAnimation() {
	if (getMugenAnimationAnimationNumber(getBlitzMugenAnimationID(gPlayer.mEntityID)) == getCurrentAnimationBase() + 3 && hasBlitzCollidedBottom(gPlayer.mEntityID) && getBlitzPhysicsVelocityY(gPlayer.mEntityID) >= 0) {
		tryPlayMugenSound(&gPlayer.mSounds, 1, 1);
		changeBlitzMugenAnimation(gPlayer.mEntityID, getCurrentAnimationBase() + 1);
	}
}
static void updatePlayerInvincible() {
	if (!gPlayer.mIsInvincible) return;

	setBlitzMugenAnimationTransparency(gPlayer.mEntityID, randfrom(0.3, 0.7));
	if (gPlayer.mInvincibleNow++ > 120) {
		gPlayer.mIsInvincible = 0;
		setBlitzMugenAnimationTransparency(gPlayer.mEntityID, 1);
	}
}

static void updatePlayerLose() {
	auto pos = getBlitzEntityPosition(gPlayer.mEntityID);
	if (pos.y > 240) {
		dieForReal();
	}
}

static void updatePlayerHandler(void* tData) {
	(void)tData;
	if (gPlayer.mIsDead) return;

	updatePlayerAnimation();
	if (!isSpeaking()) {
		if (gPlayer.mCurrentState == 0) {
			updatePlayerMovement0();
		}
		else if (gPlayer.mCurrentState == 1) {
			updatePlayerMovement1();
		}
		else if (gPlayer.mCurrentState == 2) {
			updatePlayerMovement2();
		}
	}
	constraintPlayerPosition();
	updatePlayerGravity();
	updatePlayerCamera();
	updatePlayerInvincible();
	updatePlayerLose();
}

ActorBlueprint getPlayerHandler()
{
	return makeActorBlueprint(loadPlayer, NULL, updatePlayerHandler);
}

Position getPlayerPosition()
{
	return getBlitzEntityPosition(gPlayer.mEntityID);
}

int isPlayerFinished()
{
	return gPlayer.mIsFinished;
}

MugenSpriteFile * getPlayerSprites()
{
	return &gPlayer.mSprites;
}

MugenAnimations * getPlayerAnimations()
{
	return &gPlayer.mAnimations;
}

MugenSounds * getPlayerSounds()
{
	return &gPlayer.mSounds;
}

void stopPlayer()
{
	setBlitzPhysicsVelocity(gPlayer.mEntityID, makePosition(0, 0, 0));
}
