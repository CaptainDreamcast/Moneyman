#include "questgiver.h"

#include <prism/blitz.h>

#include "level.h"
#include "player.h"
#include "gamescreen.h"
#include "storyscreen.h"

static struct {
	int mEntityID;
	int mSpeakToID;
	int mAvatarAnimationID[2];
	int mTextBoxID;
	int mTextBoxNameID;
	int mTextBoxTextID;
	string mName;

	std::vector<std::string> mOpeningText;
	std::vector<std::string> mTalkText;
	std::vector<std::string> mEndingText;

	int mIsSpeaking;
	int mTextPosition;
	std::vector<std::string> mActiveText;
	int mIsOver;
} gQuestGiver;

static void loadQuestGiver(void* tData) {
	(void)tData;
	gQuestGiver.mIsOver = 0;
	gQuestGiver.mIsSpeaking = 0;
}

static void stopSpeaking() {
	for (int i = 0; i < 2; i++) {
		setMugenAnimationInvisible(gQuestGiver.mAvatarAnimationID[i]);
	}

	setMugenAnimationInvisible(gQuestGiver.mTextBoxID);
	setMugenTextVisibility(gQuestGiver.mTextBoxNameID, 0);
	setMugenTextVisibility(gQuestGiver.mTextBoxTextID, 0);
	gQuestGiver.mIsSpeaking = 0;
}

static void setAnimationsVisible() {
	for (int i = 0; i < 2; i++) {
		setMugenAnimationVisibility(gQuestGiver.mAvatarAnimationID[i], 1);
	}
	setMugenAnimationVisibility(gQuestGiver.mTextBoxID, 1);

	setMugenTextVisibility(gQuestGiver.mTextBoxNameID, 1);
	setMugenTextVisibility(gQuestGiver.mTextBoxTextID, 1);
}

static void goToNextScreen(void* tCaller) {
	(void)tCaller;
	increaseLevel();
	setNewScreen(getGameScreen());
}

static void goToOutro(void* tCaller) {
	(void)tCaller;
	setCurrentStoryDefinitionFile("levels/OUTRO.def", 3);
	setNewScreen(getStoryScreen());
}

static void loadNextSpeakingStep() {
	while (true) {
		gQuestGiver.mTextPosition++;
		stopSpeaking();
		if (gQuestGiver.mTextPosition >= int(gQuestGiver.mActiveText.size())) {
			return;
		}
		setAnimationsVisible();

		string& step = gQuestGiver.mActiveText[gQuestGiver.mTextPosition];
		int type, speaker;
		sscanf(step.data(), "%d %d", &type, &speaker);
		const char* textStart = strchr(strchr(step.data(), ' ') + 1, ' ') + 1;

		if (type == 0) {
			string text = string(textStart);
			changeMugenText(gQuestGiver.mTextBoxNameID, speaker ?  "David" : gQuestGiver.mName.data());
			changeMugenText(gQuestGiver.mTextBoxTextID, text.data());
			setMugenTextBuildup(gQuestGiver.mTextBoxTextID, 1);
			gQuestGiver.mIsSpeaking = 1;
			return;
		}
		else if (type == 1) {
			string text = string(textStart);

			int animation;
			if (text == "normal") animation = speaker ? 2000 : 2003;
			else if (text == "happy") animation = speaker ? 2002 : 2004;
			else if (text == "sad") animation = speaker ? 2004 : 2005;
			else if (text == "angry") animation = speaker ? 2001 : 2006;
			else if (text == "confused") animation = speaker ? 2003 : 2003;
			else if (text == "shocked") animation = speaker ? 2005 : 2003;
			else animation= atoi(textStart);
			changeMugenAnimation(gQuestGiver.mAvatarAnimationID[speaker], getMugenAnimation(speaker ? getPlayerAnimations() : getLevelAnimations(), animation));
		}
		else if (type == 2) {
			addFadeOut(20, goToNextScreen, NULL);
			gQuestGiver.mIsOver = 1;
			return;
		}
		else if (type == 3) {
			addFadeOut(20, goToOutro, NULL);
			gQuestGiver.mIsOver = 1;
			return;
		}
		else {
			logErrorFormat("Unrecognized text type %s", step.data());
			return;
		}
	}

}

