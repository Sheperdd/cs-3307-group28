  // Created by svkol on 2026-02-28.
//

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#pragma once
#include <QMainWindow>

class QStackedWidget;
class LandingPage;
class SignUpPage;
class LoginPage;

class MainWindow : public QMainWindow{
    Q_OBJECT
public:
     MainWindow( QWidget *parent = nullptr);
    ~MainWindow();

private :
    QStackedWidget *stack;

    LandingPage *landingPage;
    SignUpPage *signUpPage;
    LoginPage *loginPage;


    enum PageIndex{
        LandingIndex =0, SignUpIndex =1, LoginIndex
    };


    void showLandingPage();
    void showSignUpPage();
    void showLoginPage();
private slots:
    void handleSignUpSubmitted(const QString &fname,
                            const QString &lname,
                            const QString &email,
                            const QString &password,
                            const QString &confirmPassword);

    void handleLoginSubmitted(const QString &email, const QString &password);
};



#endif //MAINWINDOW_H
