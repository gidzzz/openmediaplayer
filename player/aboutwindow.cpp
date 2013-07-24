#include "aboutwindow.h"

AboutWindow::AboutWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::AboutWindow)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5StackedWindow);
#endif

    ui->iconLabel->setPixmap(QIcon::fromTheme("openmediaplayer").pixmap(64, 64));

    ui->buildInfo->setText("Build Date: " + QString(__DATE__ ) + " "  + QString(__TIME__ ));
}

AboutWindow::~AboutWindow()
{
    delete ui;
}
