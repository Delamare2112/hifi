//
//  SpeechRecognitionScriptingInterface.h
//  interface/src/scripting
//
//  Created by Trevor Berninger on 3/20/17.
//  Copyright 2017 Limitless ltd.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <src/InterfaceLogging.h>
#include <QJsonDocument>
#include <QJsonArray>
#include <src/ui/AvatarInputs.h>
#include <QtConcurrent/QtConcurrentRun>
#include "LimitlessVoiceRecognitionScriptingInterface.h"

LimitlessVoiceRecognitionScriptingInterface::LimitlessVoiceRecognitionScriptingInterface() :
        _shouldStartListeningForVoice(false)
{
    connect(&_voiceTimer, &QTimer::timeout, this, &LimitlessVoiceRecognitionScriptingInterface::voiceTimeout);
    connect(&connection, &LimitlessConnection::onReceivedTranscription, this, [this](QString transcription){emit onReceivedTranscription(transcription);});
    connect(&connection, &LimitlessConnection::onFinishedSpeaking, this, [this](QString transcription){emit onFinishedSpeaking(transcription);});
    connection.moveToThread(&_connectionThread);
    _connectionThread.setObjectName("Limitless Connection");
    _connectionThread.start();
}

void LimitlessVoiceRecognitionScriptingInterface::update() {
    const float audioLevel = AvatarInputs::getInstance()->loudnessToAudioLevel(DependencyManager::get<AudioClient>()->getAudioAverageInputLoudness());

    if (_shouldStartListeningForVoice) {
        if (connection._streamingAudioForTranscription) {
            if (audioLevel > 0.33f) {
                if (_voiceTimer.isActive()) {
                    _voiceTimer.stop();
                }
            } else {
                _voiceTimer.start(2000);
            }
        } else if (audioLevel > 0.33f) {
            qCDebug(interfaceapp) << "Starting to listen";
            // to make sure invoke doesn't get called twice before the method actually gets called
            connection._streamingAudioForTranscription = true;
            QMetaObject::invokeMethod(&connection, "startListening", Q_ARG(QString, authCode));
        }
    }
}

void LimitlessVoiceRecognitionScriptingInterface::setListeningToVoice(bool listening) {
    _shouldStartListeningForVoice = listening;
}

void LimitlessVoiceRecognitionScriptingInterface::setAuthKey(QString key) {
    authCode = key;
}

void LimitlessVoiceRecognitionScriptingInterface::voiceTimeout() {
    qCDebug(interfaceapp) << "Timeout timer called";
    if (connection._streamingAudioForTranscription) {
        QMetaObject::invokeMethod(&connection, "stopListening");
        qCDebug(interfaceapp) << "Timeout!";
    }
    _voiceTimer.stop();
}
