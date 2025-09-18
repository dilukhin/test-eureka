#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QUuid>
#include "common/ProtoChannel.h"

// forward protobuf
namespace task { class Envelope; class Handshake; class TaskRequest; class CancelTask; class TaskStatus; }

class SumTaskWorker;

/**
 * @brief Клиент агента: подключается к контроллеру, обрабатывает запросы задач.
 */
class AgentClient final : public QObject {
    Q_OBJECT
public:
    explicit AgentClient(QObject* parent = nullptr);
    ~AgentClient() override;

    void connectToController(const QString& host, quint16 port);
    void disconnectFromController();

signals:
    void connectedChanged(bool connected);
    void lastErrorChanged(const QString& message);

private slots:
    void onConnected();
    void onDisconnected();
    void onEnvelope(const QByteArray& bytes);
    void onTransportError(const QString& msg);

    void onWorkerStatus(const QString& taskId, const QString& status, int progress, qint64 result);

private:
    void sendHandshake();
    void handleTaskRequest(const QByteArray& payload);
    void handleCancel(const QByteArray& payload);
    void sendTaskStatus(const QString& taskId, const QString& status, int progress, qint64 result);
    void sendTaskResponse(bool ok, const QString& error);

private:
    QTcpSocket* _socket = nullptr;
    std::unique_ptr<ProtoChannel> _channel;
    QString _agentId;
    QString _lastError;

    // одна задача одновременно для простоты
    std::unique_ptr<SumTaskWorker> _worker;
};
