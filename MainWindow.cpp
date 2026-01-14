#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QHeaderView>
#include <QSqlRelationalDelegate>
#include <QMessageBox>
#include <QDebug>
#include <QGroupBox>
#include <QFormLayout>
#include <QSqlError>

#include <QLineEdit>
#include <QInputDialog>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this); // Setup UI from .ui file
    setupModels();

    // Bind Views to Models
    // Patients
    ui->patientView->setModel(m_patientModel);
    ui->patientView->horizontalHeader()->setStretchLastSection(true);
    ui->patientView->hideColumn(0);
    
    // Doctors
    ui->doctorView->setModel(m_doctorModel);
    ui->doctorView->horizontalHeader()->setStretchLastSection(true);
    ui->doctorView->hideColumn(0);

    // Drugs
    ui->drugView->setModel(m_drugModel);
    ui->drugView->horizontalHeader()->setStretchLastSection(true);
    ui->drugView->hideColumn(0);
    // Allow editing
    ui->drugView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);

    // Appointments
    ui->appointmentView->setModel(m_appointmentModel);
    ui->appointmentView->setItemDelegate(new QSqlRelationalDelegate(ui->appointmentView));
    ui->appointmentView->horizontalHeader()->setStretchLastSection(true);
    ui->appointmentView->hideColumn(0);

    // Records
    ui->recordView->setModel(m_recordModel);
    ui->recordView->setItemDelegate(new QSqlRelationalDelegate(ui->recordView));
    ui->recordView->horizontalHeader()->setStretchLastSection(true);
    ui->recordView->hideColumn(0);

    // Connect UI Signals
    // Patients
    connect(ui->btnAddPatient, &QPushButton::clicked, this, &MainWindow::onAddPatient);
    connect(ui->btnDelPatient, &QPushButton::clicked, this, &MainWindow::onDeletePatient);
    connect(ui->btnSubmitPatient, &QPushButton::clicked, this, &MainWindow::onSubmitPatient);
    connect(ui->btnRevertPatient, &QPushButton::clicked, m_patientModel, &QSqlTableModel::revertAll);
    connect(ui->searchPatientEdit, &QLineEdit::textChanged, [this](const QString &text){
        m_patientModel->setFilter(QString("name LIKE '%%1%'").arg(text));
    });

    // Doctors
    connect(ui->btnAddDoctor, &QPushButton::clicked, [this](){
        m_doctorModel->insertRow(m_doctorModel->rowCount());
    });

    // Drugs
    connect(ui->searchDrugEdit, &QLineEdit::textChanged, [this](const QString &text){
        m_drugModel->setFilter(QString("name LIKE '%%1%'").arg(text));
    });

    // Appointments
    connect(ui->btnAddAppointment, &QPushButton::clicked, [this](){
        m_appointmentModel->insertRow(m_appointmentModel->rowCount());
    });

    // Records
    connect(ui->btnAddRecord, &QPushButton::clicked, [this](){
        m_recordModel->insertRow(m_recordModel->rowCount());
        m_recordModel->setData(m_recordModel->index(m_recordModel->rowCount()-1, 5), QDate::currentDate().toString("yyyy-MM-dd"));
    });

    // Dashboard Actions
    connect(ui->btnExport, &QPushButton::clicked, [this](){
        QString fileName = QFileDialog::getSaveFileName(this, "保存报表", "", "CSV Files (*.csv)");
        if (!fileName.isEmpty()) {
            QMetaObject::invokeMethod(m_statsWorker, "exportReport", Q_ARG(QString, fileName));
        }
    });

    connect(ui->btnBackup, &QPushButton::clicked, [this](){
         m_networkSync->startSync(); // Trigger manual sync
         QMessageBox::information(this, "备份", "正在后台执行远程备份...");
    });

    // Setup Stats Worker in a separate thread
    m_workerThread = new QThread(this);
    m_statsWorker = new StatsWorker(); 
    m_statsWorker->moveToThread(m_workerThread);

    connect(m_workerThread, &QThread::finished, m_statsWorker, &QObject::deleteLater);
    connect(m_statsWorker, &StatsWorker::statsUpdated, this, &MainWindow::updateStats);
    connect(m_statsWorker, &StatsWorker::reportExported, this, [this](bool success, QString msg){
        if(success) QMessageBox::information(this, "导出成功", msg);
        else QMessageBox::warning(this, "导出失败", msg);
    });
    
    m_workerThread->start();

    // Trigger stats periodically
    m_statsTimer = new QTimer(this);
    connect(m_statsTimer, &QTimer::timeout, [this]() {
        // Invoke process on the worker thread safely
        QMetaObject::invokeMethod(m_statsWorker, "process");
    });
    m_statsTimer->start(5000); // Update every 5 seconds

    // Setup Network Sync
    m_networkSync = new NetworkSync(this);
    connect(m_networkSync, &NetworkSync::syncStatusChanged, this, &MainWindow::updateSyncStatus);
    m_networkSync->startSync();

    setWindowTitle("社区医疗信息管理系统");
    resize(1000, 700);
}

