#ifndef SHARE_H
#define SHARE_H

#include <QDialog>

namespace Ui {
    class Share;
}

class Share : public QDialog
{
    Q_OBJECT

public:
    explicit Share( QWidget *parent = 0, QStringList selected = QStringList() );
    ~Share();
    QStringList files;

private:
    Ui::Share *ui;

private slots:
    void on_share_mail_released();
    void on_share_bt_released();

};

#endif // SHARE_H
