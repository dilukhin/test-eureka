#include "ControllerWindow.h"
#include "ControllerServer.h"
#include "ControllerDb.h"

#include <QTableWidget>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QUuid>

ControllerWindow::ControllerWindow(QWidget* parent)
    : QWidget(parent)
    , _server(new ControllerServer(this))
    , _db(new ControllerDb(this)) {
    setWindowTitle(QStringLiteral("Controller"));

    _table = new QTableWidget(0, 4, this);
    _table->setHorizontalHeaderLabels({QStringLiteral("ID"), QStringLiteral("Статус"), QStringLiteral("Прогресс"), QStringLiteral("Результат")});
    _table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    _table->setSelectionBehavior(QAbstractItemView::SelectRows);
    _table->setSelectionMode(QAbstractItemView::SingleSelection);

    _nEdit = new QLineEdit(QStringLiteral("1000000"), this);
    _createBtn = new QPushButton(QStringLiteral("Создать задачу"), this);
    _cancelBtn = new QPushButton(QStringLiteral("Отменить задачу"), this);

    _portEdit = new QLineEdit(QStringLiteral("5555"), this);
    _listenBtn = new QPushButton(QStringLiteral("Слушать"), this);
    _statusLbl = new QLabel(QStringLiteral("Нет соединения"), this);

    auto hl1 = new QHBoxLayout;
    hl1->addWidget(new QLabel(QStringLiteral("Порт:"), this));
    hl1->addWidget(_portEdit);
    hl1->addWidget(_listenBtn);
    hl1->addStretch();
    hl1->addWidget(_statusLbl);

    auto hl2 = new QHBoxLayout;
    hl2->addWidget(new QLabel(QStringLiteral("N:"), this));
    hl2->addWidget(_nEdit);
    hl2->addWidget(_createBtn);
    hl2->addWidget(_cancelBtn);
    hl2->addStretch();

    auto vl = new QVBoxLayout;
    vl->addLayout(hl1);
    vl->addWidget(_table, 1);
    vl->addLayout(hl2);
    setLayout(vl);

    connect(_listenBtn, &QPushButton::clicked, this, &ControllerWindow::onListenClicked);
    connect(_createBtn, &QPushButton::clicked, this, &ControllerWindow::onCreateClicked);
    connect(_cancelBtn, &QPushButton::clicked, this, &ControllerWindow::onCancelClicked);

    connect(_server, &ControllerServer::agentConnectedChanged, this, &ControllerWindow::onAgentConnected);
    connect(_server, &ControllerServer::taskStatusReceived, this, &ControllerWindow::onTaskStatus);
    connect(_server, &ControllerServer::taskResponseReceived, this, &ControllerWindow::onTaskResponse);
    connect(_server, &ControllerServer::lastError, _statusLbl, &QLabel::setText);

    // БД
    if (_db->open(QStringLiteral("controller_tasks.sqlite"))) {
        _db->initSchema();
    } else {
        _statusLbl->setText(QStringLiteral("Ошибка открытия БД"));
    }
}

ControllerWindow::~ControllerWindow() = default;

void ControllerWindow::onListenClicked() {
    bool ok = false;
    const quint16 port = _portEdit->text().toUShort(&ok);
    if (!ok || port == 0) {
        _statusLbl->setText(QStringLiteral("Некорректный порт"));
        return;
    }
    if (_server->listen(port)) {
        _statusLbl->setText(QStringLiteral("Слушаю %1").arg(port));
    }
}

void ControllerWindow::onCreateClicked() {
    if (!_server->isConnected()) {
        _statusLbl->setText(QStringLiteral("Агент не подключен"));
        return;
    }
    bool ok = false;
    const int n = _nEdit->text().toInt(&ok);
    if (!ok || n <= 0) {
        _statusLbl->setText(QStringLiteral("Некорректное N"));
        return;
    }
    const QString taskId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    ensureRowFor(taskId);
    _server->sendTaskRequest(taskId, n);
}

void ControllerWindow::onCancelClicked() {
    auto sel = _table->currentRow();
    if (sel < 0) return;
    const QString taskId = _table->item(sel, 0)->text();
    _server->sendCancel(taskId);
}

void ControllerWindow::onAgentConnected(bool connected) {
    _statusLbl->setText(connected ? QStringLiteral("Агент подключен") : QStringLiteral("Нет соединения"));
}

void ControllerWindow::ensureRowFor(const QString& taskId) {
    if (_rows.contains(taskId)) return;
    const int row = _table->rowCount();
    _table->insertRow(row);
    _table->setItem(row, 0, new QTableWidgetItem(taskId));
    _table->setItem(row, 1, new QTableWidgetItem(QStringLiteral("new")));
    _table->setItem(row, 2, new QTableWidgetItem(QStringLiteral("0")));
    _table->setItem(row, 3, new QTableWidgetItem(QStringLiteral("0")));
    _rows.insert(taskId, row);
}

void ControllerWindow::onTaskStatus(const QString& taskId, const QString& status, int progress, qint64 result) {
    ensureRowFor(taskId);
    const int row = _rows.value(taskId);
    _table->item(row, 1)->setText(status);
    _table->item(row, 2)->setText(QString::number(progress));
    _table->item(row, 3)->setText(QString::number(result));
    if (_db) {
        _db->upsertTask(taskId, status, progress, result);
    }
}

void ControllerWindow::onTaskResponse(bool success, const QString& error) {
    if (!success) {
        _statusLbl->setText(QStringLiteral("Ошибка создания задачи: %1").arg(error));
    }
}
