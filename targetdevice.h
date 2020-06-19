#ifndef TARGETDEVICE_H
#define TARGETDEVICE_H

#include <QtCore>
#include <QDebug>
#include <QtSerialPort>
#include <string>
#include <QMainWindow>

#include "cncrouter.h"
#include "psuaruidno.h"

class TargetDevice : public QObject
{
    Q_OBJECT
public:
    explicit TargetDevice(QObject *parent);
    ~TargetDevice();
    
    void ComportScan();
    void PickCNCRouterPort(QString &port_name);
    void PickPSUArduinoPort(QString &port_name);
    void CheckCNCRouterPortValid();
    int IsPSUArduinoPortValid();
    QVector<QString>& get_port_name_list();
private:
    QVector<quint16> vendor_id;
    QVector<quint16> product_id;
    QVector<QString> port_name;
    
    QObject *target_parent;
    CNCRouter *cncrouter;
    PSUAruidno *psuarduino;
public slots:
    // external request signal feedback
    void obtain_ext_check_cncrouter_valid(bool is_valid);
};

#endif // TARGETDEVICE_H