static void updateSpeaking() {
	if (!gQuestGiver.mIsSpeaking) return;

	if (hasPressedAFlank()) {
		tryPlayMugenSound(getPlayerSounds(), 1, 2);
		if (isMugenTextBuiltUp(gQuestGiver.mTextBoxTextID)) {
			loadNextSpeakingStep();
		}
		else {
			setMugenTextBuiltUp(gQuestGiver.mTextBoxTextID);
		}
	}
}

int isInSpeakingRange() {
	return fabs((getPlayerPosition() - getBlitzEntityPosition(gQuestGiver.mEntityID)).x) < 50;
}

static void startSpeaking() {
	gQuestGiver.mTextPosition = -1;
	if (getShowDialog()) {
		gQuestGiver.mActiveText = gQuestGiver.mOpeningText;
	}
	else if (isPlayerFinished()) {
		gQuestGiver.mActiveText = gQuestGiver.mEndingText;
	}
	else {
		gQuestGiver.mActiveText = gQuestGiver.mTalkText;
	}
	stopPlayer();
	loadNextSpeakingStep();
	resetShowDialog();
	gQuestGiver.mIsSpeaking = 1;
}

static void updateSpeakingStart() {
	if (gQuestGiver.mIsSpeaking) return;

	if (getShowDialog() || (isInSpeakingRange() && hasPressedAFlank())) {
		startSpeaking();
	}
}

static void updateCanSpeak() {
	setMugenAnimationVisibility(gQuestGiver.mSpeakToID, !gQuestGiver.mIsSpeaking && isInSpeakingRange());
}

static void updateQuestGiver(void* tData) {
	(void)tData;
	if (gQuestGiver.mIsOver) return;
	updateSpeaking();
	updateSpeakingStart();
	updateCanSpeak();

}

ActorBlueprint getQuestGiver()
{
	return makeActorBlueprint(loadQuestGiver, NULL, updateQuestGiver);
}

void addQuestGiver(string tName, Position tPosition, std::vector<std::string> openingText, std::vector<std::string> talkText, std::vector<std::string> endingText)
{
	gQuestGiver.mEntityID = addBlitzEntity(tPosition);
	addBlitzMugenAnimationComponent(gQuestGiver.mEntityID, getLevelSprites(), getLevelAnimations(), 2000);

	gQuestGiver.mSpeakToID = addMugenAnimation(getMugenAnimation(getLevelAnimations(), 2002), getLevelSprites(), makePosition(35, 160, 12));
	setMugenAnimationInvisible(gQuestGiver.mSpeakToID);

	gQuestGiver.mAvatarAnimationID[0] = addMugenAnimation(getMugenAnimation(getLevelAnimations(), 2001), getLevelSprites(), makePosition(80, 170, 49));
	setMugenAnimationInvisible(gQuestGiver.mAvatarAnimationID[0]);
	
	gQuestGiver.mTextBoxID = addMugenAnimation(getMugenAnimation(getLevelAnimations(), 2001), getLevelSprites(), makePosition(160, 220, 50));
	setMugenAnimationInvisible(gQuestGiver.mTextBoxID);
	gQuestGiver.mTextBoxNameID = addMugenTextMugenStyle("", makePosition(45, 151, 51), makeVector3DI(2, 0, 1));
	setMugenTextVisibility(gQuestGiver.mTextBoxNameID, 0);
	gQuestGiver.mTextBoxTextID = addMugenTextMugenStyle("", makePosition(45, 165, 51), makeVector3DI(1, 7, 1));
	setMugenTextVisibility(gQuestGiver.mTextBoxTextID, 0);
	setMugenTextTextBoxWidth(gQuestGiver.mTextBoxTextID, 240);

	gQuestGiver.mName = tName;
	gQuestGiver.mOpeningText = openingText;
	gQuestGiver.mTalkText = talkText;
	gQuestGiver.mEndingText = endingText;
}

void addQuestGiverPlayerAvatar()
{
	gQuestGiver.mAvatarAnimationID[1] = addMugenAnimation(getMugenAnimation(getPlayerAnimations(), 2001), getPlayerSprites(), makePosition(240, 170, 49));
	setMugenAnimationInvisible(gQuestGiver.mAvatarAnimationID[1]);
}

int isSpeaking()
{
	return gQuestGiver.mIsSpeaking;
}
