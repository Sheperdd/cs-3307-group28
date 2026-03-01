//
// Created by svkol on 2026-02-28.
//

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#pragma once
#include <QMainWindow>
#include <QNetworkAccessManager>

class MainWindow : public QMainWindow{

    Q_OBJECT

public:
     MainWindow( QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onButtonClicked();

private:
    QNetworkAccessManager *m_network;
};



#endif //MAINWINDOW_H
