#pragma once

#include <prism/actorhandler.h>
#include <prism/geometry.h>

void addGate(Position tPosition, int tIndex);
void removeGates(int tIndex);
ActorBlueprint getGateHandler();