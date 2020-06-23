#ifndef KEYENTERRECEIVER_H
#define KEYENTERRECEIVER_H

#include <QtCore>
#include <QKeyEvent>

class KeyEnterReceiver : public QObject
{
    Q_OBJECT
public:
    KeyEnterReceiver(QObject *obj);
protected:
    bool eventFilter(QObject *obj, QEvent *event);
private:
    QObject *input_obj;
};

#endif // KEYENTERRECEIVER_H
