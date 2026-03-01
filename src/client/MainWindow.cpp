//
// Created by svkol on 2026-02-28.
//

#include "MainWindow.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent){


    m_network = new QNetworkAccessManager(this);

    QWidget *central = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(central);

    QPushButton *btn1 = new QPushButton("Click me", this);
    layout -> addWidget(btn1);

    setCentralWidget(central);
    setWindowTitle("TorqueClient");
    resize(800, 600);

    connect(btn1, &QPushButton::clicked, this , &MainWindow::onButtonClicked);

}

MainWindow::~MainWindow(){
}

void MainWindow::onButtonClicked(){
}
