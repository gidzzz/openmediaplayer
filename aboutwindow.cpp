#include "aboutwindow.h"
#include "ui_aboutwindow.h"

AboutWindow::AboutWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::AboutWindow)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5StackedWindow);
#endif
    ui->iconLabel->setPixmap(QIcon::fromTheme("tasklaunch_media_player").pixmap(164, 164));
    QFont f = this->font();
    f.setPointSize(18);
    ui->label->setFont(f);

    ui->buildLabel->setText("Build Date: " + QString(__DATE__ ) + " "  + QString(__TIME__ ));
}

AboutWindow::~AboutWindow()
{
    delete ui;
}
