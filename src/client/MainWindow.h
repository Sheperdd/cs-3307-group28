/**
 * @file MainWindow.h
 * @brief Top-level window — owns a QStackedWidget that switches between
 *        LandingPage, SignUpPage, LoginPage, and PasswordResetPage.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#pragma once
#include <QMainWindow>

class QStackedWidget;
class LandingPage;
class SignUpPage;
class LoginPage;
class PasswordResetPage;

class MainWindow : public QMainWindow{
    Q_OBJECT
public:
     MainWindow( QWidget *parent = nullptr);
    ~MainWindow();

private :
    QStackedWidget *stack = nullptr;
    LandingPage *landingPage = nullptr;
    SignUpPage *signUpPage = nullptr;
    LoginPage *loginPage = nullptr;
    PasswordResetPage *passwordResetPage =nullptr;


    enum PageIndex{
        LandingIndex =0,
        SignUpIndex =1,
        LoginIndex,
        PasswordReset
    };


    void showLandingPage();
    void showSignUpPage();
    void showLoginPage();
    void showPasswordResetPage();


private slots:
    void handleSignUpSubmitted(const QString &fname,
                            const QString &lname,
                            const QString &email,
                            const QString &password,
                            const QString &confirmPassword);

    void handleLoginSubmitted(const QString &email, const QString &password);

    void resetPasswordSubmitted(const QString oldPassword, const QString newPassword);
};


#endif //MAINWINDOW_H
