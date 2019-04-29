#pragma once

#include <prism/actorhandler.h>
#include <prism/geometry.h>
#include <prism/mugenanimationhandler.h>

ActorBlueprint getLevelHandler();

int getLevelHeight();
Position getPlayerStartPosition();
MugenSpriteFile* getLevelSprites();
MugenAnimations* getLevelAnimations();
void increaseLevel();
void resetLevel();
void removeLevelTile(int x, int y);
int getShowDialog();
void resetShowDialog();