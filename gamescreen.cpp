#include "gamescreen.h"

#include <prism/blitz.h>

#include "level.h"
#include "player.h"
#include "collision.h"
#include "questgiver.h"
#include "gatehandler.h"

static struct {
	int dummy;

} gGameScreenData;

static void loadGameScreen() {
	loadGameCollisions();
	instantiateActor(getGateHandler());
	instantiateActor(getQuestGiver());
	instantiateActor(getLevelHandler());
	instantiateActor(getPlayerHandler());
	addQuestGiverPlayerAvatar();

	playTrack(4);
}

static void updateGameScreen() {
	if (hasPressedKeyboardKeyFlank(KEYBOARD_R_PRISM)) {
		setNewScreen(getGameScreen());
	}
}

Screen gGameScreen;

Screen * getGameScreen()
{
	gGameScreen = makeScreen(loadGameScreen, updateGameScreen);
	return &gGameScreen;
}
