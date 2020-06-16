#ifndef CNCROUTER_H
#define CNCROUTER_H

#include <QtCore>
#include <QSerialPort>

class CNCRouter : public QObject
{
    Q_OBJECT
public:
    explicit CNCRouter(QObject *parent);
    void PickComport(QString &port_name);
    int CheckValidDevice();
    void WriteIntoTarget(const QString &data);
private:
    QSerialPort *cncrouter;
    bool is_device_available = false;
    
    QStringList serialBufferList;
    QMutex serialBufMutex;
public slots:
    void ReadFromTarget();
};

#endif // CNCROUTER_H
