/**
 * @file HomePage.h
 * @brief Post-login home screen with navigation buttons.
 */

#ifndef HOMEPAGE_H
#define HOMEPAGE_H

#include <QWidget>

#include <memory>

class QVBoxLayout;
class QHBoxLayout;
class QPushButton;
class QLabel;
class QSpacerItem;


class HomePage : public QWidget{
    Q_OBJECT
public:
    HomePage(QWidget *parent = nullptr);
    ~HomePage() override;

signals:
    void findAppointmentsRequested();
    void jobDoneRequested();
    void messagesRequested();
    void logoutRequested();

private:
    QLabel      *titleLabel;
    QPushButton *findAppointmentsButton;
    QPushButton *jobDoneButton;
    QPushButton *messagesButton;
    QPushButton *logoutButton;
};



#endif //HOMEPAGE_H