MainWindow::~MainWindow()
{
    m_workerThread->quit();
    m_workerThread->wait();
    delete ui;
}

void MainWindow::setupModels()
{
    // Patients
    m_patientModel = new QSqlTableModel(this);
    m_patientModel->setTable("patients");
    m_patientModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    m_patientModel->select();
    m_patientModel->setHeaderData(1, Qt::Horizontal, "姓名");
    m_patientModel->setHeaderData(2, Qt::Horizontal, "年龄");
    m_patientModel->setHeaderData(3, Qt::Horizontal, "性别");
    m_patientModel->setHeaderData(4, Qt::Horizontal, "电话");

    // Doctors
    m_doctorModel = new QSqlTableModel(this);
    m_doctorModel->setTable("doctors");
    m_doctorModel->setEditStrategy(QSqlTableModel::OnFieldChange); // Auto save for simplicity here
    m_doctorModel->select();
    m_doctorModel->setHeaderData(1, Qt::Horizontal, "姓名");
    m_doctorModel->setHeaderData(2, Qt::Horizontal, "科室");

    // Drugs
    m_drugModel = new QSqlTableModel(this);
    m_drugModel->setTable("drugs");
    m_drugModel->setEditStrategy(QSqlTableModel::OnFieldChange);
    m_drugModel->select();
    m_drugModel->setHeaderData(1, Qt::Horizontal, "药品名");
    m_drugModel->setHeaderData(2, Qt::Horizontal, "库存");
    m_drugModel->setHeaderData(3, Qt::Horizontal, "单价");

    // Appointments
    m_appointmentModel = new QSqlRelationalTableModel(this);
    m_appointmentModel->setTable("appointments");
    m_appointmentModel->setEditStrategy(QSqlTableModel::OnFieldChange);
    // Join with patients and doctors
    m_appointmentModel->setRelation(1, QSqlRelation("patients", "id", "name"));
    m_appointmentModel->setRelation(2, QSqlRelation("doctors", "id", "name"));
    m_appointmentModel->select();
    m_appointmentModel->setHeaderData(1, Qt::Horizontal, "病人");
    m_appointmentModel->setHeaderData(2, Qt::Horizontal, "医生");
    m_appointmentModel->setHeaderData(3, Qt::Horizontal, "预约时间");

    // Records
    m_recordModel = new QSqlRelationalTableModel(this);
    m_recordModel->setTable("records");
    m_recordModel->setEditStrategy(QSqlTableModel::OnFieldChange);
    m_recordModel->setRelation(1, QSqlRelation("patients", "id", "name"));
    m_recordModel->setRelation(2, QSqlRelation("doctors", "id", "name"));
    m_recordModel->select();
    m_recordModel->setHeaderData(1, Qt::Horizontal, "病人");
    m_recordModel->setHeaderData(2, Qt::Horizontal, "医生");
    m_recordModel->setHeaderData(3, Qt::Horizontal, "诊断结果");
    m_recordModel->setHeaderData(4, Qt::Horizontal, "处方");
    m_recordModel->setHeaderData(5, Qt::Horizontal, "就诊日期");
}

// Removed old createXXXTab methods

void MainWindow::onAddPatient()
{
    m_patientModel->insertRow(m_patientModel->rowCount());
}

void MainWindow::onDeletePatient()
{
    QModelIndexList selection = ui->patientView->selectionModel()->selectedRows();
    for(int i=0; i< selection.count(); i++)
    {
        m_patientModel->removeRow(selection.at(i).row());
    }
}

void MainWindow::onSubmitPatient()
{
    if(m_patientModel->submitAll()) {
        QMessageBox::information(this, "成功", "病人信息已保存");
    } else {
        QMessageBox::warning(this, "失败", "保存失败: " + m_patientModel->lastError().text());
    }
}

void MainWindow::updateStats(int patients, int doctors, int lowStock)
{
    ui->lblPatientCount->setText(QString::number(patients));
    ui->lblDoctorCount->setText(QString::number(doctors));
    if (lowStock > 0) {
        ui->lblLowStock->setText(QString("<font color='red'>%1 (需要补货)</font>").arg(lowStock));
    } else {
        ui->lblLowStock->setText(QString::number(lowStock));
    }
}

void MainWindow::updateSyncStatus(QString status)
{
    ui->lblSyncStatus->setText(status + " [" + QTime::currentTime().toString() + "]");
}
