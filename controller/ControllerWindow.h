#pragma once
#include <QWidget>
#include <QMap>

class QTableWidget;
class QLineEdit;
class QPushButton;
class QLabel;
class ControllerServer;
class ControllerDb;

/**
 * @brief Окно контроллера: таблица задач, ввод N, кнопки "Создать", "Отменить".
 */
class ControllerWindow final : public QWidget {
    Q_OBJECT
public:
    explicit ControllerWindow(QWidget* parent = nullptr);
    ~ControllerWindow() override;

private slots:
    void onCreateClicked();
    void onCancelClicked();
    void onAgentConnected(bool connected);
    void onTaskStatus(const QString& taskId, const QString& status, int progress, qint64 result);
    void onTaskResponse(bool success, const QString& error);
    void onListenClicked();

private:
    void ensureRowFor(const QString& taskId);

private:
    QTableWidget* _table = nullptr;
    QLineEdit* _nEdit = nullptr;
    QPushButton* _createBtn = nullptr;
    QPushButton* _cancelBtn = nullptr;
    QPushButton* _listenBtn = nullptr;
    QLineEdit* _portEdit = nullptr;
    QLabel* _statusLbl = nullptr;

    ControllerServer* _server = nullptr;
    ControllerDb* _db = nullptr;

    // taskId -> row
    QMap<QString, int> _rows;
};
