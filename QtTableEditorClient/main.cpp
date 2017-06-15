#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    DatabaseManager *manager = new DatabaseManager();

    MainWindow window(manager);
    window.show();

    return a.exec();
}
