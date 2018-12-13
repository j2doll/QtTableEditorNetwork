#include "serverwindow.h"
#include "../server/server.h"

#include <QLabel>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTime>

ServerWindow::ServerWindow(QWidget *parent)
    : QWidget(parent)
{
    runServerButton = new QPushButton(tr("Launch"));
    runServerButton->setDefault(true);
    stopServerButton = new QPushButton(tr("Stop"));
    clearLogButton = new QPushButton(tr("Clear log"));

    log = new QPlainTextEdit();
    log->setReadOnly(true);

    serverStatusLabel = new QLabel(tr("not running"));

    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    QVBoxLayout* mainLayout = new QVBoxLayout();

    buttonsLayout->addWidget(runServerButton);
    buttonsLayout->addWidget(stopServerButton);

    mainLayout->addLayout(buttonsLayout);
    mainLayout->addWidget(serverStatusLabel, 0, Qt::AlignCenter);
    mainLayout->addWidget(log);
    mainLayout->addWidget(clearLogButton);

    setLayout(mainLayout);

    server = new Server();
    connect(runServerButton, SIGNAL(clicked(bool)), this, SLOT(runServer()));
    connect(stopServerButton, SIGNAL(clicked(bool)), this, SLOT(stopServer()));
    connect(server, SIGNAL(serverMessage(QString)), this, SLOT(showMessage(QString)));
    connect(clearLogButton, SIGNAL(clicked(bool)), log, SLOT(clear()));

    showMessage(tr("Welcome!"));
}

void ServerWindow::runServer()
{
    server->runServer();
    displayServerStatus(QString("launched:%1:%2")
                        .arg(server->serverAddress().toString())
                        .arg(QString::number(server->serverPort())));
}

void ServerWindow::stopServer()
{
    server->stopServer();
    displayServerStatus("not running");
}

void ServerWindow::showMessage(QString message)
{
    log->appendPlainText(QTime::currentTime().toString() + ": " + message);
}

void ServerWindow::displayServerStatus(QString status)
{
    serverStatusLabel->setText(status);
}
