#include <QMessageBox>
#include "targetdevice.h"
#include "mainwindow.h"

TargetDevice::TargetDevice(QObject *parent)
    : QObject(parent)
{
    this->cncrouter = new CNCRouter(this);
    this->psuarduino = new PSUAruidno(this);
    this->target_parent = parent;
    QObject::connect(this->cncrouter, SIGNAL(ext_valid_device(bool)), this, SLOT(obtain_ext_check_cncrouter_valid(bool)));
    
    // initiate thread
    this->cncrouter->start();
}

TargetDevice::~TargetDevice()
{
    this->cncrouter->proc_terminate();
    this->cncrouter->wait();
}

void TargetDevice::ComportScan()
{
    // clear the serial port identifier list
    if(!this->vendor_id.isEmpty()) this->vendor_id.clear();
    if(!this->product_id.isEmpty()) this->product_id.clear();
    if(!this->port_name.isEmpty()) this->port_name.clear();
    // iterative scanning over all serial ports
    foreach(const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts())
    {
        this->vendor_id.push_back(serialPortInfo.vendorIdentifier());
        this->product_id.push_back(serialPortInfo.productIdentifier());
        this->port_name.push_back(serialPortInfo.portName());
    }
}

void TargetDevice::PickCNCRouterPort(QString &port_name)
{
    this->cncrouter->PickComport(port_name);
}

void TargetDevice::PickPSUArduinoPort(QString &port_name)
{
    this->psuarduino->PickComport(port_name);
}

void TargetDevice::CheckCNCRouterPortValid()
{
    this->cncrouter->check_cncrouter_valid();
}

void TargetDevice::move_cncrouter(bool consecutive_mode, double x, double y, double z, double speed)
{
    if(consecutive_mode)
    {
        // amplify the step to nearly infinitely large
        // and use small speed
        
    }
    else
    {
        this->cncrouter->relative_stepping(x, y, z, speed);
        this->cncrouter->position_query();
    }
}

void TargetDevice::halt_cncrouter()
{
    this->cncrouter->force_brake();
}

QVector<QString> &TargetDevice::get_port_name_list()
{
    return this->port_name;
}

void TargetDevice::obtain_ext_check_cncrouter_valid(bool is_valid)
{
    MainWindow *mWin = (MainWindow*)this->target_parent;
    if(is_valid)
    {
        mWin->serialport_leds[0]->setChecked(true);
    }
    else
    {
        mWin->serialport_leds[0]->setChecked(false);
        QMessageBox::critical(nullptr, "Serial Port Invalid", "Unable to connect to this remote device.", QMessageBox::Yes);
    }
}

void TargetDevice::obtain_cncrouter_position(int state, double x, double y, double z)
{
    MainWindow *mWin = (MainWindow*)this->target_parent;
    mWin->set_position_feedback_labels(x, y, z);
    if(state) mWin->set_position_state_labels("Running");
    else mWin->set_position_state_labels("Idle");
}
