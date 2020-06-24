#ifndef INPUTDELEGATE_H
#define INPUTDELEGATE_H

#include <QtCore>
#include <QMainWindow>
#include <QLineEdit>
#include <QItemDelegate>

class InputDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    InputDelegate(QObject *obj);
    QWidget * createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
private:
    QObject *obj;
};

#endif // INPUTDELEGATE_H
