#ifndef STATSWORKER_H
#define STATSWORKER_H

#include <QObject>
#include <QTimer>

class StatsWorker : public QObject
{
    Q_OBJECT
public:
    explicit StatsWorker(QObject *parent = nullptr);

public slots:
    void process();
    void exportReport(const QString &filePath);

signals:
    void statsUpdated(int patientCount, int doctorCount, int lowStockCount);
    void reportExported(bool success, QString message);
    void finished();
};

#endif // STATSWORKER_H
//
