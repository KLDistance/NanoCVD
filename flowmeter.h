#ifndef FLOWMETER_H
#define FLOWMETER_H

#include <QtCore>
#include <QSerialPort>

class Flowmeter : public QThread
{
    Q_OBJECT
public:
    explicit Flowmeter(QObject *parent);
    ~Flowmeter();
    QSerialPort *flowmeter;
    
    void PickComport(QString &port_name);
    void FlowQueryBegin();
    void FlowQuerySuspend();
    // read-in thread processing thread terminate, suspend and resume
    void proc_terminate();
    void proc_suspend();
    void proc_resume();
    
    void WriteIntoTarget(const QString &data);
    
    void run() override;
    
    // read-in processing thread
    QMutex proc_mutex;
    QWaitCondition proc_notifier;
    bool stop = false;
    bool suspension_request = true;
    
    // serial buffer list
    QString serial_buffer_str;
    QStringList serial_buffer_list;
    QStringList serial_buffer_src;
    QMutex serial_buf_mutex;
    
    // thread timer
    QTimer *thread_timer = nullptr;
public slots:
    void ReadFromTarget();
signals:
    void flowmeter_thread_run_signal();
};

#endif // FLOWMETER_H
