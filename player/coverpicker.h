#ifndef COVERPICKER_H
#define COVERPICKER_H

#include <QDialog>
#include <QDir>
#include <QFileInfo>
#include <QMaemo5ValueButton>
#include <QKeyEvent>

#include "includes.h"
#include "ui_coverpicker.h"

namespace Ui {
    class CoverPicker;
}

class CoverPicker : public QDialog
{
    Q_OBJECT

public:
    explicit CoverPicker(QString album, QString path, QWidget *parent = 0);
    ~CoverPicker();
    QDir dir;
    QString album;
    QString cover;

public slots:
    void browse(QString path = "..");

private:
    Ui::CoverPicker *ui;
    void keyPressEvent(QKeyEvent *e);

private slots:
    void onItemActivated(QListWidgetItem* item);
};

#endif // COVERPICKER_H
