#include "ProtoChannel.h"
#include <QtEndian>

ProtoChannel::ProtoChannel(QTcpSocket* socket, QObject* parent)
    : QObject(parent)
    , _socket(socket) {
    Q_ASSERT(_socket);
    connect(_socket, &QTcpSocket::readyRead, this, &ProtoChannel::onReadyRead);
}

bool ProtoChannel::sendEnvelope(const QByteArray& envelope) {
    if (!_socket || _socket->state() != QAbstractSocket::ConnectedState) {
        emit transportError(QStringLiteral("Соединение отсутствует"));
        return false;
    }
    // Префикс длины 4 байта Big-Endian
    quint32 len = envelope.size();
    char header[4];
    qToBigEndian(len, reinterpret_cast<uchar*>(header));
    QByteArray frame;
    frame.append(header, 4);
    frame.append(envelope);
    auto written = _socket->write(frame);
    if (written != frame.size()) {
        emit transportError(QStringLiteral("Отправлено меньше байт, чем ожидалось"));
        return false;
    }
    return _socket->flush();
}

void ProtoChannel::onReadyRead() {
    _rxBuffer.append(_socket->readAll());
    processIncoming();
}

void ProtoChannel::processIncoming() {
    // Читает кадры подряд: [4][payload]
    while (_rxBuffer.size() >= 4) {
        const quint32 len = qFromBigEndian<quint32>(reinterpret_cast<const uchar*>(_rxBuffer.constData()));
        if (len > 32 * 1024 * 1024) { // защита от слишком больших пакетов
            emit transportError(QStringLiteral("Слишком большой пакет"));
            _rxBuffer.clear();
            return;
        }
        if (_rxBuffer.size() < 4 + static_cast<int>(len)) {
            // ждём добора данных
            return;
        }
        const QByteArray envelope = _rxBuffer.mid(4, len);
        _rxBuffer.remove(0, 4 + len);
        emit envelopeReceived(envelope);
    }
}
