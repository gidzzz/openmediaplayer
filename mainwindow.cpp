#include "mainwindow.h"
#include "ui_mainwindow.h"


#define musicIcon "/usr/share/icons/hicolor/164x164/hildon/mediaplayer_main_button_music.png"
#define videosIcon "/usr/share/icons/hicolor/164x164/hildon/mediaplayer_main_button_video.png"
#define radioIcon "/usr/share/icons/hicolor/164x164/hildon/mediaplayer_main_button_radio.png"
#define shuffleIcon "/usr/share/icons/hicolor/164x164/hildon/mediaplayer_main_button_shuffle.png"
#define backgroundImage "/etc/hildon/theme/mediaplayer/background.png"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setButtonIcons();
    this->setLabelText();
    this->connectSignals();
    ui->listWidget->hide();
#ifdef Q_WS_MAEMO_5
    setAttribute(Qt::WA_Maemo5StackedWindow);
    setAttribute(Qt::WA_Maemo5AutoOrientation);
#endif
    myMusicWindow = new MusicWindow(this);
    myVideosWindow = new VideosWindow(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.drawImage(this->rect(), QImage(backgroundImage));
}

void MainWindow::showMusicWindow()
{
    myMusicWindow->show();
}

void MainWindow::showVideosWindow()
{
    myVideosWindow->show();
}

void MainWindow::setButtonIcons()
{
    ui->songsButton->setIcon(QIcon(musicIcon));
    ui->videosButton->setIcon(QIcon(videosIcon));
    ui->radioButton->setIcon(QIcon(radioIcon));
    ui->shuffleAllButton->setIcon(QIcon(shuffleIcon));
}

void MainWindow::setLabelText()
{
    ui->songsButtonLabel->setText(tr("Music"));
    ui->videosButtonLabel->setText(tr("Videos"));
    ui->radioButtonLabel->setText(tr("Internet Radio"));
    ui->shuffleLabel->setText(tr("Shuffle all songs"));
}

void MainWindow::connectSignals()
{
    connect(ui->songsButton, SIGNAL(clicked()), this, SLOT(showMusicWindow()));
    connect(ui->videosButton, SIGNAL(clicked()), this, SLOT(showVideosWindow()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(showAbout()));
    connect(ui->listWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(processListClicks(QListWidgetItem*)));
#ifdef Q_WS_MAEMO_5
    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));
#endif
}

void MainWindow::orientationChanged()
{
    QRect screenGeometry = QApplication::desktop()->screenGeometry();
    if (screenGeometry.width() > screenGeometry.height()){
        // Landscape mode
        qDebug() << "MainWindow: Orientation changed: Landscape.";
        if(!ui->listWidget->isHidden()) {
            ui->listWidget->hide();
        ui->songsButton->show();
        ui->songsButtonLabel->show();
        ui->videosButton->show();
        ui->videosButtonLabel->show();
        ui->radioButton->show();
        ui->radioButtonLabel->show();
        ui->shuffleAllButton->show();
        ui->shuffleLabel->show();
        }
    } else {
        // Portrait mode
        qDebug() << "MainWindow: Orientation changed: Portrait.";
        ui->songsButton->hide();
        ui->songsButtonLabel->hide();
        ui->videosButton->hide();
        ui->videosButtonLabel->hide();
        ui->radioButton->hide();
        ui->radioButtonLabel->hide();
        ui->shuffleAllButton->hide();
        ui->shuffleLabel->hide();
        ui->listWidget->setGeometry(QRect(0, 0, 480, 800));
        if(ui->listWidget->isHidden())
            ui->listWidget->show();

        //QMainWindow::setCentralWidget(ui->listWidget);
    }
}

void MainWindow::showAbout()
{
    QMessageBox::information(this, tr("About"),
                             "A stock media player rewrite in Qt\nCopyright 2010-2011 <whoever's working on it>\n\nLicensed under GPLv3");
}

void MainWindow::processListClicks(QListWidgetItem* item)
{
    QString itemName = item->statusTip();
    if(itemName == "songs_button")
        this->showMusicWindow();
    else if(itemName == "videos_button")
        this->showVideosWindow();
}
