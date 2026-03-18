/**
 * @file main.cpp
 * @brief Entry point for the TorqueDesk Qt client.
 */
#include <QDebug>
#include <QApplication>

#include "../MainWindows/LandingPage.h"
#include "../MainWindows/HomePage.h"
#include "../MainWindows/MainWindow.h"

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
