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
    // check if cnc router comport is valid
    void CheckCNCRouterPortValid();
    // move cnc router
    void move_cncrouter(bool consecutive_mode, double x, double y, double z, double speed);
    // check cncrouter position
    void position_query();
    // halt cnc router
    void halt_cncrouter();
    
    // check if arduino comport is valid
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
    void obtain_ext_check_psuarduino_valid(bool is_valid);
    // cncrouter position information feedback
    void obtain_cncrouter_position(int state, double x, double y, double z);
};

#endif // TARGETDEVICE_H
