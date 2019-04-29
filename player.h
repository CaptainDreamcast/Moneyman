#pragma once

#include <prism/actorhandler.h>
#include <prism/geometry.h>
#include <prism/mugenanimationhandler.h>
#include <prism/mugensoundfilereader.h>

ActorBlueprint getPlayerHandler();

Position getPlayerPosition();
int isPlayerFinished();
MugenSpriteFile* getPlayerSprites();
MugenAnimations* getPlayerAnimations();
MugenSounds* getPlayerSounds();
void stopPlayer();