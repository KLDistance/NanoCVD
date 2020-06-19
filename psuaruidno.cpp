#include "psuaruidno.h"

PSUAruidno::PSUAruidno(QObject *parent) :
    QObject(parent)
{
    this->psuarduino = new QSerialPort(this);
}

void PSUAruidno::PickComport(QString &port_name)
{
    qDebug() << "PSUArduino picked " << port_name;
    this->psuarduino->setPortName(port_name);
    this->psuarduino->open(QSerialPort::ReadWrite);
    this->psuarduino->setBaudRate(QSerialPort::Baud9600);
    this->psuarduino->setDataBits(QSerialPort::Data8);
    this->psuarduino->setFlowControl(QSerialPort::SoftwareControl);
    this->psuarduino->setParity(QSerialPort::NoParity);
    this->psuarduino->setStopBits(QSerialPort::OneStop);
}

int PSUAruidno::CheckValidDevice()
{
    // identifier host sender: "[host_nanocvd_8392af00]"
    this->WriteIntoTarget("[host_nanocvd_8392af00]");
    // identifier target response: "[arduino_nanocvd_8392af01]"
    const QString feedback_lut_str = "[arduino_nanocvd_8392af01]";
    // 10 iterations for checking the buffer
    for(int iter = 0; iter < 10; iter++)
    {
        // cycle for serial port response
        QThread::msleep(100);
        this->serialBufMutex.lock();
        for(int jter = 0; jter < this->serialBufferList.size(); jter++)
        {
            if(this->serialBufferList[jter].length() == feedback_lut_str.length() && 
                    this->serialBufferList[jter] == feedback_lut_str)
            {
                this->serialBufferList.clear();
                this->serialBufMutex.unlock();
                // obtain valid response, return 1
                return 1;
            }
        }
        this->serialBufMutex.unlock();
    }
    // if host fails to get valid response, return 0
    return 0;
}

void PSUAruidno::WriteIntoTarget(const QString &data)
{
    this->psuarduino->write(data.toUtf8());
}

void PSUAruidno::ReadFromDevice()
{
    char buf[4096];
    this->psuarduino->readLine(buf, sizeof(buf));
    this->serialBufMutex.lock();
    this->serialBufferList.append(QString(buf));
    this->serialBufMutex.unlock();
}
