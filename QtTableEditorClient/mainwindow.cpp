#include "mainwindow.h"
#include "table/table.h"
#include "dialogs/adddialog.h"
#include "dialogs/searchdialog.h"
#include "dialogs/deletedialog.h"
#include "dialogs/serverdialog.h"

#include <QMessageBox>
#include <QInputDialog>

MainWindow::MainWindow(DatabaseManager *mng, QWidget *parent)
    : QMainWindow(parent)
{
    setManager(mng);

    createActions();
    createMenus();
    createToolBars();

    table = new MultipageTable(mng, this);
    setCentralWidget(table);
    showMaximized();
}

MainWindow::~MainWindow()
{

}

void MainWindow::createActions()
{
    openFileAction = new QAction(QIcon(":/icons/icon_open_file.png"), tr("Open file"), this);
    openFileAction->setShortcut(QKeySequence::Open);
    openFileAction->setStatusTip(tr("Open the file with the table of students"));
    connect(openFileAction, SIGNAL(triggered(bool)), this, SLOT(requestOpenFile()));

    saveFileAction = new QAction(QIcon(":/icons/icon_save_file.png"), tr("Save file"), this);
    saveFileAction->setShortcut(QKeySequence::Save);
    saveFileAction->setStatusTip(tr("Save current student table"));
    connect(saveFileAction, SIGNAL(triggered(bool)), this, SLOT(saveFile()));

    saveAsAction = new QAction(QIcon(":/icons/icon_save_as_file.png"), tr("Save as ..."), this);
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    saveAsAction->setStatusTip(tr("Save the current student table to a new file."));
    connect(saveAsAction, SIGNAL(triggered(bool)), this, SLOT(saveFileAs()));

    addStudentAction = new QAction(QIcon(":/icons/icon_add.png"), tr("Add a note"), this);
    addStudentAction->setShortcut(Qt::Key_1);
    addStudentAction->setStatusTip(tr("Add a new student record to the table"));
    connect(addStudentAction, SIGNAL(triggered(bool)), this, SLOT(showAddDialog()));

    findStudentAction = new QAction(QIcon(":/icons/icon_find.png"), tr("Find records"), this);
    findStudentAction->setShortcut(Qt::Key_2);
    findStudentAction->setStatusTip(tr("Find student records by specified criteria"));
    connect(findStudentAction, SIGNAL(triggered(bool)), this, SLOT(showSearchDialog()));

    deleteStudentAction = new QAction(QIcon(":/icons/icon_delete.png"), tr("Delete records"), this);
    deleteStudentAction->setShortcut(Qt::Key_3);
    deleteStudentAction->setStatusTip(tr("Find and delete student records by specified criteria"));
    connect(deleteStudentAction, SIGNAL(triggered(bool)), this, SLOT(showDeleteDialog()));

    connectToServerAction = new QAction(QIcon(":/icons/icon_no_connection.png"), tr("Connect to server"), this);
    connectToServerAction->setShortcut(Qt::Key_4);
    connectToServerAction->setStatusTip(tr("Connect to remote server"));
    connect(connectToServerAction, SIGNAL(triggered(bool)), this, SLOT(showServerDialog()));

    aboutQtAction = new QAction(tr("About Qt"), this);
    aboutQtAction->setStatusTip(tr("Display Qt Library Reference"));
    connect(aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("File"));
    fileMenu->addAction(openFileAction);
    fileMenu->addAction(saveFileAction);
    fileMenu->addAction(saveAsAction);
    fileMenu->addSeparator();
    fileMenu->addAction(connectToServerAction);

    editMenu = menuBar()->addMenu(tr("Edit"));
    editMenu->addAction(addStudentAction);
    editMenu->addAction(findStudentAction);
    editMenu->addAction(deleteStudentAction);

    helpMenu = menuBar()->addMenu(tr("reference"));
    helpMenu->addAction(aboutQtAction);
}

