#include "targetdevice.h"
#include "mainwindow.h"

TargetDevice::TargetDevice(QObject *parent)
    : QThread(parent)
{
    this->target_parent = parent;
    this->cncrouter = new CNCRouter(this);
    this->psuarduino = new PSUAruidno(this);
    this->flowmeter = new Flowmeter(this);
    // init messagebox
    this->msgbox = new QMessageBox(nullptr);
    this->msgbox->setWindowTitle("Serial Port Invalid");
    this->msgbox->setModal(false);
    this->msgbox->hide();
    
    QObject::connect(this->cncrouter, SIGNAL(ext_valid_device(bool)), this, SLOT(obtain_ext_check_cncrouter_valid(bool)));
    QObject::connect(this->cncrouter, SIGNAL(PositionUpdated(int, double, double, double)), 
                     this, SLOT(obtain_cncrouter_position(int, double, double, double)), Qt::DirectConnection);
    QObject::connect(this->psuarduino, SIGNAL(ext_valid_device(bool)), this, SLOT(obtain_ext_check_psuarduino_valid(bool)));
    QObject::connect(this->cncrouter, SIGNAL(cncrouter_thread_run_signal()), this, SLOT(run_signal_from_cncrouter()), Qt::DirectConnection);
    QObject::connect(this->psuarduino, SIGNAL(psuarduino_thread_run_signal()), this, SLOT(run_signal_from_arduino()), Qt::DirectConnection);
    QObject::connect(this->flowmeter, SIGNAL(flowmeter_thread_run_signal()), this, SLOT(run_signal_from_flowmeter()), Qt::DirectConnection);
}

TargetDevice::~TargetDevice()
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

void TargetDevice::PickCNCRouterPort(QString &port_name)
{
    this->cncrouter->PickComport(port_name);
}

void TargetDevice::PickPSUArduinoPort(QString &port_name)
{
    this->psuarduino->PickComport(port_name);
}

void TargetDevice::PickFlowmeterPort(QString &port_name)
{
    this->flowmeter->PickComport(port_name);
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
    this->thread_timer = new QTimer(0);
    this->thread_timer->moveToThread(this);
    this->thread_timer->start();
    this->cncrouter->start();
    this->psuarduino->start();
    emit this->thread_run_signal();
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
        //QMessageBox::critical(nullptr, "Serial Port Invalid", "Unable to connect to CNC Router!", QMessageBox::Yes);
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
        //QMessageBox::critical(nullptr, "Serial Port Invalid", "Unable to connect to Arduino!", QMessageBox::Yes);
    }
}

void TargetDevice::obtain_cncrouter_position(int state, double x, double y, double z)
{
    MainWindow *mWin = (MainWindow*)this->target_parent;
    mWin->set_position_feedback_labels(x, y, z);
    if(state) mWin->set_position_state_labels("Running");
    else mWin->set_position_state_labels("Idle");
}

void TargetDevice::run_signal_from_arduino()
{
    while(1)
    {
        this->psuarduino->proc_mutex.lock();
        if(this->psuarduino->suspension_request) this->psuarduino->proc_notifier.wait(&this->psuarduino->proc_mutex);
        if(this->psuarduino->stop)
        {
            this->psuarduino->proc_mutex.unlock();
            break;
        }
        this->psuarduino->proc_mutex.unlock();
        this->psuarduino->ext_mutex.lock();
        switch(this->psuarduino->ext_request)
        {
        case 1:
        {
            this->psuarduino->ext_request = 0;
            this->psuarduino->ext_mutex.unlock();
            bool valid_state = this->psuarduino->CheckValidDevice();
            if(!valid_state) emit this->psuarduino_main_thread_msgbox();
            emit this->psuarduino->ext_valid_device(valid_state);
            break;
        }
        case 2:
        {
            this->psuarduino->ext_request = 0;
            this->psuarduino->ext_mutex.unlock();
            break;
        }
        }
        this->psuarduino->proc_suspend();
    }
}

void TargetDevice::run_signal_from_cncrouter()
{
    while(1)
    {
        this->cncrouter->proc_mutex.lock();
        if(this->cncrouter->suspension_request) this->cncrouter->proc_notifier.wait(&this->cncrouter->proc_mutex);
        if(this->cncrouter->stop)
        {
            this->cncrouter->proc_mutex.unlock();
            break;
        }
        this->cncrouter->proc_mutex.unlock();
        // external requests
        this->cncrouter->ext_mutex.lock();
        switch(this->cncrouter->request)
        {
        // cncrouter serial port valid check
        case 1:
        {
            this->cncrouter->request = 0;
            this->cncrouter->ext_mutex.unlock();
            this->cncrouter->proc_suspend();
            bool valid_state = this->cncrouter->CheckValidDevice();
            if(!valid_state) emit this->cncrouter_main_thread_msgbox();
            emit this->cncrouter->ext_valid_device(valid_state);
            break;
        }
        // position handler
        case 2:
        {
            this->cncrouter->ext_mutex.unlock();
            QThread::msleep(20);
            this->cncrouter->position_feedback_handler();
        }
        }
    }
}

void TargetDevice::run_signal_from_flowmeter()
{
    while(1)
    {
        this->flowmeter->proc_mutex.lock();
        if(this->flowmeter->suspension_request) this->flowmeter->proc_notifier.wait(&this->flowmeter->proc_mutex);
        if(this->flowmeter->stop)
        {
            this->flowmeter->proc_mutex.unlock();
            break;
        }
        this->flowmeter->proc_mutex.unlock();
        // query meter flow rate
        // #1
        QString flow_rate_query = QString("!0") + QString::number(1) + QString(",F\r\n");
        this->flowmeter->WriteIntoTarget(flow_rate_query);
        QThread::msleep(400);
        // #2
        flow_rate_query = QString("!0") + QString::number(2) + QString(",F\r\n");
        this->flowmeter->WriteIntoTarget(flow_rate_query);
        QThread::msleep(400);
        // #3
        flow_rate_query = QString("!0") + QString::number(3) + QString(",F\r\n");
        this->flowmeter->WriteIntoTarget(flow_rate_query);
        QThread::msleep(400);
        // obtain flow rate in ml/min for 3 meters
        double propf = 0, butaf = 0, argof = 0;
        // 01 for propane, 02 for butane, 03 for argon
        this->flowmeter->serial_buf_mutex.lock();
        if(this->flowmeter->serial_buffer_list.size() > 0)
        {
           for(int iter = 0; iter < this->flowmeter->serial_buffer_list.size(); iter++)
           {
               if(this->flowmeter->serial_buffer_src[iter].toInt() == 1)
               {
                   // propane meter
                   propf = this->flowmeter->serial_buffer_list[iter].toDouble();
               }
               if(this->flowmeter->serial_buffer_src[iter].toInt() == 2)
               {
                   // butane meter
                   butaf = this->flowmeter->serial_buffer_list[iter].toDouble();
               }
               if(this->flowmeter->serial_buffer_src[iter].toInt() == 3)
               {
                   // argon meter
                   argof = this->flowmeter->serial_buffer_list[iter].toDouble();
               }
           }
           // signaling the UI thread
           qDebug() << propf << "," << butaf << "," << argof;
           emit this->flowrate_indicator_update(propf, butaf, argof);
        }
        this->flowmeter->serial_buf_mutex.unlock();
    }
}
