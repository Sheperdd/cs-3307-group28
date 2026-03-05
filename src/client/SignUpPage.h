//
// Created by svkol on 2026-02-28.
//

#ifndef SIGNUPPAGE_H
#define SIGNUPPAGE_H

#pragma once
#include <QWidget>

class QLineEdit;
class QPushButton;
class QLabel;


class SignUpPage : public QWidget{

    Q_OBJECT

public:
    explicit SignUpPage(QWidget *parent = nullptr);
    ~SignUpPage();

    void setErrorMessage(const QString &message);

signals:
    void signUpSubmitted(const QString &fname,
        const QString &lname,
        const QString &mail,
        const QString &password,
        const QString &confirmPassword);

    void backRequested();

private:
    QLabel *m_titleLabel ,*m_errorLabel;
    QLineEdit *m_fnameEdit, *m_lnameEdit, *m_emailEdit, *m_passwordEdit, *m_confirmPasswordEdit;

    QPushButton signUpButton, backButton;
};



#endif //SIGNUPPAGE_H
