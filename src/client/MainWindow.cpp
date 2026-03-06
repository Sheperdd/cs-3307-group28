#include "MainWindow.h"
#include <QStackedWidget>
#include <QDebug>

#include "LandingPage.h"
#include "LoginPage.h"
#include "PasswordResetPage.h"
#include "SignUpPage.h"


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent){

    //creating a stacked widget and setting it in the center
    stack = new QStackedWidget(this);
    setCentralWidget(stack);

    //creating pages
    landingPage = new LandingPage(this);
    signUpPage = new SignUpPage(this);
    loginPage = new LoginPage(this);
    passwordResetPage = new PasswordResetPage(this);
    
    //creating stack in known order
    stack->addWidget(landingPage);
    stack->addWidget(signUpPage);
    stack->addWidget(loginPage);
    stack->addWidget(passwordResetPage);

    //starting on the landing page
    stack->setCurrentIndex(LandingIndex);

    //Creating navigations
    // From landing page to sign-up page
    connect(landingPage, &LandingPage::signUpRequested,
            this, &MainWindow::showSignUpPage);

    //Will change later, just to make sure that it goes somewhere
    connect (landingPage, &LandingPage::loginRequested, this, &MainWindow::showLoginPage);

    // From sign-up page back to landing page
    connect(signUpPage, &SignUpPage::backRequested,
            this, &MainWindow::showLandingPage);

    // Submission of sign-up
    connect(signUpPage, &SignUpPage::signUpSubmitted,
            this, &MainWindow::handleSignUpSubmitted);

    //handling the login submitted
    connect(loginPage, &LoginPage::loginSubmitted, this, &MainWindow::handleLoginSubmitted);

    connect (loginPage, &LoginPage::passwordResetRequested, this, &MainWindow::showPasswordResetPage);

    connect(loginPage, &LoginPage::backRequested, this, &MainWindow::showLandingPage);


    connect(passwordResetPage, &PasswordResetPage::backRequested, this, &MainWindow::showLoginPage);
}



MainWindow::~MainWindow(){
}

void MainWindow::showLandingPage(){

    stack -> setCurrentIndex(LandingIndex);
}

void MainWindow::showSignUpPage(){
    stack -> setCurrentIndex(SignUpIndex);
}

void MainWindow::showLoginPage(){
    stack -> setCurrentIndex(LoginIndex);
}

void MainWindow::showPasswordResetPage(){
    stack -> setCurrentIndex(PasswordReset);
}


void MainWindow::handleSignUpSubmitted(const QString& fname, const QString& lname,
                                       const QString& email,
                                       const QString& password,
                                       const QString& confirmPassword){

    qDebug() << "Sign-up submitted:"
            << "name = " << fname << " " << lname
            << ", email  = " << email;

    if (password != confirmPassword){
        qDebug() << "Passwords do not match,";
        return;
    }

    showLandingPage();
}

void MainWindow::handleLoginSubmitted(const QString& email, const QString& password){


}

void MainWindow::resetPasswordSubmitted(const QString oldPassword, const QString newPassword)
{
    // TODO: implement your real logic here
    qDebug() << "Password reset requested. Old password:" << oldPassword
             << "New password:" << newPassword;

    // Example: switch back to login page after reset
    showLoginPage();
}