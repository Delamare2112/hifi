//
//  NPC_AC.js
//  scripts/interaction
//
//  Created by Trevor Berninger on 3/20/17.
//  Copyright 2017 Limitless ltd.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

var currentlyUsedIndices = [];
var currentlyEngaged = false;
var questionNumber = 0;
function getRandomRiddle() {
	var randIndex = null;
	do {
		randIndex = Math.floor(Math.random() * 15) + 1;
	}while(randIndex in currentlyUsedIndices)

	currentlyUsedIndices.push(randIndex);
	return randIndex.toString();
}

Script.include("file:///home/delamare/gitclones/hifi/unpublishedScripts/interaction/NPCHelpers_v8.js", function(){print("NPCHelpers included.");main();});

var idleAnim = "https://storage.googleapis.com/limitlessserv-144100.appspot.com/hifi%20assets/idle.fbx";
var FST = "https://storage.googleapis.com/limitlessserv-144100.appspot.com/hifi%20assets/Animation/myYisup.fst";

Agent.isAvatar = true;
Avatar.skeletonModelURL = FST;
Avatar.displayName = "NPC";
// Avatar.position = {x: 13444.3555, y: 3.5, z: -427.1562};
Avatar.orientation = {x: 0.707, y: 0, z: 0, w: 0.707};
Avatar.position = {x: -1400.1, y: 52, z: -1280.5};
Avatar.dimensions = {x: 10.0499, y: 10.4895, z: 2.3335};

var startingOrientation = Avatar.orientation;
Messages.subscribe("interactionComs");

var gem = Entities.addEntity({position: {x: -1400.1, y: 52, z: -1280.5}, type: "Sphere", color: {red:200,green:200,blue:200}});
function updateGem() {
	if(audioInjector) {
		var colorVal = (audioInjector.loudness + 0.3) * 255;
		Entities.editEntity(gem, {color: {red:colorVal,green:0,blue:0}});
	}
}

function main() {
	storyURL = "https://storage.googleapis.com/limitlessserv-144100.appspot.com/hifi%20assets/Sphinx_t7.json";
	Messages.messageReceived.connect(function (channel, message, sender) {
		print(sender + " -> NPC @" + Agent.sessionUUID + ": " + message);
		if(channel === "interactionComs" && strContains(message, Agent.sessionUUID)) {
			if(strContains(message, "onFocused")) {
				blocked = false;
				currentlyEngaged = true;
				currentlyUsedIndices = [];
				doActionFromServer("start");
			}
			else if (strContains(message, "onLostFocused")) {
				blocked = false;
				Avatar.orientation = startingOrientation;
				npcContinueStory('', 'https://storage.googleapis.com/limitlessserv-144100.appspot.com/hifi%20assets/beingOfLightWave.fbx', 'start');
			}
			else if (strContains(message, "speaking")) {
				// resetIgnoreTimer();
			}
			else {
				var voiceDataIndex = message.search("voiceData");
				if (voiceDataIndex != -1) {
					var words = message.substr(voiceDataIndex+10);
					if(strContains(words, "repeat") || (strContains(words, "say") && strContains(words, "again"))) {
						doActionFromServer("init");
					} 
					else {
						doActionFromServer("words", words);
					}
				}
			}
		}
	});
	Script.update.connect(updateGem);
	playAnim(idleAnim, true);
}
