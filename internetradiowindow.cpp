#include "internetradiowindow.h"

InternetRadioWindow::InternetRadioWindow(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::InternetRadioWindow)
{
    ui->setupUi(this);
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5StackedWindow);
#endif
    ui->centralwidget->setLayout(ui->verticalLayout);
    window = new RadioNowPlayingWindow(this);
    this->connectSignals();
    this->orientationChanged();
}

InternetRadioWindow::~InternetRadioWindow()
{
    delete ui;
}

void InternetRadioWindow::connectSignals()
{
    connect(ui->actionFM_transmitter, SIGNAL(triggered()), this, SLOT(showFMTXDialog()));
    connect(ui->actionAdd_radio_bookmark, SIGNAL(triggered()), this, SLOT(showAddBookmarkDialog()));
    connect(ui->listWidget, SIGNAL(clicked(QModelIndex)), this, SLOT(onStationSelected()));
    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));
}

void InternetRadioWindow::showFMTXDialog()
{
#ifdef Q_WS_MAEMO_5
    FMTXDialog *fmtxDialog = new FMTXDialog(this);
    fmtxDialog->show();
#endif
}

void InternetRadioWindow::onStationSelected()
{
    window->show();
}

void InternetRadioWindow::showAddBookmarkDialog()
{
    bookmarkDialog = new QDialog(this);
    bookmarkDialog->setWindowTitle(tr("Add radio bookmark"));
    QRect screenGeometry = QApplication::desktop()->screenGeometry();

    // Labels
    nameLabel = new QLabel(bookmarkDialog);
    nameLabel->setText(tr("Name"));
    addressLabel = new QLabel(bookmarkDialog);
    addressLabel->setText(tr("Web address"));

    // Input boxes
    nameBox = new QLineEdit(bookmarkDialog);
    addressBox = new QLineEdit(bookmarkDialog);
    addressBox->setText("http://");

    // Spacer above save button
    QSpacerItem *verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

    // Save button
    saveButton = new QPushButton(bookmarkDialog);
    saveButton->setText(tr("Save"));
    saveButton->setDefault(true);
    connect(saveButton, SIGNAL(clicked()), this, SLOT(onSaveClicked()));
    buttonBox = new QDialogButtonBox(Qt::Vertical);
    buttonBox->addButton(saveButton, QDialogButtonBox::ActionRole);

    // Layouts
    QVBoxLayout *labelLayout = new QVBoxLayout();
    labelLayout->addWidget(nameLabel);
    labelLayout->addWidget(addressLabel);

    QVBoxLayout *boxLayout = new QVBoxLayout();
    boxLayout->addWidget(nameBox);
    boxLayout->addWidget(addressBox);

    QHBoxLayout *dialogLayout = new QHBoxLayout();
    if (screenGeometry.width() < screenGeometry.height()) {
        dialogLayout->setDirection(QBoxLayout::TopToBottom);
    }

    QVBoxLayout *saveButtonLayout = new QVBoxLayout();
    if (screenGeometry.width() > screenGeometry.height()) {
        saveButtonLayout->addItem(verticalSpacer);
    }
    saveButtonLayout->addWidget(buttonBox);

    // Pack all layouts together
    QHBoxLayout *horizontalLayout = new QHBoxLayout();
    horizontalLayout->addItem(labelLayout);
    horizontalLayout->addItem(boxLayout);

    dialogLayout->addItem(horizontalLayout);
    dialogLayout->addItem(saveButtonLayout);

    bookmarkDialog->setLayout(dialogLayout);
    bookmarkDialog->setAttribute(Qt::WA_DeleteOnClose);
    bookmarkDialog->show();
}

void InternetRadioWindow::onSaveClicked()
{
    if(nameBox->text().isEmpty()) {
#ifdef Q_WS_MAEMO_5
        QMaemo5InformationBox::information(this, tr("Unable to add empty bookmark"));
#else
        QMessageBox::critical(this, tr("Error"), tr("Unable to add empty bookmark"));
#endif
    } else {
        if(!saveButton->text().contains("http://")) {
#ifdef Q_WS_MAEMO_5
            QMaemo5InformationBox::information(this, tr("Invalid URL"));
#else
            QMessageBox::critical(this, tr("Error"), tr("Invalid URL"));
#endif
        } else {
            if(!saveButton->text().contains("*.*")) {
#ifdef Q_WS_MAEMO_5
                QMaemo5InformationBox::information(this, tr("Invalid URL"));
#else
                QMessageBox::critical(this, tr("Error"), tr("Invalid URL"));
#endif
            }
        }
    }
}

void InternetRadioWindow::orientationChanged()
{
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    ui->indicator->setGeometry(screenGeometry.width()-122, screenGeometry.height()-(70+55),
                               ui->indicator->width(),ui->indicator->height());
    ui->indicator->raise();
}
