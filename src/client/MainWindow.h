  // Created by svkol on 2026-02-28.
//

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#pragma once
#include <QMainWindow>

class QStackedWidget;
class LandingPage;
class SignUpPage;

class MainWindow : public QMainWindow{
    Q_OBJECT
public:
     MainWindow( QWidget *parent = nullptr);
    ~MainWindow();

private :
    QStackedWidget *stack;

    LandingPage *landingPage;
    SignUpPage *signUpPage;

    enum PageIndex{
        LandingIndex =0, SignUpIndex =1
    };


    void showLandingPage();
    void showSignUpPage();
private slots:
    void handleSignUpSubmitted(const QString &fname,
                            const QString &lname,
                            const QString &email,
                            const QString &password,
                            const QString &confirmPassword);
};



#endif //MAINWINDOW_H
