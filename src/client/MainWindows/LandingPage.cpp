/**
 * @file LandingPage.cpp
 * @brief LandingPage layout and button signal wiring.
 */
#include "LandingPage.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>

LandingPage::LandingPage(QWidget *parent)
    : QWidget(parent)
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignCenter);

    // Title
    m_titleLabel = new QLabel(tr("TorqueDesk"), this);
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setAlignment(Qt::AlignHCenter);

    // Tagline
    m_taglineLabel = new QLabel(tr("Welcome. Choose how you want to continue."), this);
    m_taglineLabel->setAlignment(Qt::AlignHCenter);

    // Description
    m_descriptionLabel = new QLabel(tr("Please log in or create an account to continue."), this);
    m_descriptionLabel->setAlignment(Qt::AlignHCenter);
    m_descriptionLabel->setWordWrap(true);

    // Buttons
    m_loginButton = new QPushButton(tr("Log In"), this);
    m_signupButton = new QPushButton(tr("Sign Up"), this);

    //Button layout
    auto *buttonsLayout = new QHBoxLayout();
    buttonsLayout->addWidget(m_loginButton);
    buttonsLayout->addWidget(m_signupButton);

    // Add widgets to layout
    mainLayout->addWidget(m_titleLabel);
    mainLayout->addWidget(m_taglineLabel);
    mainLayout->addWidget(m_descriptionLabel);
    mainLayout->addSpacing(16);
    mainLayout->addLayout(buttonsLayout);

    // Emit the signals when buttons are clicked
    connect(m_loginButton, &QPushButton::clicked, this, [this]() {
        emit loginRequested();
    });
    connect(m_signupButton, &QPushButton::clicked, this, [this]() {
        emit signUpRequested();
    });
}

LandingPage::~LandingPage() = default;