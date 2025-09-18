#pragma once

#include <QObject>
#include <QThread>
#include <atomic>

/**
 * @brief Воркер суммирования 1..N, работает в своем QThread, поддерживает cancel.
 */
class SumTaskWorker final : public QThread {
    Q_OBJECT
public:
    SumTaskWorker(QString taskId, qint64 n, QObject* parent = nullptr);
    ~SumTaskWorker() override;

    /**
     * @brief Идентификатор задачи.
     */
    QString taskId() const { return _taskId; }

    /**
     * @brief Запросить отмену.
     */
    void cancel();

signals:
    /**
     * @brief Репорт статуса.
     */
    void statusChanged(const QString& taskId, const QString& status, int progress, qint64 result);

protected:
    void run() override;

private:
    QString _taskId;
    qint64 _n;
    std::atomic_bool _canceled{false};
};
