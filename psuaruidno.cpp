#include "psuaruidno.h"

PSUAruidno::PSUAruidno(QObject *parent) :
    QThread(parent)
{
    this->psuarduino = new QSerialPort(this);
}

PSUAruidno::~PSUAruidno()
{
    if(this->psuarduino->isOpen()) this->psuarduino->close();
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
    QObject::connect(this->psuarduino, SIGNAL(readyRead()), this, SLOT(ReadFromDevice()));
}

int PSUAruidno::CheckValidDevice()
{
    // identifier target response: "[arduino_nanocvd_8392af01]"
    const QString feedback_lut_str = "[arduino_nanocvd_8392af01]";
    // 10 iterations for checking the buffer
    for(int iter = 0; iter < 10; iter++)
    {
        // cycle for serial port response
        QThread::msleep(200);
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
    this->psuarduino->setRequestToSend(true);
    this->psuarduino->write(data.toUtf8());
    this->psuarduino->waitForBytesWritten();
    this->psuarduino->setRequestToSend(false);
}

void PSUAruidno::check_arduino_valid()
{
    // identifier host sender: "[host_nanocvd_8392af00]"
    this->WriteIntoTarget("[host_nanocvd_8392af00]");
}

void PSUAruidno::run()
{
    while(1)
    {
        this->proc_mutex.lock();
        if(this->suspension_request) this->proc_notifier.wait(&this->proc_mutex);
        if(this->stop)
        {
            this->proc_mutex.unlock();
            break;
        }
        this->proc_mutex.unlock();
        
    }
}

void PSUAruidno::proc_terminate()
{
    this->proc_mutex.lock();
    this->stop = true;
    this->proc_notifier.wakeAll();
    this->proc_mutex.unlock();
    if(this->psuarduino->isOpen()) this->psuarduino->close();
}

void PSUAruidno::proc_suspend()
{
    this->proc_mutex.lock();
    this->suspension_request = true;
    this->proc_mutex.unlock();
}

void PSUAruidno::proc_resume()
{
    this->proc_mutex.lock();
    this->suspension_request = false;
    this->proc_notifier.wakeAll();
    this->proc_mutex.unlock();
}

void PSUAruidno::ReadFromDevice()
{
    QString msg = "";
    while(this->psuarduino->canReadLine())
    {
        QString tmp = QString::fromUtf8(this->psuarduino->readLine());
        msg += tmp;
    }
    this->serialBufMutex.lock();
    this->serialBufferList.append(msg);
    this->serialBufferList.removeAll("");
    this->serialBufMutex.unlock();
}
