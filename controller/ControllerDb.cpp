#include "ControllerDb.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>

ControllerDb::ControllerDb(QObject* parent) : QObject(parent) {}

ControllerDb::~ControllerDb() {
    if (_db.isOpen()) {
        _db.close();
    }
}

bool ControllerDb::open(const QString& filePath) {
    _db = QSqlDatabase::addDatabase("QSQLITE");
    _db.setDatabaseName(filePath);
    return _db.open();
}

void ControllerDb::initSchema() {
    QSqlQuery q;
    q.exec(QStringLiteral(
        "CREATE TABLE IF NOT EXISTS tasks ("
        "id TEXT PRIMARY KEY,"
        "status TEXT,"
        "progress INTEGER,"
        "result INTEGER)"));
}

bool ControllerDb::upsertTask(const QString& id, const QString& status, int progress, qint64 result) {
    QSqlQuery q;
    q.prepare(QStringLiteral(
        "INSERT INTO tasks(id, status, progress, result) VALUES(?,?,?,?) "
        "ON CONFLICT(id) DO UPDATE SET status=excluded.status, progress=excluded.progress, result=excluded.result"));
    q.addBindValue(id);
    q.addBindValue(status);
    q.addBindValue(progress);
    q.addBindValue(static_cast<qlonglong>(result));
    return q.exec();
}
