#ifndef TARGETDEVICE_H
#define TARGETDEVICE_H

#include <QtCore>
#include <QDebug>
#include <QtSerialPort>
#include <string>

class TargetDevice : public QObject
{
    Q_OBJECT
public:
    explicit TargetDevice(QObject *parent);
};

#endif // TARGETDEVICE_H
