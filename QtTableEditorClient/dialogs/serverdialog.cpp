#include "serverdialog.h"
#include "../manager/manager.h"
#include "../socket/socketadapter.h"

#include <QTime>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QIntValidator>
#include <QPushButton>
#include <QLabel>
#include <QHostAddress>
#include <QTcpSocket>

ConnectToServerDialog::ConnectToServerDialog(DatabaseManager *mng, QWidget *parent)
    : QDialog(parent)
{
    setManager(mng);

    setWindowTitle(tr("Connect to server"));
    manageInputFields();
    manageButtons();
    manageLayouts();
}

DatabaseManager *ConnectToServerDialog::getManager() const
{
    return m_manager;
}

void ConnectToServerDialog::setManager(DatabaseManager *value)
{
    m_manager = value;
    if (m_manager)
    {
        connect(m_manager->getSocket()->getSocket(), SIGNAL(error(QAbstractSocket::SocketError)),
                this, SLOT(connectionError(QAbstractSocket::SocketError)));
        connect(m_manager->getSocket()->getSocket(), SIGNAL(stateChanged(QAbstractSocket::SocketState)),
                this, SLOT(connectionStateChanged(QAbstractSocket::SocketState)));
    }
}

void ConnectToServerDialog::connectToServer()
{
    QString addressString = m_addressInput->text();
    QString portString = m_portInput->text();

    QHostAddress address(addressString);
    int port = portString.toInt();

    if (!address.toIPv4Address())
    {
        showMessage(tr("Invalid address entered"));
        return;
    }
    if (0 == port)
    {
        showMessage(tr("Incorrect port entered"));
        return;
    }

    getManager()->connectToServer(address, port);
}

void ConnectToServerDialog::disconnectFromServer()
{
    getManager()->disconnectFromServer();
}

void ConnectToServerDialog::connectionStateChanged(QAbstractSocket::SocketState socketState)
{
    switch (socketState)
    {
    case QAbstractSocket::HostLookupState:
        showMessage(tr("Search server ..."));
        break;
    case QAbstractSocket::ConnectingState:
        showMessage(tr("Server found. Connecting ..."));
        break;
    case QAbstractSocket::ConnectedState:
        showMessage(tr("Successfully connected to server!"));
        break;
    case QAbstractSocket::ClosingState:
        showMessage(tr("Connection closing ..."));
        break;
    case QAbstractSocket::UnconnectedState:
        showMessage(tr("Server connection closed."));
        break;
    }
}

void ConnectToServerDialog::showMessage(const QString &message)
{
    m_log->appendPlainText(QTime::currentTime().toString() + ": " + message);
}

void ConnectToServerDialog::connectionError(QAbstractSocket::SocketError socketError)
{
    switch (socketError)
    {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    case QAbstractSocket::HostNotFoundError:
        showMessage(tr("Server not found. Check the correct IP address and port."));
        break;
    case QAbstractSocket::ConnectionRefusedError:
        showMessage(tr("Connection refused."));
        break;
    default:
        showMessage(tr("An error occurred: %1").arg(getManager()->getSocket()->getSocket()->errorString()));
    }
}

void ConnectToServerDialog::manageInputFields()
{
    m_addressLabel = new QLabel(tr("IP address:"));

    m_addressInput = new QLineEdit();

    QString ipRange = "(?:[0-1_]?[0-9_]?[0-9_]|2[0-4_][0-9_]|25[0-5_])";
    QRegExp ipRegex ("^" + ipRange + "\\." + ipRange + "\\." + ipRange + "\\." + ipRange + "$");
    QRegExpValidator *ipValidator = new QRegExpValidator(ipRegex, this);
    m_addressInput->setValidator(ipValidator);
    m_addressInput->setInputMask("000.000.000.000;_");

    m_portLabel = new QLabel(tr("Port:"));

    m_portInput = new QLineEdit();
    m_portInput->setValidator(new QIntValidator(1, 65535));

    m_log = new QPlainTextEdit();
    m_log->setReadOnly(true);
}

void ConnectToServerDialog::manageButtons()
{
    m_connectButton = new QPushButton(tr("Connect"));
    connect(m_connectButton, SIGNAL(clicked(bool)), this, SLOT(connectToServer()));
    m_connectButton->setDefault(true);

    m_disconnectButton = new QPushButton(tr("Disconnect"));
    connect(m_disconnectButton, SIGNAL(clicked(bool)), this, SLOT(disconnectFromServer()));
    m_disconnectButton->setDefault(true);

    m_cancelButton = new QPushButton(tr("Back"));
    connect(m_cancelButton, SIGNAL(clicked(bool)), this, SLOT(reject()));
}

void ConnectToServerDialog::manageLayouts()
{
    QHBoxLayout* inputLayout = new QHBoxLayout();
    inputLayout->addWidget(m_addressLabel);
    inputLayout->addWidget(m_addressInput);
    inputLayout->addWidget(m_portLabel);
    inputLayout->addWidget(m_portInput);

    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    buttonsLayout->addWidget(m_connectButton);
    buttonsLayout->addWidget(m_disconnectButton);
    buttonsLayout->addWidget(m_cancelButton);

    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->addLayout(inputLayout);
    mainLayout->addWidget(m_log);
    mainLayout->addLayout(buttonsLayout);

    setLayout(mainLayout);
}
