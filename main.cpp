#include "MainWindow.h"
#include "DatabaseManager.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    // High DPI support
    // QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    
    QApplication a(argc, argv);

    // Initialize Database
    if (!DatabaseManager::instance().connectToDatabase()) {
        return -1;
    }

    MainWindow w;
    w.show();

    return a.exec();
}
//
