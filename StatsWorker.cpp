#include "StatsWorker.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QThread>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QSqlDatabase>
#include <QCoreApplication>

StatsWorker::StatsWorker(QObject *parent) : QObject(parent)
{
}

void StatsWorker::process()
{
    // Create a thread-specific connection
    const QString connectionName = "StatsWorkerConnection";
    
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
        db.setDatabaseName(QCoreApplication::applicationDirPath() + "/medical_enhanced.db");
        
        if (!db.open()) {
            qDebug() << "StatsWorker: Failed to open database:" << db.lastError().text();
            emit finished();
            return;
        }

        QSqlQuery query(db);
        int patientCount = 0;
        if (query.exec("SELECT count(*) FROM patients")) {
            if (query.next()) patientCount = query.value(0).toInt();
        }

        int doctorCount = 0;
        if (query.exec("SELECT count(*) FROM doctors")) {
            if (query.next()) doctorCount = query.value(0).toInt();
        }

        int lowStockCount = 0;
        // Assume stock < 10 is low
        if (query.exec("SELECT count(*) FROM drugs WHERE stock < 10")) {
            if (query.next()) lowStockCount = query.value(0).toInt();
        }

        emit statsUpdated(patientCount, doctorCount, lowStockCount);
        db.close();
    }
    // Remove database connection after use (and after QSqlDatabase object is destroyed)
    QSqlDatabase::removeDatabase(connectionName);
    
    emit finished();
}

void StatsWorker::exportReport(const QString &filePath)
{
    const QString connectionName = "StatsWorkerExportConnection";
    
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
        db.setDatabaseName(QCoreApplication::applicationDirPath() + "/medical_enhanced.db");
        
        if (!db.open()) {
            emit reportExported(false, "无法打开数据库: " + db.lastError().text());
            return;
        }

        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            emit reportExported(false, "无法打开文件进行写入");
            db.close();
            return;
        }

        QTextStream out(&file);
        // Add BOM for Excel compatibility
        out << QString::fromUtf8("\xEF\xBB\xBF"); 
        out << "生成时间: " << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << "\n\n";
        
        QSqlQuery query(db);
        
        // Patients
        out << "=== 病人列表 ===\n";
        out << "ID,姓名,年龄,性别,电话\n";
        if (query.exec("SELECT * FROM patients")) {
            while (query.next()) {
                out << query.value(0).toString() << ","
                    << query.value(1).toString() << ","
                    << query.value(2).toString() << ","
                    << query.value(3).toString() << ","
                    << query.value(4).toString() << "\n";
            }
        }
        out << "\n";

        // Doctors
        out << "=== 医生列表 ===\n";
        out << "ID,姓名,科室\n";
        if (query.exec("SELECT * FROM doctors")) {
            while (query.next()) {
                out << query.value(0).toString() << ","
                    << query.value(1).toString() << ","
                    << query.value(2).toString() << "\n";
            }
        }
        out << "\n";
        
        // Low Stock Drugs
        out << "=== 低库存药品预警 ===\n";
        out << "ID,药品名,库存,单价\n";
        if (query.exec("SELECT * FROM drugs WHERE stock < 10")) {
            while (query.next()) {
                out << query.value(0).toString() << ","
                    << query.value(1).toString() << ","
                    << query.value(2).toString() << ","
                    << query.value(3).toString() << "\n";
            }
        }

        file.close();
        db.close();
    }
    QSqlDatabase::removeDatabase(connectionName);

    emit reportExported(true, "报表已成功导出至: " + filePath);
}
//
