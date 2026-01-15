#include "DatabaseManager.h"

DatabaseManager::DatabaseManager()
{
}

DatabaseManager::~DatabaseManager()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}

DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager instance;
    return instance;
}

#include <QCoreApplication>

bool DatabaseManager::connectToDatabase()
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    
    // Use absolute path in application directory
    QString dbPath = QCoreApplication::applicationDirPath() + "/medical_enhanced.db";
    m_db.setDatabaseName(dbPath);

    qDebug() << "Connecting to database at:" << dbPath;
    qDebug() << "Available drivers:" << QSqlDatabase::drivers();

    if (!m_db.open()) {
        qDebug() << "Error: Connection to database failed" << m_db.lastError();
        return false;
    }
    qDebug() << "Database connected successfully.";
    initTables();
    return true;
}

void DatabaseManager::initTables()
{
    QSqlQuery query;

    // 1. Patients Table
    query.exec("CREATE TABLE IF NOT EXISTS patients ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "name TEXT NOT NULL, "
               "age INTEGER, "
               "gender TEXT, "
               "phone TEXT)");

    // 2. Doctors Table
    query.exec("CREATE TABLE IF NOT EXISTS doctors ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "name TEXT NOT NULL, "
               "department TEXT)");

    // 3. Drugs Table
    query.exec("CREATE TABLE IF NOT EXISTS drugs ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "name TEXT NOT NULL, "
               "stock INTEGER, "
               "price REAL)");

    // 4. Appointments Table
    query.exec("CREATE TABLE IF NOT EXISTS appointments ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "patient_id INTEGER, "
               "doctor_id INTEGER, "
               "appointment_time TEXT, "
               "FOREIGN KEY(patient_id) REFERENCES patients(id), "
               "FOREIGN KEY(doctor_id) REFERENCES doctors(id))");

    // 5. Records Table (Consultation history)
    query.exec("CREATE TABLE IF NOT EXISTS records ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "patient_id INTEGER, "
               "doctor_id INTEGER, "
               "diagnosis TEXT, "
               "prescription TEXT, "
               "record_date TEXT, "
               "FOREIGN KEY(patient_id) REFERENCES patients(id), "
               "FOREIGN KEY(doctor_id) REFERENCES doctors(id))");

    // Insert dummy data if empty
    QSqlQuery checkQuery("SELECT count(*) FROM patients");
    if (checkQuery.next() && checkQuery.value(0).toInt() == 0) {
        query.exec("INSERT INTO patients (name, age, gender, phone) VALUES ('张三', 30, '男', '13800138000')");
        query.exec("INSERT INTO patients (name, age, gender, phone) VALUES ('李四', 25, '女', '13900139000')");
        
        query.exec("INSERT INTO doctors (name, department) VALUES ('王医生', '内科')");
        query.exec("INSERT INTO doctors (name, department) VALUES ('赵医生', '外科')");
        
        query.exec("INSERT INTO drugs (name, stock, price) VALUES ('阿莫西林', 100, 25.5)");
        query.exec("INSERT INTO drugs (name, stock, price) VALUES ('感冒灵', 50, 12.0)");
        query.exec("INSERT INTO drugs (name, stock, price) VALUES ('布洛芬', 5, 30.0)"); // Low stock example
    }
}
//
