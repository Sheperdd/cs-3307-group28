//
// Created by svkol on 2026-02-28.
//

#include "MainWindow.h"
#include <QStackedWidget>
#include <QDebug>

#include "LandingPage.h"
#include "LoginPage.h"
#include "SignUpPage.h"


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent){

    //creating a stacked widget and setting it in the center
    stack = new QStackedWidget(this);
    setCentralWidget(stack);

    //creating pages
    landingPage = new LandingPage(this);
    signUpPage = new SignUpPage(this);

    //creating stack in known order
    stack->addWidget(landingPage);
    stack->addWidget(signUpPage);

    //starting on the landing page
    stack -> setCurrentIndex(LandingIndex);

    //Creating navigations
    // From landing page to sign-up page
connect(landingPage, &LandingPage::signUpRequested,
        this, &MainWindow::showSignUpPage);

// From sign-up page back to landing page
connect(signUpPage, &SignUpPage::backRequested,
        this, &MainWindow::showLandingPage);

// Submission of sign-up
connect(signUpPage, &SignUpPage::signUpSubmitted,
        this, &MainWindow::handleSignUpSubmitted);

connect(loginPage, &LoginPage::loginSubmitted, this, &MainWindow::handleLoginSubmitted);
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
