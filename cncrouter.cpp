#include "cncrouter.h"

// identifier target response: feedback_lut chars
const char feedback_lut[29] = {
    0x0d, 0x0a, 0x47, 0x72, 0x62, 0x6c, 0x20,
    0x31, 0x2e, 0x31, 0x66, 0x20, 0x5b, 0x27,
    0x24, 0x27, 0x20, 0x66, 0x6f, 0x72, 0x20,
    0x68, 0x65, 0x6c, 0x70, 0x5d, 0x0d, 0x0a, 0x00
};

CNCRouter::CNCRouter(QObject *parent) :
    QThread(parent)
{
    this->cncrouter = new QSerialPort(this);
}

CNCRouter::~CNCRouter()
{
    if(this->cncrouter->isOpen()) this->cncrouter->close();
}

void CNCRouter::PickComport(QString &port_name)
{
    if(this->cncrouter->isOpen()) this->cncrouter->close();
    qDebug() << "CNCRouter picked " << port_name;
    this->cncrouter->setPortName(port_name);
    this->cncrouter->open(QSerialPort::ReadWrite);
    this->cncrouter->setBaudRate(QSerialPort::Baud115200);
    this->cncrouter->setDataBits(QSerialPort::Data8);
    this->cncrouter->setFlowControl(QSerialPort::SoftwareControl);
    this->cncrouter->setParity(QSerialPort::NoParity);
    this->cncrouter->setStopBits(QSerialPort::OneStop);
    QObject::connect(this->cncrouter, SIGNAL(readyRead()), this, SLOT(ReadFromTarget()));
}

int CNCRouter::CheckValidDevice()
{
    const QString feedback_lut_str = QString(feedback_lut);
    // 25 iterations for checking the buffer (5s timeout)
    for(int iter = 0; iter < 30; iter++)
    {
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
        // cycle for serial port response
        QThread::msleep(100);
    }
    // if host fails to get valid response, return 0
    return 0;
}

void CNCRouter::WriteIntoTarget(const QString &data)
{
    this->cncrouter->setRequestToSend(true);
    this->cncrouter->write(data.toUtf8());
    this->cncrouter->waitForBytesWritten();
    this->cncrouter->setRequestToSend(false);
}

void CNCRouter::run()
{
    this->thread_timer = new QTimer(0);
    this->thread_timer->moveToThread(this);
    this->thread_timer->start();
    emit this->cncrouter_thread_run_signal();
}

void CNCRouter::proc_terminate()
{
    this->proc_mutex.lock();
    this->stop = true;
    this->proc_notifier.wakeAll();
    this->proc_mutex.unlock();
    if(this->cncrouter->isOpen()) this->cncrouter->close();
}

void CNCRouter::proc_suspend()
{
    this->proc_mutex.lock();
    this->suspension_request = true;
    this->proc_mutex.unlock();
}

void CNCRouter::proc_resume()
{
    this->proc_mutex.lock();
    this->suspension_request = false;
    this->proc_notifier.wakeAll();
    this->proc_mutex.unlock();
}

void CNCRouter::relative_stepping(double xstep, double ystep, double zstep, double speed)
{
    QString serial_input = "G21G91G1X" + QString::number(xstep) + "Y" + 
            QString::number(ystep) + "Z" + QString::number(zstep) + "F" + QString::number(speed)+ "\n";
    this->WriteIntoTarget(serial_input);
}

void CNCRouter::position_query()
{
    this->ext_mutex.lock();
    this->request = 2;
    this->ext_mutex.unlock();
    this->proc_resume();
    for(int iter = 0; iter < 2; iter++)
    {
        QThread::msleep(50);
        this->WriteIntoTarget("?\n");
    }
}

void CNCRouter::force_brake()
{
    // ! \n 0x18 \n
    const char bytes[] = {0x21, 0x0d};
    const char bytes2[] = {0x18, 0x0d};
    this->cncrouter->setRequestToSend(true);
    this->cncrouter->write(bytes);
    this->cncrouter->waitForBytesWritten();
    this->cncrouter->setRequestToSend(false);
    //QThread::msleep(150);
    this->cncrouter->setRequestToSend(true);
    this->cncrouter->write(bytes2);
    this->cncrouter->waitForBytesWritten();
    this->cncrouter->setRequestToSend(false);
}

bool CNCRouter::is_boot_response(QString &str)
{
    const QString feedback_lut_str = QString(feedback_lut);
    if(str == feedback_lut_str) return 1;
    else return 0;
}

bool CNCRouter::is_position_info(QString &str)
{
    if(str.contains("MPos:")) return 1;
    else return 0;
}

void CNCRouter::extract_position_info(QString &str)
{
    qDebug() << "-----------" << str;
    // check idle state
    int state = 1;
    QString tmp_state_str = str.split("|")[0];
    if(tmp_state_str == "<Idle" || tmp_state_str == "<Run") 
    {
        this->ext_mutex.lock();
        this->request = 0;
        this->ext_mutex.unlock();
        this->proc_suspend();
        state = 0;
        // check positions
        double x_pos = 0;
        double y_pos = 0;
        double z_pos = 0;
        int match_index = str.indexOf("MPos:", 0, Qt::CaseInsensitive);
        QString substr = QString::fromStdString(&((str.toStdString())[match_index + 5])).split("|")[0];
        QStringList position_str_list = substr.split(",");
        x_pos = position_str_list[0].remove("MPos:", Qt::CaseInsensitive).toDouble();
        y_pos = position_str_list[1].toDouble();
        z_pos = position_str_list[2].toDouble();
        // send signals
        emit PositionUpdated(state, x_pos, y_pos, z_pos);
    }
}

void CNCRouter::position_feedback_handler()
{
    this->serialBufMutex.lock();
    for(int iter = this->serialBufferList.size() - 1; iter >= 0; iter--)
    {
        QString content_str = this->serialBufferList[iter];
        if(content_str.contains("MPos:", Qt::CaseInsensitive))
        {
            this->serialBufMutex.unlock();
            this->extract_position_info(content_str);
            this->serialBufMutex.lock();
            this->serialBufferList.clear();
            this->cncrouter->clear();
            break;
        }
    }
    this->serialBufMutex.unlock();
}

void CNCRouter::check_cncrouter_valid()
{
    this->ext_mutex.lock();
    this->request = 1;  // check cncrouter validity
    this->ext_mutex.unlock();
    this->proc_resume();
}

void CNCRouter::ReadFromTarget()
{
    QString msg = "";
    while(this->cncrouter->canReadLine())
    {
        QString tmp = QString::fromUtf8(this->cncrouter->readLine());
        msg += tmp;
    }
    this->serialBufMutex.lock();
    this->serialBufferList.append(msg);
    this->serialBufferList.removeAll("");
    this->serialBufMutex.unlock();
}
