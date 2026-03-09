//
// Created by svkol on 2026-02-28.
//

#include "SignUpPage.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>


SignUpPage::SignUpPage(QWidget* parent): QWidget(parent){
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignTop);
    mainLayout->setContentsMargins(40,40,40,40);
    mainLayout->setSpacing(16);

    //title
    m_titleLabel = new QLabel(tr("Create a TorqueDesk Account"), this);
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(20);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);

    //Error Labels
    m_errorLabel = new QLabel(this);
    m_errorLabel->setStyleSheet("color: red ");
    m_errorLabel->setVisible(false);

    //Inputs
    m_fnameEdit = new QLineEdit(this);
    m_fnameEdit-> setPlaceholderText("Please enter your first name");

    m_lnameEdit = new QLineEdit(this);
    m_lnameEdit-> setPlaceholderText("Please enter your last name");

    m_emailEdit = new QLineEdit(this);
    m_emailEdit->setPlaceholderText(tr("Please enter your email"));

    m_userType = new QComboBox(this );
    m_userType -> addItem(tr("Customer"), QVariant("customer"));
    m_userType -> addItem(tr("Mechanic"), QVariant("mechanic"));

    m_passwordEdit = new QLineEdit();
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText("Please create your password");

    m_confirmPasswordEdit = new QLineEdit();
    m_confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    m_confirmPasswordEdit->setPlaceholderText("Please confirm your password");

    //Inputs form
    auto *formLayout = new QFormLayout();
    formLayout -> addRow(tr("First Name"), m_fnameEdit);
    formLayout -> addRow(tr("Last Name"), m_lnameEdit);
    formLayout -> addRow(tr("Email"), m_emailEdit);
    formLayout -> addRow(tr("Select type of the account "), m_userType);
    formLayout -> addRow(tr("Password"), m_passwordEdit);
    formLayout -> addRow(tr("Confirm Password"), m_confirmPasswordEdit);

    //Buttons
    signUpButton = new QPushButton(tr("Sign up"), this);
    backButton = new QPushButton(tr("Back"), this);

    auto *buttonsLayout = new QHBoxLayout();
    buttonsLayout -> addWidget(backButton);
    buttonsLayout->addStretch();
    buttonsLayout -> addWidget(signUpButton);

    //Assemble
    mainLayout ->addWidget(m_titleLabel);
    mainLayout -> addSpacing(8);
    mainLayout -> addWidget(m_errorLabel);
    mainLayout-> addLayout(formLayout);
    mainLayout -> addStretch();
    mainLayout -> addLayout(buttonsLayout);


    //Connections
    connect(signUpButton, &QPushButton::clicked, this, [this](){
        emit signUpSubmitted(
            m_fnameEdit -> text(),
            m_lnameEdit -> text(),
            m_emailEdit -> text(),
            m_userType->currentData().toString(),
            m_passwordEdit -> text(),
            m_confirmPasswordEdit-> text()
        );
    });

    connect(backButton, &QPushButton::clicked, this, [this](){
        emit backRequested();
    });

}

SignUpPage::~SignUpPage()= default;

void SignUpPage::setErrorMessage(const QString& message){
    if (message.isEmpty()){
        m_errorLabel -> clear();
        m_errorLabel -> setVisible(false);
    }else{
        m_errorLabel -> setText(message);
        m_errorLabel -> setVisible(true);
    }
}
