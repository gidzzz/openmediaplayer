#ifndef QMAEMO5ROTATOR_H
#define QMAEMO5ROTATOR_H
//----------
// Provides a means of WORKING automatic rotation for Maemo 5 apps. (The default Qt solution is buggy.)
//----------

#include <Qt>
#include <QtGui>
#include <QtCore>

class QMaemo5Rotator : public QObject
{

    Q_OBJECT

public:

    enum RotationBehavior
    {
        AutomaticBehavior = 0,
        LandscapeBehavior = 1,
        PortraitBehavior = 2
    };

    enum Orientation
    {
        LandscapeOrientation = 0,
        PortraitOrientation = 1
    };

private:

    bool isSetUp;
    RotationBehavior _currentBehavior;
    Orientation _currentOrientation;

public:

    explicit QMaemo5Rotator(RotationBehavior behavior = LandscapeBehavior, QWidget *parent = NULL);
    ~QMaemo5Rotator();

    const RotationBehavior currentBehavior();
    const Orientation currentOrientation();
    void setCurrentBehavior(RotationBehavior value);
    void setCurrentOrientation(Orientation value);

signals:
    //void orientationChanged(Orientation orientation);
    void portrait();
    void landscape();

private slots:

    void on_orientation_changed(const QString& newOrienation);

};
#endif // QMAEMO5ROTATOR_H
