#include "internetradiowindow.h"
#include "ui_internetradiowindow.h"

InternetRadioWindow::InternetRadioWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::InternetRadioWindow)
{
    ui->setupUi(this);
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5StackedWindow);
#endif
    QMainWindow::setCentralWidget(ui->verticalLayoutWidget);
    this->connectSignals();
}

InternetRadioWindow::~InternetRadioWindow()
{
    delete ui;
}

void InternetRadioWindow::connectSignals()
{
    connect(ui->actionFM_transmitter, SIGNAL(triggered()), this, SLOT(showFMTXDialog()));
    connect(ui->actionAdd_radio_bookmark, SIGNAL(triggered()), this, SLOT(showAddBookmarkDialog()));
}

void InternetRadioWindow::showFMTXDialog()
{
#ifdef Q_WS_MAEMO_5
    osso_context1 = osso_initialize("qt-mediaplayer", "0.1", TRUE, NULL);
    osso_cp_plugin_execute(osso_context1, "libcpfmtx.so", this, TRUE);
#endif
}

void InternetRadioWindow::showAddBookmarkDialog()
{
    bookmarkDialog = new QDialog(this);
    bookmarkDialog->setWindowTitle(tr("Add radio bookmark"));
    horizontalLayout = new QHBoxLayout(bookmarkDialog);

    nameLabel = new QLabel(bookmarkDialog);
    nameLabel->setText(tr("Name"));
    addressLabel = new QLabel(bookmarkDialog);
    addressLabel->setText(tr("Web address"));

    labelLayout = new QVBoxLayout(bookmarkDialog);
    labelLayout->addWidget(nameLabel);
    labelLayout->addWidget(addressLabel);

    nameBox = new QLineEdit(bookmarkDialog);
    addressBox = new QLineEdit(bookmarkDialog);

    lineEditLayout = new QVBoxLayout(bookmarkDialog);
    lineEditLayout->addWidget(nameBox);
    lineEditLayout->addWidget(addressBox);

    verticalLayout_1 = new QVBoxLayout(bookmarkDialog);
    verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    saveButton = new QPushButton(bookmarkDialog);
    saveButton->setText(tr("Save"));
    verticalLayout_1->addItem(verticalSpacer);
    verticalLayout_1->addWidget(saveButton);

    horizontalLayout->addLayout(labelLayout);
    horizontalLayout->addLayout(lineEditLayout);
    horizontalLayout->addLayout(verticalLayout_1);

    bookmarkDialog->setLayout(horizontalLayout);
    bookmarkDialog->setAttribute(Qt::WA_DeleteOnClose);
    bookmarkDialog->show();
}
