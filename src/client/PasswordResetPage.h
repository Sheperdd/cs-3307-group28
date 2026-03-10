/**
 * @file PasswordResetPage.h
 * @brief Password reset form — old password + new password fields.
 */

#ifndef PASSWORDRESETPAGE_H
#define PASSWORDRESETPAGE_H



#include <QWidget>

class QLineEdit;
class QPushButton;
class QLabel;
class QVBoxLayout;
class QFormLayout;

class PasswordResetPage : public QWidget
{
    Q_OBJECT
public:
    explicit PasswordResetPage(QWidget *parent = nullptr);

    signals:
        void backRequested();
        void passwordResetSubmitted(const QString &oldPassword,
                                const QString &newPassword);

private slots:
    void onResetButtonClicked();
    void onBackButtonClicked();

private:
    QLineEdit   *oldPasswordEdit = nullptr;
    QLineEdit   *newPasswordEdit = nullptr;
    QPushButton *resetButton     = nullptr;
    QPushButton *backButton      = nullptr;
};


#endif //PASSWORDRESETPAGE_H
