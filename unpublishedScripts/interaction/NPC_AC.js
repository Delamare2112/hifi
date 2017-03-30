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
var timers = [];
var currentlyEngaged = false;
var questionNumber = 0;
var heartbeatTimeout = false;
function getRandomRiddle() {
    var randIndex = null;
    do {
        randIndex = Math.floor(Math.random() * 15) + 1;
    } while (randIndex in currentlyUsedIndices);

    currentlyUsedIndices.push(randIndex);
    return randIndex.toString();
}

Script.include("https://raw.githubusercontent.com/Delamare2112/hifi/Interaction/unpublishedScripts/interaction/NPCHelpers.js", function(){
    print("NPCHelpers included.");main();
});

var idleAnim = "https://storage.googleapis.com/limitlessserv-144100.appspot.com/hifi%20assets/idle.fbx";
var FST = "https://s3.amazonaws.com/hifi-public/tony/fixed-sphinx/sphinx.fst";

Agent.isAvatar = true;
Avatar.skeletonModelURL = FST;
Avatar.displayName = "NPC";
Avatar.position = {x: 0.3, y: -23.4, z: 8.0};
Avatar.orientation = {x: 0, y: 1, z: 0, w: 0};
// Avatar.position = {x: 1340.3555, y: 4.078, z: -420.1562};
// Avatar.orientation = {x: 0, y: -0.707, z: 0, w: 0.707};
Avatar.scale = 2;

Messages.subscribe("interactionComs");

var gem = Entities.addEntity({
    localPosition: {x:1,y:0,z:0}, 
    parentJointIndex: Avatar.getJointIndex('HeadTop_End'), 
    type: "Box", 
    color: {red:100,green:200,blue:200}, 
    parentID: Agent.sessionUUID,
    dimensions: {x: 0.15, y: 0.15, z: 0.15},
    rotation: Quat.fromPitchYawRollDegrees(-46.8985, 1.6406, 27.0456)
});

function updateGem() {
    if (audioInjector) {
        var colorVal = (audioInjector.loudness + 0.3) * 255;
        Entities.editEntity(gem, {color: {red:100,green:colorVal,blue:200}});
    }
}

function endInteraction() {
    if(_qid == "Restarting")
        return;
    print("ending interaction");
    blocked = false;
    currentlyEngaged = false;
    if(audioInjector)
        audioInjector.stop();
    for (var t in timers) {
        Script.clearTimeout(timers[t]);
    }
    npcRespondBlocking(
        'https://storage.googleapis.com/limitlessserv-144100.appspot.com/hifi%20assets/ScratchDialogue/EarlyExit_0' + (Math.floor(Math.random() * 2) + 1).toString() + '.wav',
        'https://storage.googleapis.com/limitlessserv-144100.appspot.com/hifi%20assets/Animation/reversedSphinx.fbx', 
        function(){
            Avatar.startAnimation('https://storage.googleapis.com/limitlessserv-144100.appspot.com/hifi%20assets/Animation/Hifi_Sphinx_Anim_Entrance_Kneel_Combined_with_Intro.fbx', 0);
        }
    );
}

function main() {
    storyURL = "https://storage.googleapis.com/limitlessserv-144100.appspot.com/hifi%20assets/Sphinx_t16.json";
    Messages.messageReceived.connect(function (channel, message, sender) {
        print(sender + " -> NPC @" + Agent.sessionUUID + ": " + message);
        if (channel === "interactionComs" && strContains(message, Agent.sessionUUID)) {
            if (strContains(message, 'beat')) {
                if(heartbeatTimeout) {
                    Script.clearTimeout(heartbeatTimeout);
                    heartbeatTimeout = false;
                }
                heartbeatTimeout = Script.setTimeout(endInteraction, 1500);
            }
            else if (strContains(message, "onFocused") && !currentlyEngaged) {
                blocked = false;
                currentlyEngaged = true;
                currentlyUsedIndices = [];
                doActionFromServer("start");
            } else if (strContains(message, "leftArea")) {
                // endInteraction();
            } else if (strContains(message, "speaking")) {
                // resetIgnoreTimer();
            } else {
                var voiceDataIndex = message.search("voiceData");
                if (voiceDataIndex != -1) {
                    var words = message.substr(voiceDataIndex+10);
                    if (!isNaN(_qid) && (strContains(words, "repeat") || (strContains(words, "say") && strContains(words, "again")))) {
                        doActionFromServer("init");
                    } else {
                        doActionFromServer("words", words);
                    }
                }
            }
        }
    });
    // Script.update.connect(updateGem);
    Avatar.startAnimation("https://storage.googleapis.com/limitlessserv-144100.appspot.com/hifi%20assets/Animation/Hifi_Sphinx_Anim_Entrance_Kneel_Combined_with_Intro.fbx", 0);
}
