#include "AgentWindow.h"
#include "AgentClient.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

AgentWindow::AgentWindow(QWidget* parent)
    : QWidget(parent)
    , _client(new AgentClient(this)) {
    setWindowTitle(QStringLiteral("Agent"));

    _hostEdit = new QLineEdit(QStringLiteral("127.0.0.1"), this);
    _portEdit = new QLineEdit(QStringLiteral("5555"), this);
    _connectBtn = new QPushButton(QStringLiteral("Подключиться"), this);
    _statusLbl = new QLabel(QStringLiteral("Отключено"), this);

    auto hl = new QHBoxLayout;
    hl->addWidget(new QLabel(QStringLiteral("Host:"), this));
    hl->addWidget(_hostEdit, 1);
    hl->addWidget(new QLabel(QStringLiteral("Port:"), this));
    hl->addWidget(_portEdit);
    hl->addWidget(_connectBtn);

    auto vl = new QVBoxLayout;
    vl->addLayout(hl);
    vl->addWidget(_statusLbl, 0, Qt::AlignLeft);
    setLayout(vl);

    connect(_connectBtn, &QPushButton::clicked, this, &AgentWindow::onConnectClicked);
    connect(_client, &AgentClient::connectedChanged, this, &AgentWindow::onConnectedChanged);
    connect(_client, &AgentClient::lastErrorChanged, this, &AgentWindow::onErrorChanged);
}

AgentWindow::~AgentWindow() = default;

void AgentWindow::onConnectClicked() {
    if (_connected) {
        _client->disconnectFromController();
    } else {
        bool ok = false;
        const quint16 port = _portEdit->text().toUShort(&ok);
        if (!ok || port == 0) {
            _statusLbl->setText(QStringLiteral("Некорректный порт"));
            return;
        }
        _client->connectToController(_hostEdit->text(), port);
    }
}

void AgentWindow::onConnectedChanged(bool connected) {
    _connected = connected;
    _connectBtn->setText(connected ? QStringLiteral("Отключиться") : QStringLiteral("Подключиться"));
    _statusLbl->setText(connected ? QStringLiteral("Подключено") : QStringLiteral("Отключено"));
}

void AgentWindow::onErrorChanged(const QString& message) {
    _statusLbl->setText(QStringLiteral("Ошибка: %1").arg(message));
}
