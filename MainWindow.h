#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlTableModel>
#include <QSqlRelationalTableModel>
#include <QTableView>
#include <QThread>
#include <QLabel>
#include "StatsWorker.h"
#include "NetworkSync.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void updateStats(int patients, int doctors, int lowStock);
    void updateSyncStatus(QString status);
    void onAddPatient();
    void onDeletePatient();
    void onSubmitPatient(); // Save changes

private:
    void setupModels();

    // UI
    Ui::MainWindow *ui;

    // Models
    QSqlTableModel *m_patientModel;
    QSqlTableModel *m_doctorModel;
    QSqlTableModel *m_drugModel;
    QSqlRelationalTableModel *m_appointmentModel;
    QSqlRelationalTableModel *m_recordModel;

    // Background Worker
    QThread *m_workerThread;
    StatsWorker *m_statsWorker;
    QTimer *m_statsTimer;

    // Network
    NetworkSync *m_networkSync;
};

#endif // MAINWINDOW_H
//
