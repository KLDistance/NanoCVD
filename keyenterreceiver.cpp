#include "keyenterreceiver.h"
#include "mainwindow.h"

KeyEnterReceiver::KeyEnterReceiver(QObject *obj)
{
    this->input_obj = obj;
}

bool KeyEnterReceiver::eventFilter(QObject *obj, QEvent *event)
{
    if(event->type() == QEvent::KeyPress)
    {
        QKeyEvent *key = static_cast<QKeyEvent*>(event);
        if((key->key() == Qt::Key_Enter) || (key->key() == Qt::Key_Return) || (key->key() == Qt::Key_Escape))
        {
            MainWindow *mWin = (MainWindow*)this->input_obj;
            mWin->get_central_widget()->setFocus();
        }
        else
        {
            return QObject::eventFilter(obj, event);
        }
        return true;
    }
    else
    {
        return QObject::eventFilter(obj, event);
    }
    return false;
}
