#include <QMessageBox>
#include "targetdevice.h"
#include "mainwindow.h"

TargetDevice::TargetDevice(QObject *parent)
    : QThread(parent)
{
    this->cncrouter = new CNCRouter(this);
    this->psuarduino = new PSUAruidno(this);
    this->target_parent = parent;
    QObject::connect(this->cncrouter, SIGNAL(ext_valid_device(bool)), this, SLOT(obtain_ext_check_cncrouter_valid(bool)));
    QObject::connect(this->cncrouter, SIGNAL(PositionUpdated(int, double, double, double)), 
                     this, SLOT(obtain_cncrouter_position(int, double, double, double)));
    QObject::connect(this->psuarduino, SIGNAL(ext_valid_device(bool)), this, SLOT(obtain_ext_check_psuarduino_valid(bool)));
    // initiate thread
    this->cncrouter->start();
    this->psuarduino->start();
}

TargetDevice::~TargetDevice()
{
    this->cncrouter->proc_terminate();
    this->cncrouter->wait();
    this->psuarduino->proc_terminate();
    this->psuarduino->wait();
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

void TargetDevice::CheckPSUArduinoPortValid()
{
    this->psuarduino->check_arduino_valid();
}

void TargetDevice::move_cncrouter(bool consecutive_mode, double x, double y, double z, double speed)
{
    if(consecutive_mode)
    {
        // amplify the step to nearly infinitely large
        // and use small speed (deprecated)
        this->cncrouter->relative_stepping(x, y, z, speed);
    }
    else
    {
        this->cncrouter->relative_stepping(x, y, z, speed);
    }
}

void TargetDevice::position_query()
{
    this->cncrouter->position_query();
}

void TargetDevice::halt_cncrouter()
{
    this->cncrouter->force_brake();
}

void TargetDevice::run()
{
    MainWindow *mWin = (MainWindow*)this->target_parent;
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
        // prepare running stages
        mWin->set_routine_running_state(1);
        if(this->routine_wait.size() > 0) this->routine_wait.clear();
        if(this->routine_heat.size() > 0) this->routine_heat.clear();
        if(this->routine_velocity.size() > 0) this->routine_velocity.clear();
        if(this->routine_displacement.size() > 0) this->routine_displacement.clear();
        int stage_num = mWin->obtain_table_contains(this->routine_heat, this->routine_heat, 
                                    this->routine_velocity, this->routine_displacement);
        this->routine_wait.append(0);
        this->routine_heat.append(0);
        this->routine_velocity.append(0);
        this->routine_displacement.append(0);
        // prequiescence, iter in 100 msec
        int m_wait = (int)(mWin->obtain_prequiescent_data() * 1000);
        int m_100 = m_wait / 100;
        for(int iter = 0; iter < m_100; iter++)
        {
            this->proc_mutex.lock();
            if(this->suspension_request)
            {
                this->proc_mutex.unlock();
                goto EXIT_PROC;
            }
            this->proc_mutex.unlock();
            QThread::msleep(100);
        }
        QThread::msleep(m_wait - m_100 * 100);
        // start running stages
        for(int iter = 0; iter < stage_num + 1; iter++)
        {
            // wait
            m_wait = (int)(this->routine_wait[iter] * 1000);
            m_100 = m_wait / 100;
            for(int jter = 0; jter < m_100; jter++)
            {
                this->proc_mutex.lock();
                if(this->suspension_request)
                {
                    this->proc_mutex.unlock();
                    goto EXIT_PROC;
                }
                this->proc_mutex.unlock();
                QThread::msleep(100);
            }
            // heat up
            this->psuarduino->write_volt_value(this->routine_heat[iter]);
            // move
            this->cncrouter->relative_stepping(this->routine_displacement[iter], 0, 0, this->routine_velocity[iter]);
        }
        // break using goto
        EXIT_PROC: QThread::msleep(100);
    }
}

void TargetDevice::proc_terminate()
{
    this->proc_mutex.lock();
    this->stop = true;
    this->proc_notifier.wakeAll();
    this->proc_mutex.unlock();
}

void TargetDevice::proc_suspend()
{
    this->proc_mutex.lock();
    this->suspension_request = true;
    this->proc_mutex.unlock();
}

void TargetDevice::proc_resume()
{
    this->proc_mutex.lock();
    this->suspension_request = false;
    this->proc_notifier.wakeAll();
    this->proc_mutex.unlock();
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
        QMessageBox::critical(nullptr, "Serial Port Invalid", "Unable to connect to CNC Router!", QMessageBox::Yes);
    }
}

void TargetDevice::obtain_ext_check_psuarduino_valid(bool is_valid)
{
    MainWindow *mWin = (MainWindow*)this->target_parent;
    if(is_valid)
    {
        mWin->serialport_leds[1]->setChecked(true);
    }
    else
    {
        mWin->serialport_leds[1]->setChecked(false);
        QMessageBox::critical(nullptr, "Serial Port Invalid", "Unable to connect to Arduino!", QMessageBox::Yes);
    }
}

void TargetDevice::obtain_cncrouter_position(int state, double x, double y, double z)
{
    MainWindow *mWin = (MainWindow*)this->target_parent;
    mWin->set_position_feedback_labels(x, y, z);
    if(state) mWin->set_position_state_labels("Running");
    else mWin->set_position_state_labels("Idle");
}
