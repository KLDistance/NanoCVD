#include "targetdevice.h"

TargetDevice::TargetDevice(QObject *parent)
    : QObject(parent)
{
    
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
