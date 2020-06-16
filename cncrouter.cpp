#include "cncrouter.h"

CNCRouter::CNCRouter(QObject *parent) :
    QObject(parent)
{
    this->cncrouter = nullptr;
}

void CNCRouter::PickComport(QString &port_name)
{
    qDebug() << "CNCRouter picked " << port_name;
    this->cncrouter->setPortName(port_name);
    this->cncrouter->open(QSerialPort::ReadWrite);
    this->cncrouter->setBaudRate(QSerialPort::Baud115200);
    this->cncrouter->setDataBits(QSerialPort::Data8);
    this->cncrouter->setFlowControl(QSerialPort::SoftwareControl);
    this->cncrouter->setParity(QSerialPort::NoParity);
    this->cncrouter->setStopBits(QSerialPort::OneStop);
}

int CNCRouter::CheckValidDevice()
{
    // identifier target response: feedback_lut chars
    const char feedback_lut[28] = {
        0x0d, 0x0a, 0x47, 0x72, 0x62, 0x6c, 0x20,
        0x31, 0x2e, 0x31, 0x66, 0x20, 0x5b, 0x27,
        0x24, 0x27, 0x20, 0x66, 0x6f, 0x72, 0x20,
        0x68, 0x65, 0x6c, 0x70, 0x5d, 0x0d, 0x0a
    };
    const QString feedback_lut_str = QString(feedback_lut);
    // 10 iterations for checking the buffer
    for(int iter = 0; iter < 10; iter++)
    {
        // cycle for serial port response
        QThread::msleep(100);
        this->serialBufMutex.lock();
        for(int jter = 0; jter < this->serialBufferList.size(); jter++)
        {
            if(this->serialBufferList[jter].length() == 28 && this->serialBufferList[jter] == feedback_lut_str)
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

void CNCRouter::WriteIntoTarget(const QString &data)
{
    this->cncrouter->write(data.toUtf8());
}

void CNCRouter::ReadFromTarget()
{
    char buf[4096];
    this->cncrouter->readLine(buf, sizeof(buf));
    this->serialBufMutex.lock();
    this->serialBufferList.append(QString(buf));
    this->serialBufMutex.unlock();
}
