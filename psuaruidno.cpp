#include "psuaruidno.h"

PSUAruidno::PSUAruidno(QObject *parent) :
    QThread(parent)
{
    this->psuarduino = new QSerialPort(this);
    this->read_volt_LUT_from_file();
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
    const QString feedback_lut_str = "arduino_nanocvd_8392af01";
    // 10 iterations for checking the buffer
    for(int iter = 0; iter < 25; iter++)
    {
        // cycle for serial port response
        this->serialBufMutex.lock();
        for(int jter = 0; jter < this->serialBufferList.size(); jter++)
        {
            if(this->serialBufferList[jter].contains(feedback_lut_str))
            {
                this->serialBufferList.clear();
                this->serialBufMutex.unlock();
                // obtain valid response, return 1
                return 1;
            }
        }
        this->serialBufMutex.unlock();
        QThread::msleep(200);
    }
    // if host fails to get valid response, return 0
    return 0;
}

void PSUAruidno::WriteIntoTarget(const QString &data)
{
    this->psuarduino->write(data.toUtf8());
}

void PSUAruidno::write_volt_value(double target_output)
{
    
}

void PSUAruidno::check_arduino_valid()
{
    // identifier host sender: "[host_nanocvd_8392af00]"
    this->WriteIntoTarget("[host_nanocvd_8392af00]\n");
    this->proc_resume();
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
        this->proc_suspend();
        emit this->ext_valid_device(this->CheckValidDevice());
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

void PSUAruidno::read_volt_LUT_from_file()
{
    QStringList word_list;
    QFile f("./dependencies/volt_lut.csv");
    if(f.open(QIODevice::ReadOnly))
    {
        QString data;
        data = f.readAll();
        word_list = data.split("\n");
        word_list.removeAll("");
        f.close();
    }
    for(int iter = 0; iter < word_list.size(); iter++)
    {
        QStringList tmp_list = word_list[iter].split(",");
        this->volt_output.append(tmp_list[0].toDouble());
        this->volt_real_output.append((tmp_list[1].toDouble()));
    }
}

void PSUAruidno::ReadFromDevice()
{
    QString msg = "";
    while(this->psuarduino->canReadLine())
    {
        QString tmp = QString::fromUtf8(this->psuarduino->readLine());
        msg += tmp;
    }
    qDebug() << msg;
    this->serialBufMutex.lock();
    this->serialBufferList.append(msg);
    this->serialBufferList.removeAll("");
    this->serialBufMutex.unlock();
}
