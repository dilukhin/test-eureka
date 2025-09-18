#pragma once
#include <QObject>
#include <QSqlDatabase>
#include <QString>

/**
 * @brief Обёртка над SQLite: таблица tasks(id TEXT PK, status TEXT, progress INT, result INT).
 */
class ControllerDb final : public QObject {
    Q_OBJECT
public:
    explicit ControllerDb(QObject* parent = nullptr);
    ~ControllerDb() override;

    bool open(const QString& filePath);
    bool upsertTask(const QString& id, const QString& status, int progress, qint64 result);
    void initSchema();

private:
    QSqlDatabase _db;
};
