#ifndef NETWORKSYNC_H
#define NETWORKSYNC_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>

class NetworkSync : public QObject
{
    Q_OBJECT
public:
    explicit NetworkSync(QObject *parent = nullptr);
    void startSync();

signals:
    void syncStatusChanged(QString status);
    void dataSynced(QString info);

private slots:
    void performSync();

private:
    QTimer *m_timer;
};

#endif // NETWORKSYNC_H
//
