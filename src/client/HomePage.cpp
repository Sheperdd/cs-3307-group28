//
// Created by svkol on 2026-03-05.
//

#include "HomePage.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QSpacerItem>

HomePage::HomePage(QWidget *parent)
    : QWidget(parent),
      titleLabel(nullptr),
      findAppointmentsButton(nullptr),
      jobDoneButton(nullptr),
      messagesButton(nullptr),
      logoutButton(nullptr)
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    mainLayout->setSpacing(20);

    // Title
    titleLabel = new QLabel(tr("Home"), this);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(titleFont.pointSize() + 6);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);

    mainLayout->addWidget(titleLabel);

    // Spacer
    mainLayout->addStretch(1);

    // Main action buttons
    findAppointmentsButton = new QPushButton(tr("Find Appointments"), this);
    jobDoneButton          = new QPushButton(tr("Completed Jobs"), this);
    messagesButton         = new QPushButton(tr("Messages"), this);
    logoutButton           = new QPushButton(tr("Log Out"), this);

    findAppointmentsButton->setMinimumHeight(40);
    jobDoneButton->setMinimumHeight(40);
    messagesButton->setMinimumHeight(40);
    logoutButton->setMinimumHeight(30);

    mainLayout->addWidget(findAppointmentsButton);
    mainLayout->addWidget(jobDoneButton);
    mainLayout->addWidget(messagesButton);

    // Spacer before logout
    mainLayout->addStretch(1);

    mainLayout->addWidget(logoutButton, 0, Qt::AlignRight);

    // Wire button clicks to signals
    connect(findAppointmentsButton, &QPushButton::clicked,
            this, &HomePage::findAppointmentsRequested);
    connect(jobDoneButton, &QPushButton::clicked,
            this, &HomePage::jobDoneRequested);
    connect(messagesButton, &QPushButton::clicked,
            this, &HomePage::messagesRequested);
    connect(logoutButton, &QPushButton::clicked,
            this, &HomePage::logoutRequested);
}

HomePage::~HomePage() = default;
