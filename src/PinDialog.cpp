#include "PinDialog.hpp"
#include <qaction.h>
#include <qapplication.h>
#include <qboxlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlogging.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qpushbutton.h>
#include <qtmetamacros.h>
#include <qwidget.h>
#include <QCloseEvent>

PinDialog::PinDialog(QWidget *parent) : QWidget(parent) {
    setWindowTitle("pkhey");
    setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint);

    pLabel = new QLabel(this);

    pInput = new QLineEdit(this);
    pInput->setEchoMode(QLineEdit::Password);

    pSubmit = new QPushButton("Submit", this);
    pCancel = new QPushButton("Cancel", this);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(pLabel);
    layout->addWidget(pInput);
    layout->addWidget(pSubmit);
    layout->addWidget(pCancel);
    connect(pSubmit, &QPushButton::clicked, this, [this]() {
        emit pinSubmitted(pInput->text());
    });
    connect(pCancel, &QPushButton::clicked, this, [this]() {
        emit cancelled();
    });

    QAction *exitAction = new QAction("&Exit", this);
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &PinDialog::cancelled);
    addAction(exitAction);
}

void PinDialog::reset() {
    pInput->clear();
}

void PinDialog::closeEvent(QCloseEvent *event) {
    qDebug() << "close action fired";
    emit cancelled();
    event->accept();
}

void PinDialog::setIdentity(const QString &identity) {
    pLabel->setText(identity);
}
