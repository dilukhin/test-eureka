#include "AgentClient.h"
#include "SumTaskWorker.h"

#include <QHostAddress>
#include <QThread>

// protobuf
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

AgentClient::AgentClient(QObject* parent)
    : QObject(parent)
    , _socket(new QTcpSocket(this))
    , _agentId(QUuid::createUuid().toString(QUuid::WithoutBraces)) {
    connect(_socket, &QTcpSocket::connected, this, &AgentClient::onConnected);
    connect(_socket, &QTcpSocket::disconnected, this, &AgentClient::onDisconnected);
}

AgentClient::~AgentClient() = default;

void AgentClient::connectToController(const QString& host, quint16 port) {
    if (_socket->state() != QAbstractSocket::UnconnectedState) {
        _socket->abort();
    }
    _socket->connectToHost(host, port);
}

void AgentClient::disconnectFromController() {
    _socket->disconnectFromHost();
}

void AgentClient::onConnected() {
    _channel = std::make_unique<ProtoChannel>(_socket);
    connect(_channel.get(), &ProtoChannel::envelopeReceived, this, &AgentClient::onEnvelope);
    connect(_channel.get(), &ProtoChannel::transportError, this, &AgentClient::onTransportError);
    sendHandshake();
    emit connectedChanged(true);
}

void AgentClient::onDisconnected() {
    _channel.reset();
    emit connectedChanged(false);
}

void AgentClient::onTransportError(const QString& msg) {
    _lastError = msg;
    emit lastErrorChanged(_lastError);
}

void AgentClient::sendHandshake() {
    task::Handshake hs;
    hs.set_agent_id(_agentId.toStdString());
    hs.set_protocol_version("1.0");
    QByteArray payload;
    payload.resize(hs.ByteSizeLong());
    hs.SerializeToArray(payload.data(), payload.size());
    _channel->sendEnvelope(packEnvelope("Handshake", payload));
}

void AgentClient::onEnvelope(const QByteArray& bytes) {
    task::Envelope env;
    if (!env.ParseFromArray(bytes.constData(), bytes.size())) {
        onTransportError(QStringLiteral("Не удалось распарсить Envelope"));
        return;
    }
    const QString kind = QString::fromStdString(env.kind());
    const QByteArray payload(env.payload().data(), static_cast<int>(env.payload().size()));
    if (kind == QLatin1String("TaskRequest")) {
        handleTaskRequest(payload);
    } else if (kind == QLatin1String("CancelTask")) {
        handleCancel(payload);
    }
}

void AgentClient::handleTaskRequest(const QByteArray& payload) {
    task::TaskRequest req;
    if (!req.ParseFromArray(payload.constData(), payload.size())) {
        sendTaskResponse(false, QStringLiteral("Некорректный TaskRequest"));
        return;
    }
    const QString taskId = QString::fromStdString(req.task_id());
    const QString type = QString::fromStdString(req.task_type());

    if (type != QLatin1String("sum")) {
        sendTaskResponse(false, QStringLiteral("Неизвестный тип задачи: %1").arg(type));
        return;
    }
    if (_worker) {
        sendTaskResponse(false, QStringLiteral("Задача уже выполняется"));
        return;
    }

    // Создаёт воркер и поток
    _worker = std::make_unique<SumTaskWorker>(taskId, req.param_n());
    connect(_worker.get(), &SumTaskWorker::statusChanged,
            this, &AgentClient::onWorkerStatus, Qt::QueuedConnection);

    // Старт в отдельном потоке
    _worker->start();

    sendTaskResponse(true, QString());
    // Отправить первичный статус
    sendTaskStatus(taskId, QStringLiteral("running"), 0, 0);
}

void AgentClient::handleCancel(const QByteArray& payload) {
    task::CancelTask c;
    if (!c.ParseFromArray(payload.constData(), payload.size())) {
        return;
    }
    const QString taskId = QString::fromStdString(c.task_id());
    if (_worker && _worker->taskId() == taskId) {
        _worker->cancel();
    }
}

void AgentClient::onWorkerStatus(const QString& taskId, const QString& status, int progress, qint64 result) {
    sendTaskStatus(taskId, status, progress, result);
    if (status == QLatin1String("completed") || status == QLatin1String("error") || status == QLatin1String("canceled")) {
        _worker.reset();
    }
}

void AgentClient::sendTaskStatus(const QString& taskId, const QString& status, int progress, qint64 result) {
    task::TaskStatus st;
    st.set_task_id(taskId.toStdString());
    st.set_status(status.toStdString());
    st.set_progress(progress);
    st.set_result(result);
    QByteArray payload;
    payload.resize(st.ByteSizeLong());
    st.SerializeToArray(payload.data(), payload.size());
    if (_channel) {
        _channel->sendEnvelope(packEnvelope("TaskStatus", payload));
    }
}

void AgentClient::sendTaskResponse(bool ok, const QString& error) {
    task::TaskResponse resp;
    resp.set_success(ok);
    if (!ok) {
        resp.set_error_message(error.toStdString());
    }
    QByteArray payload;
    payload.resize(resp.ByteSizeLong());
    resp.SerializeToArray(payload.data(), payload.size());
    if (_channel) {
        _channel->sendEnvelope(packEnvelope("TaskResponse", payload));
    }
}
