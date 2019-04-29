#pragma once

#include <string>
#include <vector>

#include <prism/actorhandler.h>
#include <prism/geometry.h>

ActorBlueprint getQuestGiver();

int isInSpeakingRange();
void addQuestGiver(std::string tName, Position tPosition, std::vector<std::string> openingText, std::vector<std::string> talkText, std::vector<std::string> endingText);
void addQuestGiverPlayerAvatar();
int isSpeaking();