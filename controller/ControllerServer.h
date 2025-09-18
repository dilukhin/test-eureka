#pragma once
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <memory>
#include "common/ProtoChannel.h"

// forward protobuf
namespace task { class Envelope; class TaskRequest; class TaskStatus; class TaskResponse; class CancelTask; }

/**
 * @brief Сервер контроллера: принимает одного агента и маршрутизирует сообщения.
 */
class ControllerServer final : public QObject {
    Q_OBJECT
public:
    explicit ControllerServer(QObject* parent = nullptr);
    ~ControllerServer() override;

    bool listen(quint16 port);
    void close();

    bool isConnected() const { return _channel != nullptr; }

    bool sendTaskRequest(const QString& taskId, int n);
    bool sendCancel(const QString& taskId);

signals:
    void agentConnectedChanged(bool connected);
    void taskStatusReceived(const QString& taskId, const QString& status, int progress, qint64 result);
    void taskResponseReceived(bool success, const QString& error);
    void lastError(const QString& message);

private slots:
    void onNewConnection();
    void onAgentDisconnected();
    void onEnvelope(const QByteArray& envBytes);
    void onTransportError(const QString& msg);

private:
    QTcpServer _server;
    QTcpSocket* _socket = nullptr;
    std::unique_ptr<ProtoChannel> _channel;
};
