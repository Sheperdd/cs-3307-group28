//
// Created by svkol on 2026-02-28.
//
#include <QDebug>
#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    qDebug() << "Starting TorqueDesk client...";
    QApplication app(argc, argv);

    MainWindow window;
    window.show();

    int result = app.exec();
    qDebug() << "Event loop exited with code:" << result;
    return result;
}
