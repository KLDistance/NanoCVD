#ifndef TARGETDEVICE_H
#define TARGETDEVICE_H

#include <QtCore>
#include <QDebug>
#include <QtSerialPort>
#include <string>

#include "cncrouter.h"
#include "psuaruidno.h"

class TargetDevice : public QObject
{
    Q_OBJECT
public:
    explicit TargetDevice(QObject *parent);
    
    void ComportScan();
private:
    QVector<quint16> vendor_id;
    QVector<quint16> product_id;
    QVector<QString> port_name;
};

#endif // TARGETDEVICE_H
