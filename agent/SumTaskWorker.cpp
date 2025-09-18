#include "SumTaskWorker.h"
#include <QtMath>

SumTaskWorker::SumTaskWorker(QString taskId, qint64 n, QObject* parent)
    : QThread(parent)
    , _taskId(std::move(taskId))
    , _n(n) {}

SumTaskWorker::~SumTaskWorker() {
    // Подождать завершения потока при уничтожении
    if (isRunning()) {
        _canceled = true;
        wait(200);
    }
}

void SumTaskWorker::cancel() {
    _canceled = true;
}

void SumTaskWorker::run() {
    if (_n <= 0) {
        emit statusChanged(_taskId, QStringLiteral("error"), 0, 0);
        return;
    }
    emit statusChanged(_taskId, QStringLiteral("running"), 0, 0);

    // Суммирование с прогрессом
    qint64 sum = 0;
    const qint64 step = qMax<qint64>(1, _n / 100);
    for (qint64 i = 1; i <= _n; ++i) {
        if (_canceled.load(std::memory_order_relaxed)) {
            emit statusChanged(_taskId, QStringLiteral("canceled"), 0, 0);
            return;
        }
        sum += i;
        if (i % step == 0 || i == _n) {
            const int progress = static_cast<int>((i * 100) / _n);
            emit statusChanged(_taskId, QStringLiteral("running"), progress, 0);
            // Дать UI/сокету шанс прокачать события
            QThread::msleep(1);
        }
    }
    emit statusChanged(_taskId, QStringLiteral("completed"), 100, sum);
}
