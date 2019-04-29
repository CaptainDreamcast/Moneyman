#pragma once

#include <prism/actorhandler.h>
#include <prism/geometry.h>

ActorBlueprint getCollectableHandler();
void addCollectable(Position tPosition);
void addFinalCollectable(Position tPosition);