#include "titlescreen.h"

#include <prism/blitz.h>

#include "gamescreen.h"
#include "level.h"

static struct {
	MugenSpriteFile mSprites;

} gTitleScreenData;

static void loadTitleScreen() {
	gTitleScreenData.mSprites = loadMugenSpriteFileWithoutPalette("levels/TITLE.sff");
	addMugenAnimation(createOneFrameMugenAnimationForSprite(1, 0), &gTitleScreenData.mSprites, makePosition(0, 0, 1));
	addFadeIn(30, NULL, NULL);
	setWrapperTitleScreen(getTitleScreen());
	playTrack(3);
}


static void gotoGameScreen(void* tCaller) {
	(void)tCaller;
	resetLevel();
	setNewScreen(getGameScreen());
}

static void updateTitleScreen() {

	if (hasPressedAFlank() || hasPressedStartFlank()) {
		addFadeOut(30, gotoGameScreen, NULL);
	}
}

static Screen gTitleScreen;

Screen* getTitleScreen() {
	gTitleScreen = makeScreen(loadTitleScreen, updateTitleScreen);
	return &gTitleScreen;
};
