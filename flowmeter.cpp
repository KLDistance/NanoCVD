#include "flowmeter.h"

Flowmeter::Flowmeter(QObject *parent) :
    QThread(parent)
{
    this->flowmeter = new QSerialPort(this);
}

Flowmeter::~Flowmeter()
{
    if(this->flowmeter->isOpen()) this->flowmeter->close();
}

void Flowmeter::PickComport(QString &port_name)
{
    if(this->flowmeter->isOpen()) this->flowmeter->close();
    qDebug() << "Flowmeter picked " << port_name;
    this->flowmeter->setPortName(port_name);
    this->flowmeter->open(QSerialPort::ReadWrite);
    this->flowmeter->setBaudRate(QSerialPort::Baud9600);
    this->flowmeter->setDataBits(QSerialPort::Data8);
    this->flowmeter->setFlowControl(QSerialPort::NoFlowControl);
    this->flowmeter->setParity(QSerialPort::NoParity);
    this->flowmeter->setStopBits(QSerialPort::OneStop);
    QObject::connect(this->flowmeter, SIGNAL(readyRead()), this, SLOT(ReadFromTarget()));
}

void Flowmeter::FlowQueryBegin()
{
    
}

void Flowmeter::FlowQuerySuspend()
{
    
}

void Flowmeter::proc_terminate()
{
    this->proc_mutex.lock();
    this->stop = true;
    this->proc_notifier.wakeAll();
    this->proc_mutex.unlock();
    if(this->flowmeter->isOpen()) this->flowmeter->close();
}

void Flowmeter::proc_suspend()
{
    this->proc_mutex.lock();
    this->suspension_request = true;
    this->proc_mutex.unlock();
}

void Flowmeter::proc_resume()
{
    this->proc_mutex.lock();
    this->suspension_request = false;
    this->proc_notifier.wakeAll();
    this->proc_mutex.unlock();
}

void Flowmeter::WriteIntoTarget(const QString &data)
{
    this->flowmeter->setRequestToSend(true);
    this->flowmeter->write(data.toUtf8());
    this->flowmeter->waitForBytesWritten();
    this->flowmeter->setRequestToSend(false);
}

void Flowmeter::run()
{
    this->thread_timer = new QTimer(0);
    this->thread_timer->moveToThread(this);
    this->thread_timer->start();
    emit this->flowmeter_thread_run_signal();
}

void Flowmeter::ReadFromTarget()
{
    while(this->flowmeter->canReadLine())
    {
        QString tmp = QString::fromUtf8(this->flowmeter->readLine());
        this->serial_buffer_str += tmp;
    }
    QStringList serial_split = this->serial_buffer_str.split("\r\n");
    for(int iter = 0; iter < serial_split.size() - 1; iter++)
    {
        // split flowmeter address and flowrate value and separate in two arraylists
        QStringList serial_section_split = serial_split[iter].split(",");
        this->serial_buf_mutex.lock();
        this->serial_buffer_src.push_back(QString(&(serial_section_split[0].toStdString())[1]));
        this->serial_buffer_list.push_back(serial_section_split[1]);
        this->serial_buf_mutex.unlock();
    }
    this->serial_buffer_str = serial_split[serial_split.size() - 1];
}
