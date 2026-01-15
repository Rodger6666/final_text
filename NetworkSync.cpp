#include "NetworkSync.h"
#include <QDebug>

NetworkSync::NetworkSync(QObject *parent) : QObject(parent)
{
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(performSync()));
}

void NetworkSync::startSync()
{
    // Sync every 10 seconds
    m_timer->start(10000); 
    performSync(); // Trigger immediately
}

void NetworkSync::performSync()
{
    emit syncStatusChanged("正在同步药品信息...");
    
    // Simulate network delay
    QTimer::singleShot(2000, [this]() {
        // In a real app, use QNetworkAccessManager here.
        // QNetworkAccessManager *manager = new QNetworkAccessManager(this);
        // manager->get(QNetworkRequest(QUrl("http://example.com/api/drugs")));
        
        // Mock success
        emit syncStatusChanged("同步完成");
        emit dataSynced("药品库已更新至最新版本 (Mock Data)");
    });
}
//
