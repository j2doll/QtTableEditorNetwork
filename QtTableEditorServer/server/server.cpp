#include "server.h"
#include "clientwrapper.h"

#include <QNetworkInterface>
#include <QMessageBox>
#include <QThread>

Server::Server(QWidget *parent)
    :QTcpServer(parent)
{

}

void Server::runServer()
{
    QHostAddress ipAddress = QHostAddress::Any;

    if (!listen(ipAddress))
    {
        QString strError = errorString();
        emit serverMessage( tr("Could not start server: %1")
                            .arg(strError) );
        stopServer();
    }
    else
    {
        emit serverMessage(tr("Server successfully started at:%1:%2")
                           .arg(serverAddress().toString())
                           .arg(QString::number(serverPort())));
    }

    /*
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    foreach (QHostAddress ipAddress, ipAddressesList)
    {
        if (ipAddress != QHostAddress::LocalHost && ipAddress.toIPv4Address())
        {
            if (!listen(ipAddress))
            {
                QString strError = errorString();
                emit serverMessage( tr("Could not start server: %1")
                                    .arg(strError) );
                stopServer();
            }
            else
            {
                emit serverMessage(tr("Server successfully started at:%1:%2")
                                   .arg(serverAddress().toString())
                                   .arg(QString::number(serverPort())));
            }
        }
    }
    */
}

void Server::stopServer()
{
    emit serverClosed();
    close();
    emit serverMessage(tr("Server successfully stopped"));
}

void Server::clientConnected(const QHostAddress& clientAddress, quint16 clientPort)
{
    emit serverMessage(tr("Client connected:%1:%2")
                       .arg(clientAddress.toString())
                       .arg(QString::number(clientPort)));
}

void Server::clientDisconnected(const QHostAddress& clientAddress, quint16 clientPort)
{
    emit serverMessage(tr("Client disconnected:%1:%2").
                       arg(clientAddress.toString()).
                       arg(QString::number(clientPort)));
}

void Server::requestReceived(const QString& request)
{
    auto client = qobject_cast<ThreadableClientWrapper *>(QObject::sender());
    emit serverMessage(tr("[%1:%2] Request received:%3")
                       .arg(client->getClientAddress().toString())
                       .arg(QString::number(client->getClientPort()))
                       .arg(request));
}

void Server::responseSent(const QString &response)
{
    auto client = qobject_cast<ThreadableClientWrapper *>(QObject::sender());
    emit serverMessage(tr("[%1:%2] Responded:%3")
                       .arg(client->getClientAddress().toString())
                       .arg(QString::number(client->getClientPort()))
                       .arg(response));
}

void Server::incomingConnection(qintptr socketDescriptor)
{
    ThreadableClientWrapper* client = new ThreadableClientWrapper(socketDescriptor);
    QThread* clientThread = new QThread();
    client->moveToThread(clientThread);

    connect(this, SIGNAL(serverClosed()), client, SLOT(stop()));
    connect(clientThread, SIGNAL(started()), client, SLOT(run()));
    connect(client, SIGNAL(finished()), clientThread, SLOT(quit()));
    connect(client, SIGNAL(finished()), client, SLOT(deleteLater()));
    connect(client, SIGNAL(finished()), clientThread, SLOT(deleteLater()));

    qRegisterMetaType<QHostAddress>();
    connect(client, SIGNAL(clientConnected(QHostAddress,quint16)), this, SLOT(clientConnected(QHostAddress,quint16)));
    connect(client, SIGNAL(clientDisconnected(QHostAddress,quint16)), this, SLOT(clientDisconnected(QHostAddress,quint16)));
    connect(client, SIGNAL(requestReceived(QString)), this, SLOT(requestReceived(QString)));
    connect(client, SIGNAL(responseSent(QString)), this, SLOT(responseSent(QString)));

    clientThread->start();
}

