#include "ControllerServer.h"
#include "task.pb.h"

namespace {
QByteArray packEnvelope(const QString& kind, const QByteArray& payload) {
    task::Envelope env;
    env.set_kind(kind.toStdString());
    env.set_payload(payload.data(), static_cast<int>(payload.size()));
    QByteArray bytes;
    bytes.resize(env.ByteSizeLong());
    env.SerializeToArray(bytes.data(), bytes.size());
    return bytes;
}
}

ControllerServer::ControllerServer(QObject* parent)
    : QObject(parent) {
    connect(&_server, &QTcpServer::newConnection, this, &ControllerServer::onNewConnection);
}

ControllerServer::~ControllerServer() {
    close();
}

bool ControllerServer::listen(quint16 port) {
    if (_server.isListening()) {
        _server.close();
    }
    const bool ok = _server.listen(QHostAddress::Any, port);
    if (!ok) {
        emit lastError(QStringLiteral("Не удалось слушать порт %1").arg(port));
    }
    return ok;
}

void ControllerServer::close() {
    if (_socket) {
        _socket->disconnectFromHost();
        _socket->deleteLater();
        _socket = nullptr;
    }
    _channel.reset();
    if (_server.isListening()) {
        _server.close();
    }
    emit agentConnectedChanged(false);
}

void ControllerServer::onNewConnection() {
    if (_socket) {
        // Разрешён только один агент
        auto extra = _server.nextPendingConnection();
        extra->disconnectFromHost();
        extra->deleteLater();
        return;
    }
    _socket = _server.nextPendingConnection();
    _channel = std::make_unique<ProtoChannel>(_socket);
    connect(_socket, &QTcpSocket::disconnected, this, &ControllerServer::onAgentDisconnected);
    connect(_channel.get(), &ProtoChannel::envelopeReceived, this, &ControllerServer::onEnvelope);
    connect(_channel.get(), &ProtoChannel::transportError, this, &ControllerServer::onTransportError);
    emit agentConnectedChanged(true);
}

void ControllerServer::onAgentDisconnected() {
    _channel.reset();
    _socket->deleteLater();
    _socket = nullptr;
    emit agentConnectedChanged(false);
}

void ControllerServer::onTransportError(const QString& msg) {
    emit lastError(msg);
}

bool ControllerServer::sendTaskRequest(const QString& taskId, int n) {
    if (!_channel) return false;
    task::TaskRequest r;
    r.set_task_id(taskId.toStdString());
    r.set_task_type("sum");
    r.set_param_n(n);
    QByteArray payload;
    payload.resize(r.ByteSizeLong());
    r.SerializeToArray(payload.data(), payload.size());
    return _channel->sendEnvelope(packEnvelope("TaskRequest", payload));
}

bool ControllerServer::sendCancel(const QString& taskId) {
    if (!_channel) return false;
    task::CancelTask c;
    c.set_task_id(taskId.toStdString());
    QByteArray payload;
    payload.resize(c.ByteSizeLong());
    c.SerializeToArray(payload.data(), payload.size());
    return _channel->sendEnvelope(packEnvelope("CancelTask", payload));
}

void ControllerServer::onEnvelope(const QByteArray& envBytes) {
    task::Envelope env;
    if (!env.ParseFromArray(envBytes.constData(), envBytes.size())) {
        emit lastError(QStringLiteral("Ошибка парсинга Envelope"));
        return;
    }
    const QString kind = QString::fromStdString(env.kind());
    const QByteArray payload(env.payload().data(), static_cast<int>(env.payload().size()));

    if (kind == QLatin1String("TaskStatus")) {
        task::TaskStatus s;
        if (s.ParseFromArray(payload.constData(), payload.size())) {
            emit taskStatusReceived(QString::fromStdString(s.task_id()),
                                    QString::fromStdString(s.status()),
                                    s.progress(),
                                    s.result());
        }
    } else if (kind == QLatin1String("TaskResponse")) {
        task::TaskResponse r;
        if (r.ParseFromArray(payload.constData(), payload.size())) {
            emit taskResponseReceived(r.success(),
                                      QString::fromStdString(r.error_message()));
        }
    } else if (kind == QLatin1String("Handshake")) {
        // Можно логировать agent_id / protocol_version при желании
    }
}
