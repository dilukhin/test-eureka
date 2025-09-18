#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QByteArray>
#include <QDataStream>
#include <memory>

namespace task { class Envelope; }

/**
 * @brief Двунаправленный канал обмена protobuf поверх QTcpSocket с length-prefixed фреймингом.
 *
 * Формат кадра:
 *   [u32_be length][bytes payload = Envelope(serialized)]
 * Где length = размер сериализованного Envelope.
 */
class ProtoChannel final : public QObject {
    Q_OBJECT
public:
    explicit ProtoChannel(QTcpSocket* socket, QObject* parent = nullptr);

    /**
     * @brief Отправить Envelope.
     * @param envelope Буфер сериализованного Envelope.
     * @return true при успехе.
     */
    bool sendEnvelope(const QByteArray& envelope);

signals:
    /**
     * @brief Получен полностью разобранный Envelope (сырые байты protobuf).
     */
    void envelopeReceived(const QByteArray& envelopeBytes);

    /**
     * @brief Ошибка уровня транспорта/парсинга.
     */
    void transportError(const QString& message);

private slots:
    void onReadyRead();

private:
    void processIncoming();

private:
    QTcpSocket* _socket;
    QByteArray _rxBuffer;
};
