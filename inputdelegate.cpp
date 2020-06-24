#include "inputdelegate.h"
#include "keyenterreceiver.h"
#include "mainwindow.h"

InputDelegate::InputDelegate(QObject *obj)
{
    this->obj = obj;
}

QWidget *InputDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QLineEdit *lineEdit = new QLineEdit(parent);
    // set validator
    MainWindow *mWin = (MainWindow*)this->obj;
    QDoubleValidator *validator = new QDoubleValidator(0, 100, 3, mWin);
    lineEdit->setValidator(validator);
    lineEdit->setFocus();
    return lineEdit;
}
