#include "PasswordResetPage.h"

#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QFormLayout>

PasswordResetPage::PasswordResetPage(QWidget *parent)
    : QWidget(parent)
{
    auto *mainLayout = new QVBoxLayout(this);

    auto *titleLabel = new QLabel(tr("Reset Password"), this);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(titleFont.pointSize() + 4);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);

    auto *formLayout = new QFormLayout();

    oldPasswordEdit = new QLineEdit(this);
    oldPasswordEdit->setEchoMode(QLineEdit::Password);
    oldPasswordEdit->setPlaceholderText(tr("Enter current password"));

    newPasswordEdit = new QLineEdit(this);
    newPasswordEdit->setEchoMode(QLineEdit::Password);
    newPasswordEdit->setPlaceholderText(tr("Enter new password"));

    formLayout->addRow(tr("Current password:"), oldPasswordEdit);
    formLayout->addRow(tr("New password:"), newPasswordEdit);

    resetButton = new QPushButton(tr("Reset Password"), this);
    backButton  = new QPushButton(tr("Back"), this);

    auto *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(backButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(resetButton);

    mainLayout->addWidget(titleLabel);
    mainLayout->addLayout(formLayout);
    mainLayout->addStretch();
    mainLayout->addLayout(buttonLayout);

    connect(resetButton, &QPushButton::clicked,
            this, &PasswordResetPage::onResetButtonClicked);
    connect(backButton, &QPushButton::clicked,
            this, &PasswordResetPage::onBackButtonClicked);
}

void PasswordResetPage::onResetButtonClicked()
{
    const QString oldPwd = oldPasswordEdit->text();
    const QString newPwd = newPasswordEdit->text();
    emit passwordResetSubmitted(oldPwd, newPwd);
}

void PasswordResetPage::onBackButtonClicked()
{
    emit backRequested();
}