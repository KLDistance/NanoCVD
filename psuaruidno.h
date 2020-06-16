#ifndef PSUARUIDNO_H
#define PSUARUIDNO_H

#include <QtCore>
#include <QSerialPort>

class PSUAruidno : public QObject
{
    Q_OBJECT
public:
    explicit PSUAruidno(QObject *parent);
    
    void PickComport(QString &port_name);
    int CheckValidDevice();
    void WriteIntoTarget(const QString &data);
private:
    QSerialPort *psuarduino;
    bool is_device_available = false;
    
    QStringList serialBufferList;
    QMutex serialBufMutex;
public slots:
    void ReadFromDevice();
};

#endif // PSUARUIDNO_H
