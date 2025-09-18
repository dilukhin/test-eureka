#pragma once
#include <QWidget>

class QLineEdit;
class QPushButton;
class QLabel;
class AgentClient;

/**
 * @brief Простое окно агента: ввод host/port, кнопка "Подключиться".
 */
class AgentWindow final : public QWidget {
    Q_OBJECT
public:
    explicit AgentWindow(QWidget* parent = nullptr);
    ~AgentWindow() override;

private slots:
    void onConnectClicked();
    void onConnectedChanged(bool connected);
    void onErrorChanged(const QString& message);

private:
    QLineEdit* _hostEdit = nullptr;
    QLineEdit* _portEdit = nullptr;
    QPushButton* _connectBtn = nullptr;
    QLabel* _statusLbl = nullptr;

    AgentClient* _client = nullptr;
    bool _connected = false;
};
