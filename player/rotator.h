#ifndef ROTATOR_H
#define ROTATOR_H

#include <QObject>
#include <QWidget>
#include <QApplication>
#include <QDesktopWidget>

class Rotator : public QObject
{
    Q_OBJECT

public:
    enum Orientation {
        Automatic,
        Landscape,
        Portrait
    };

    static Rotator* acquire();

    void setPolicy(Orientation policy);
    void setSlave(QWidget *subject);

    Orientation policy();
    int width();
    int height();

signals:
    void rotated(int width, int height);

private slots:
    void onResized();

private:
    Rotator();

    static Rotator *instance;

    QWidget *m_slave;
    Orientation m_policy;
    int w;
    int h;
};

#endif // ROTATOR_H
