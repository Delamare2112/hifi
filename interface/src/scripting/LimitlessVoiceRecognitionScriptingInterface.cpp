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
    _voiceTimer.setSingleShot(true);
    connect(&_voiceTimer, &QTimer::timeout, this, &LimitlessVoiceRecognitionScriptingInterface::voiceTimeout);
    connect(&_connection, &LimitlessConnection::onReceivedTranscription, this, [this](QString transcription){emit onReceivedTranscription(transcription);});
    connect(&_connection, &LimitlessConnection::onFinishedSpeaking, this, [this](QString transcription){emit onFinishedSpeaking(transcription);});
    _connection.moveToThread(&_connectionThread);
    _connectionThread.setObjectName("Limitless Connection");
    _connectionThread.start();
}

void LimitlessVoiceRecognitionScriptingInterface::update() {
    const float audioLevel = AvatarInputs::getInstance()->loudnessToAudioLevel(DependencyManager::get<AudioClient>()->getAudioAverageInputLoudness());

    if (_shouldStartListeningForVoice) {
        if (_connection._streamingAudioForTranscription) {
            if (audioLevel > 0.33f) {
                if (_voiceTimer.isActive()) {
                    _voiceTimer.stop();
                }
            } else if (!_voiceTimer.isActive()){
                _voiceTimer.start(2000);
            }
        } else if (audioLevel > 0.33f) {
            qCDebug(interfaceapp) << "Starting to listen";
            // to make sure invoke doesn't get called twice before the method actually gets called
            _connection._streamingAudioForTranscription = true;
            QMetaObject::invokeMethod(&_connection, "startListening", Q_ARG(QString, authCode));
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
    if (_connection._streamingAudioForTranscription) {
        QMetaObject::invokeMethod(&_connection, "stopListening");
        qCDebug(interfaceapp) << "Timeout!";
    }
}
