#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>
#include <QString>

class DatabaseManager
{
public:
    static DatabaseManager& instance();
    bool connectToDatabase();
    void initTables();

private:
    DatabaseManager();
    ~DatabaseManager();
    QSqlDatabase m_db;
};

#endif // DATABASEMANAGER_H
//
