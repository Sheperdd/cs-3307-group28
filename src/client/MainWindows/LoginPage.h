/**
 * @file LoginPage.h
 * @brief Login form — email + password fields, forgot-password link.
 */

#ifndef LOGIN_H
#define LOGIN_H
#include <qobjectdefs.h>
#include <QApplication>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

class LoginPage : public QWidget {

    Q_OBJECT

public:
    LoginPage(QWidget *parent);
    ~LoginPage() override;

    void showErrorMessages(const QString &message);

signals:
    void loginSubmitted(const QString &email, const QString &password);
    void backRequested();
    void passwordResetRequested();
private:

    QLabel *title, *error;
    QLineEdit *email, *password;
    QPushButton *loginButton, *backButton, *forgotPasswordButton;


};



#endif //LOGIN_H
