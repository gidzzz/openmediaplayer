#ifndef QROTATEDLABEL_H
#define QROTATEDLABEL_H

#include <QWidget>
#include <QPainter>
#include <QTime>
#include <QTimer>
#include <QFont>

class QRotatedLabel : public QWidget
{
    Q_OBJECT
public:
    explicit QRotatedLabel(QWidget *parent = 0);
    void setText(QString);

protected:
    void paintEvent(QPaintEvent *);

signals:

public slots:

private:
    QString text;

};

#endif // QROTATEDLABEL_H
