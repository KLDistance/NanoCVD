#ifndef PSUARUIDNO_H
#define PSUARUIDNO_H

#include <QtCore>
#include <QSerialPort>
#include <QTextStream>
#include <qfile.h>

class PSUAruidno : public QThread
{
    Q_OBJECT
public:
    explicit PSUAruidno(QObject *parent);
    ~PSUAruidno();
    
    void PickComport(QString &port_name);
    void init_comport_object();
    int CheckValidDevice();
    void WriteIntoTarget(const QString &data);
    void write_volt_value();
    void check_arduino_valid();
    void run() override;
    
    // read-in processing thread terminate, suspend and resume
    void proc_terminate();
    void proc_suspend();
    void proc_resume();
    
    // request write voltage into the target
    void ext_write_volt(double index);
    
    // ext mutex
    QMutex ext_mutex;

    QSerialPort *psuarduino = nullptr;
    bool is_device_available = false;
    
    // read-in processing thread
    QMutex proc_mutex;
    QWaitCondition proc_notifier;
    bool stop = false;
    bool suspension_request = true;
    
    // write in volt
    double write_in_volt = 0;
    // external request
    int ext_request = 0;
    
    QStringList serialBufferList;
    QMutex serialBufMutex;
    
    QVector<double> volt_output;
    QVector<double> volt_real_output;
    // thread timer
    QTimer *thread_timer = nullptr;
    // read voltage LUT from file
    void read_volt_LUT_from_file();
public slots:
    void ReadFromDevice();
signals:
    void ext_valid_device(bool is_valid);
    void psuarduino_thread_run_signal();
    void init_arduino_serial_port_trigger();
};

#endif // PSUARUIDNO_H
