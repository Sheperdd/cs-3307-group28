/**
 * @file LandingPage.h
 * @brief Initial screen with Log In / Sign Up buttons.
 */

#ifndef LANDINGPAGE_H
#define LANDINGPAGE_H

#pragma once
#include <QWidget>


class QPushButton;
class QLabel;

class LandingPage : public QWidget{

    Q_OBJECT

public:
    explicit LandingPage(QWidget * parent = nullptr);
    ~LandingPage();

signals:
    void loginRequested();
    void signUpRequested();

private:
    QLabel *m_titleLabel, *m_taglineLabel, *m_descriptionLabel;
    QPushButton *m_loginButton, *m_signupButton;

};



#endif //LANDINGPAGE_H
