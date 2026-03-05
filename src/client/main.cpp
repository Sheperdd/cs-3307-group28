//
// Created by svkol on 2026-02-28.
//
#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainWindow window;
    window.show();

    return app.exec(); // Starts the event loop
}