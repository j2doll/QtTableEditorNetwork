#include "clientwrapper.h"
#include "../model/database.h"
#include "../socket/socketadapter.h"

#include <QTcpSocket>
#include <QHostAddress>

ThreadableClientWrapper::ThreadableClientWrapper(qintptr socketDescriptor, QObject *parent)
    : QObject(parent), socketDescriptor(socketDescriptor)
{

}

ThreadableClientWrapper::~ThreadableClientWrapper()
{
    if (database)
    {
        delete database;
    }
    if (socket)
    {
        delete socket;
    }
}

void ThreadableClientWrapper::run()
{
    QTcpSocket* sock = new QTcpSocket();
    // error handling needed
    sock->setSocketDescriptor(socketDescriptor);
    socket = new TcpSocketAdapter(sock, this);

    emit clientConnected(getClientAddress(), getClientPort());

    connect(socket, SIGNAL(requestReceived(TcpSocketAdapter::REQUESTS,QString)),
            this, SLOT(parseRequest(TcpSocketAdapter::REQUESTS,QString)));
    connect(socket->getSocket(), SIGNAL(disconnected()), this, SLOT(socketDisconnected()));

    database = new StudentDatabase();

    connect(database, SIGNAL(studentAdded()), this, SLOT(databaseUpdated()));
    connect(database, SIGNAL(studentDeleted()), this, SLOT(databaseUpdated()));
    connect(database, SIGNAL(studentsDeleted(int)), this, SLOT(studentsDeleted(int)));
    connect(database, SIGNAL(invalidInsertion()), this, SLOT(invalidStudentInserted()));
    connect(database, SIGNAL(duplicateInsertion()), this, SLOT(duplicateStudentInserted()));
}

void ThreadableClientWrapper::stop()
{
    disconnect(socket, SIGNAL(requestReceived(TcpSocketAdapter::REQUESTS,QString)),
               this, SLOT(parseRequest(TcpSocketAdapter::REQUESTS,QString)));
    disconnect(socket->getSocket(), SIGNAL(disconnected()), this, SLOT(stop()));

    disconnect(database, SIGNAL(studentAdded()), this, SLOT(databaseUpdated()));
    disconnect(database, SIGNAL(studentDeleted()), this, SLOT(databaseUpdated()));
    disconnect(database, SIGNAL(studentsDeleted(int)), this, SLOT(studentsDeleted(int)));
    disconnect(database, SIGNAL(invalidInsertion()), this, SLOT(invalidStudentInserted()));
    disconnect(database, SIGNAL(duplicateInsertion()), this, SLOT(duplicateStudentInserted()));

    getSocketAdapter()->getSocket()->disconnectFromHost();

    emit finished();
}

void ThreadableClientWrapper::socketDisconnected()
{
    emit clientDisconnected(getClientAddress(), getClientPort());
    stop();
}

void ThreadableClientWrapper::parseRequest(TcpSocketAdapter::REQUESTS requestId, QString data)
{
    using Tcp = TcpSocketAdapter;
    switch (requestId)
    {
    case Tcp::REQUESTS::ADD_STUDENT:
    {
        emit requestReceived(tr("adding record"));
        Student student = Student::fromString(data);
        database->addStudent(student);
        break;
    }
    case Tcp::REQUESTS::SEARCH_STUDENTS:
    {
        emit requestReceived(tr("search records"));
        StudentSearchPattern pattern = StudentSearchPattern::fromString(data);
        database->setSearchPattern(pattern);
        break;
    }
    case Tcp::REQUESTS::REMOVE_STUDENTS:
    {
        emit requestReceived(tr("deleting records"));
        StudentSearchPattern pattern = StudentSearchPattern::fromString(data);
        database->removeStudents(pattern);
        break;
    }
    case Tcp::REQUESTS::GET_PAGE:
    {
        QStringList numbers = data.split(QChar(' '));
        emit requestReceived(tr("page receipt: No.%1 (per%2)")
                             .arg(numbers[0].toInt())
                             .arg(numbers[1].toInt()) );

        Student::StudentSet page = database->getSetOfStudents(numbers[0].toInt(), numbers[1].toInt());
        socket->sendRequest(Tcp::REQUESTS::GET_PAGE, Student::studentsToString(page));
        emit responseSent(tr("page submission: size - %1").arg(QString::number(page.size())));
        break;
    }
    case Tcp::REQUESTS::COUNT_PAGES:
    {
        emit requestReceived(tr("page counting"));
        int studentsPerPage = data.toInt();
        int pages = database->countPages(studentsPerPage);

        socket->sendRequest(Tcp::REQUESTS::COUNT_PAGES, QString::number(pages));
        emit responseSent(tr("number of pages %1").arg(QString::number(pages)));
        break;
    }
    case Tcp::REQUESTS::VALIDATE_PAGE:
    {
        emit requestReceived(tr("page check"));
        QStringList numbers = data.split(QChar(' '));
        bool isValidPage = database->validatePageBounds(numbers[0].toInt(), numbers[1].toInt());

        socket->sendRequest(Tcp::REQUESTS::VALIDATE_PAGE, isValidPage ? "true" : "false");
        emit responseSent(tr("verified page:%1").arg(isValidPage ? "true" : "false"));
        break;
    }
    case Tcp::REQUESTS::SAVE_DATABASE:
    {
        QString fileName = data;
        emit requestReceived(tr("save to file: %1").arg(fileName));
        database->getXmlHandler()->writeToFile(fileName);
        break;
    }
    case Tcp::REQUESTS::LOAD_DATABASE:
    {
        QString fileName = data;
        emit requestReceived(tr("Loading from file: %1").arg(fileName));
        database->getXmlHandler()->readFromFile(fileName);
        break;
    }
    case Tcp::REQUESTS::LOAD_FILES:
    {
        emit requestReceived(tr("list of available files"));
        QStringList files = database->getXmlHandler()->getAvailableFiles();
        socket->sendRequest(Tcp::REQUESTS::LOAD_FILES, files.join(QChar('|')));
        emit responseSent(tr("list of available files "));
        break;
    }
    }
}

void ThreadableClientWrapper::databaseUpdated()
{
    socket->sendRequest(TcpSocketAdapter::REQUESTS::DATABASE_UPDATED);
    emit responseSent(tr("list of records updated"));
}

void ThreadableClientWrapper::studentsDeleted(int amount)
{
    QString data = QString::number(amount);
    socket->sendRequest(TcpSocketAdapter::REQUESTS::STUDENTS_DELETED, data);
    emit responseSent(tr("%1 records deleted").arg(amount));
}

void ThreadableClientWrapper::invalidStudentInserted()
{
    socket->sendRequest(TcpSocketAdapter::REQUESTS::INVALID_INSERTION);
    emit responseSent(tr("add incorrect entry"));
}

void ThreadableClientWrapper::duplicateStudentInserted()
{
    socket->sendRequest(TcpSocketAdapter::REQUESTS::DUPLICATE_INSERTION);
    emit responseSent(tr("re-add entry"));
}

TcpSocketAdapter* ThreadableClientWrapper::getSocketAdapter() const
{
    return socket;
}

QHostAddress ThreadableClientWrapper::getClientAddress() const
{
    return getSocketAdapter()->getSocket()->peerAddress();
}

quint16 ThreadableClientWrapper::getClientPort() const
{
    return getSocketAdapter()->getSocket()->peerPort();
}
