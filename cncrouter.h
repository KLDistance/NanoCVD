#ifndef CNCROUTER_H
#define CNCROUTER_H

#include <QtCore>
#include <QSerialPort>

class CNCRouter : public QThread
{
    Q_OBJECT
public:
    explicit CNCRouter(QObject *parent);
    ~CNCRouter();
    void PickComport(QString &port_name);
    int CheckValidDevice();
    void WriteIntoTarget(const QString &data);
    
    // thread processing on serial ports readins
    void run() override; 
    // read-in processing thread terminate, suspend and resume
    void proc_terminate();
    void proc_suspend();
    void proc_resume();
    
    // write stepping data
    void relative_stepping(double xstep, double ystep, double zstep, double speed);
    // query position
    void position_query();
    // force brake
    void force_brake();
    
    // external requests
    // check cncrouter serial port validity
    void check_cncrouter_valid();
private:
    QSerialPort *cncrouter;
    bool is_device_available = false;
    
    // serial buffer list
    QStringList serialBufferList;
    QMutex serialBufMutex;
    
    // read-in processing thread
    QMutex proc_mutex;
    QWaitCondition proc_notifier;
    bool stop = false;
    bool suspension_request = true;
    // external request
    int request = 0;
    QMutex ext_mutex;
    
    // initial boot response recognition
    bool is_boot_response(QString &str);
    // position feedback recognition
    bool is_position_info(QString &str);
    // separate working position from string
    void extract_position_info(QString &str);
    
public slots:
    void ReadFromTarget();
signals:
    // state: 0 -- idle, 1 -- running
    void PositionUpdated(int state, double x, double y, double z);
    // external request signals
    void ext_valid_device(bool is_valid);
};

#endif // CNCROUTER_H
