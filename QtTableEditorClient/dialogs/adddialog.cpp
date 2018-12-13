#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QCalendarWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QGroupBox>
#include <QButtonGroup>
#include <QRadioButton>

#include "adddialog.h"

AddStudentDialog::AddStudentDialog(DatabaseManager* mng, QWidget* parent)
    : QDialog(parent)
{
    setManager(mng);

    firstNameInput = new QLineEdit(this);
    firstNameInput->setPlaceholderText(tr("Enter your name..."));
    secondNameInput = new QLineEdit(this);
    secondNameInput->setPlaceholderText(tr("Enter the last name ..."));
    middleNameInput = new QLineEdit(this);
    middleNameInput->setPlaceholderText(tr("Enter the middle name ..."));

    birthDateInput = new QCalendarWidget(this);
    birthDateInput->setGridVisible(true);
    birthDateInput->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    enrollDateInput = new QCalendarWidget(this);
    enrollDateInput->setGridVisible(true);
    enrollDateInput->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    graduateDateInput = new QCalendarWidget(this);
    graduateDateInput->setGridVisible(true);
    graduateDateInput->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);

    manageButtons();
    manageDateSwitcher();
    manageLayouts();

    connect(manager, SIGNAL(invalidStudentInserted()), this, SLOT(notifyInvalidInput()));
    connect(manager, SIGNAL(duplicateStudentInserted()), this, SLOT(notifyDuplication()));
}

void AddStudentDialog::addStudentToDatabase()
{
    QString first = firstNameInput->text();
    QString second = secondNameInput->text();
    QString middle = middleNameInput->text();

    QDate birth = birthDateInput->selectedDate();
    QDate enroll = enrollDateInput->selectedDate();
    QDate graduate = graduateDateInput->selectedDate();

    if (manager)
    {
        manager->addStudent(first, second, middle, birth, enroll, graduate);
    }
}

void AddStudentDialog::notifyInvalidInput()
{
    QMessageBox::warning(this,
                         tr("Mistake"),
                         tr("Incorrect information entered!"),
                         QMessageBox::Ok);
}

void AddStudentDialog::notifyDuplication()
{
    QMessageBox::warning(this,
                         tr("Mistake"),
                         tr("A record of such a student already exists!"),
                         QMessageBox::Ok);
}

void AddStudentDialog::manageButtons()
{
    ok = new QPushButton(tr("To add"), this);
    connect(ok, SIGNAL(clicked(bool)), this, SLOT(addStudentToDatabase()));
    ok->setDefault(true);

    cancel = new QPushButton(tr("Cancel"), this);
    connect(cancel, SIGNAL(clicked(bool)), this, SLOT(reject()));
}

void AddStudentDialog::manageLayouts()
{
    QVBoxLayout* verticalMain = new QVBoxLayout();
    QVBoxLayout* verticalTop = new QVBoxLayout();
    QHBoxLayout* horizontalBottom = new QHBoxLayout();
    QHBoxLayout* buttonsLayout = new QHBoxLayout();

    verticalTop->addWidget(firstNameInput);
    verticalTop->addWidget(secondNameInput);
    verticalTop->addWidget(middleNameInput);
    verticalTop->addWidget(dateSwitcher);

    horizontalBottom->addWidget(dates);

    buttonsLayout->addWidget(ok);
    buttonsLayout->addWidget(cancel);

    verticalMain->addLayout(verticalTop);
    verticalMain->addLayout(horizontalBottom);
    verticalMain->addLayout(buttonsLayout);

    setLayout(verticalMain);
}

void AddStudentDialog::manageDateSwitcher()
{
    dateSwitcher = new QGroupBox(tr("Select a date:"), this);
    QRadioButton* birthDateButton = new QRadioButton(tr("Date of Birth"));
    QRadioButton* enrollDateButton = new QRadioButton(tr("receipt date"));
    QRadioButton* graduateDateButton = new QRadioButton(tr("expiration date"));

    QHBoxLayout* groupBoxLayout = new QHBoxLayout();
    groupBoxLayout->addWidget(birthDateButton);
    groupBoxLayout->addWidget(enrollDateButton);
    groupBoxLayout->addWidget(graduateDateButton);
    dateSwitcher->setLayout(groupBoxLayout);

    QButtonGroup* radioButtons = new QButtonGroup();
    radioButtons->addButton(birthDateButton, 0);
    radioButtons->addButton(enrollDateButton, 1);
    radioButtons->addButton(graduateDateButton, 2);
    birthDateButton->setChecked(true);

    dates = new QStackedWidget(this);
    dates->addWidget(birthDateInput);
    dates->addWidget(enrollDateInput);
    dates->addWidget(graduateDateInput);
    dates->setCurrentIndex(0);

    connect(radioButtons, SIGNAL(buttonToggled(int, bool)), dates, SLOT(setCurrentIndex(int)));
}

void AddStudentDialog::setManager(DatabaseManager* value)
{
    manager = value;
}
