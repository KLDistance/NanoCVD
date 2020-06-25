#ifndef PSUARUIDNO_H
#define PSUARUIDNO_H

#include <QtCore>
#include <QSerialPort>

class PSUAruidno : public QThread
{
    Q_OBJECT
public:
    explicit PSUAruidno(QObject *parent);
    ~PSUAruidno();
    
    void PickComport(QString &port_name);
    int CheckValidDevice();
    void WriteIntoTarget(const QString &data);
    void check_arduino_valid();
    void run() override;
    
    // read-in processing thread terminate, suspend and resume
    void proc_terminate();
    void proc_suspend();
    void proc_resume();
private:
    QSerialPort *psuarduino;
    bool is_device_available = false;
    
    // read-in processing thread
    QMutex proc_mutex;
    QWaitCondition proc_notifier;
    bool stop = false;
    bool suspension_request = true;
    
    QStringList serialBufferList;
    QMutex serialBufMutex;
public slots:
    void ReadFromDevice();
signals:
    void ext_valid_device(bool is_valid);
};

#endif // PSUARUIDNO_H
