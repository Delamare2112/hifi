#include <QJsonDocument>
#include <QJsonArray>
#include <src/InterfaceLogging.h>
#include <src/ui/AvatarInputs.h>
#include "LimitlessConnection.h"
#include "LimitlessVoiceRecognitionScriptingInterface.h"

LimitlessConnection::LimitlessConnection() :
        _streamingAudioForTranscription(false),
        _authenticated(false)
{
    qCDebug(interfaceapp) << "A connection was constructed";
}

void LimitlessConnection::startListening() {
    if(_streamingAudioForTranscription)
        return;
    qCDebug(interfaceapp) << "Starting to listen from connection";
    _streamingAudioForTranscription = true;
    _transcribeServerSocket.reset(new QTcpSocket(this));
    connect(_transcribeServerSocket.get(), &QTcpSocket::readyRead, this,
            &LimitlessConnection::transcriptionReceived);
    connect(DependencyManager::get<AudioClient>().data(), &AudioClient::inputReceived, this,
            &LimitlessConnection::audioInputReceived);

    qCDebug(interfaceapp) << "Setting up connection";
    static const auto host = "gserv_devel.studiolimitless.com";
    _transcribeServerSocket->connectToHost(host, 1407);
    _transcribeServerSocket->waitForConnected();
    QString requestHeader = QString::asprintf("Authorization: %s\r\nfs: %i\r\n",
                                              DependencyManager::get<LimitlessVoiceRecognitionScriptingInterface>()->authCode.toLocal8Bit().data(),
                                              AudioConstants::SAMPLE_RATE);
    qCDebug(interfaceapp) << "Sending: " << requestHeader;
    _transcribeServerSocket->write(requestHeader.toLocal8Bit());
    _transcribeServerSocket->waitForBytesWritten();
    _transcribeServerSocket->waitForReadyRead(); // Wait for server to tell us if the auth was successful.
}

void LimitlessConnection::stopListening() {
    qCDebug(interfaceapp) << "Stop listening from connection";
    emit onFinishedSpeaking(_currentTranscription);
    _transcribeServerSocket->waitForBytesWritten();
    _transcribeServerSocket->close();
    _currentTranscription = "";
    _authenticated = false;
    _transcribeServerSocket.reset(nullptr);
}

void LimitlessConnection::audioInputReceived(const QByteArray& inputSamples) {
    if (_transcribeServerSocket && _transcribeServerSocket->isWritable()
        && _transcribeServerSocket->state() != QAbstractSocket::SocketState::UnconnectedState) {
        _audioDataBuffer.enqueue(inputSamples);
        _transcribeServerSocket->write(inputSamples.data(), inputSamples.size());
        _transcribeServerSocket->waitForBytesWritten();
    }
}

void LimitlessConnection::transcriptionReceived() {
    qCDebug(interfaceapp) << "transcriptionReceived";
    while (_transcribeServerSocket && _transcribeServerSocket->bytesAvailable() > 0) {
        const QByteArray data = _transcribeServerSocket->readAll();
        qCDebug(interfaceapp) << "Got: " << data;
        _serverDataBuffer.append(data);
        int begin = _serverDataBuffer.indexOf('<');
        int end = _serverDataBuffer.indexOf('>');
        while (begin > -1 && end > -1) {
            const int len = end - begin;
            const QByteArray serverMessage = _serverDataBuffer.mid(begin+1, len-1);
            if (serverMessage.contains("1407")) {
                qCDebug(interfaceapp) << "Limitless Speech Server denied.";
                stopListening();
                return;
            } else if (serverMessage.contains("1408")) {
                qCDebug(interfaceapp) << "Authenticated!";
                _serverDataBuffer.clear();
                _authenticated = true;
                return;
            }
            QJsonObject json = QJsonDocument::fromJson(serverMessage.data()).object();
            _serverDataBuffer.remove(begin, len+1);
            _currentTranscription = json["alternatives"].toArray()[0].toObject()["transcript"].toString();
            emit onReceivedTranscription(_currentTranscription);
            if (json["isFinal"] == true) {
                stopListening();
                qCDebug(interfaceapp) << "Final transcription: " << _currentTranscription;
                return;
            }
            begin = _serverDataBuffer.indexOf('<');
            end = _serverDataBuffer.indexOf('>');
        }
    }
}
