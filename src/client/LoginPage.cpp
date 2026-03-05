#include "LoginPage.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QFont>

LoginPage::LoginPage(QWidget *parent)
    : QWidget(parent),
      title(nullptr),
      error(nullptr),
      email(nullptr),
      password(nullptr),
      loginButton(nullptr),
      backButton(nullptr),
      forgotPasswordButton(nullptr)
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignTop);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    mainLayout->setSpacing(16);

    // Title
    title = new QLabel(tr("Log in to TorqueDesk"), this);
    QFont titleFont = title->font();
    titleFont.setPointSize(20);
    titleFont.setBold(true);
    title->setFont(titleFont);
    mainLayout->addWidget(title);

    // Error label
    error = new QLabel(this);
    error->setStyleSheet("color: red;");
    error->setVisible(false);
    mainLayout->addWidget(error);

    // Email and password fields
    email = new QLineEdit(this);
    email->setPlaceholderText(tr("Enter your email"));

    password = new QLineEdit(this);
    password->setPlaceholderText(tr("Enter your password"));
    password->setEchoMode(QLineEdit::Password);

    auto *formLayout = new QFormLayout();
    formLayout->addRow(tr("Email"), email);
    formLayout->addRow(tr("Password"), password);
    mainLayout->addLayout(formLayout);

    // Buttons
    loginButton = new QPushButton(tr("Log in"), this);
    forgotPasswordButton = new QPushButton(tr("Forgot Password?"), this);
    backButton = new QPushButton(tr("Back"), this);

    auto *buttonsLayout = new QHBoxLayout();
    buttonsLayout->addWidget(loginButton);
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(forgotPasswordButton);
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(backButton);

    mainLayout->addLayout(buttonsLayout);

    // Connections
    connect(loginButton, &QPushButton::clicked, this, [this]() {
        emit loginSubmitted(email->text(), password->text());
    });

    connect(forgotPasswordButton, &QPushButton::clicked, this, [this]() {
        emit passwordResetRequested();
    });

    connect(backButton, &QPushButton::clicked, this, [this]() {
        emit backRequested();
    });
}

LoginPage::~LoginPage() = default;

void LoginPage::showErrorMessages(const QString &message)
{
    if (message.isEmpty()) {
        error->clear();
        error->setVisible(false);
    } else {
        error->setText(message);
        error->setVisible(true);
    }
}