void MainWindow::createToolBars()
{
    fileToolBar = new QToolBar(tr("Work with file"));
    fileToolBar->setIconSize(QSize(40, 40));
    fileToolBar->addAction(openFileAction);
    fileToolBar->addAction(saveFileAction);
    fileToolBar->addAction(saveAsAction);
    fileToolBar->addSeparator();
    fileToolBar->addAction(connectToServerAction);
    addToolBar(fileToolBar);

    editToolBar = new QToolBar(tr("Edit"));
    editToolBar->setIconSize(QSize(40, 40));
    editToolBar->addAction(addStudentAction);
    editToolBar->addAction(findStudentAction);
    editToolBar->addAction(deleteStudentAction);
    addToolBar(Qt::LeftToolBarArea, editToolBar);
}

bool MainWindow::saveConfirmation()
{
//    int respond = QMessageBox::warning(this, tr("Сохранить файл"),
//                                       tr("Вы хотите сохранить изменения?"),
//                                       QMessageBox::Yes | QMessageBox::No |
//                                       QMessageBox::Cancel);
//    if (respond == QMessageBox::Yes)
//        return saveFile();
//    else if (respond == QMessageBox::Cancel)
//        return false;
//    return true;
}

void MainWindow::openFile(const QStringList& availableFiles)
{
//    if (saveConfirmation())
//    {
    disconnect(getManager(), SIGNAL(availableFilesReceived(QStringList)),
               this, SLOT(openFile(QStringList)));
    bool ok;
    QString fileName = QInputDialog::getItem(this,
                                             tr("Loading"),
                                             tr("Enter the file name to upload:"),
                                             availableFiles,
                                             0,
                                             true,
                                             &ok);
    if (ok && !fileName.isEmpty())
    {
        getManager()->loadDatabaseFromFile(fileName);
    }
    currentFile = fileName;
//    }
}

void MainWindow::saveFile()
{
    if (currentFile.isEmpty())
        saveFileAs();
    else
        getManager()->saveDatabaseToFile(currentFile);
}

void MainWindow::saveFileAs()
{
    bool ok;
    QString fileName = QInputDialog::getText(this,
                                             tr("Preservation"),
                                             tr("Enter the name of the file to save:"),
                                             QLineEdit::Normal,
                                             tr("new_file"),
                                             &ok);
    if (ok)
    {
        currentFile = fileName;
        if (!fileName.isEmpty())
        {
            getManager()->saveDatabaseToFile(currentFile);
        }
    }
}

DatabaseManager *MainWindow::getManager() const
{
    return manager;
}

void MainWindow::setManager(DatabaseManager *value)
{
    manager = value;
    if (manager)
    {
        connect(manager->getSocket()->getSocket(), SIGNAL(disconnected()),
                this, SLOT(lostConnection()));
        connect(manager->getSocket()->getSocket(), SIGNAL(connected()),
                this, SLOT(connectionEstablished()));
    }
}

void MainWindow::showAddDialog()
{
    AddStudentDialog dialog(getManager(), this);
    dialog.exec();
}

void MainWindow::showSearchDialog()
{
    SearchStudentDialog dialog(getManager(), this);
    dialog.exec();
}

void MainWindow::showDeleteDialog()
{
    DeleteStudentDialog dialog(getManager(), this);
    dialog.exec();
}

void MainWindow::showServerDialog()
{
    ConnectToServerDialog dialog(getManager(), this);
    dialog.exec();
}

void MainWindow::lostConnection()
{
    connectToServerAction->setIcon(QIcon(":/icons/icon_no_connection.png"));
    QMessageBox::warning(this, tr("the Internet"), tr("Server connection lost!"),
                         QMessageBox::Ok);
}

void MainWindow::connectionEstablished()
{
    connectToServerAction->setIcon(QIcon(":/icons/icon_connection.png"));
}

void MainWindow::requestOpenFile()
{
    connect(getManager(), SIGNAL(availableFilesReceived(QStringList)),
            this, SLOT(openFile(QStringList)));
    getManager()->getAvailableFiles();
}
