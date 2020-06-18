#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QBoxLayout>
#include <QStandardItem>

#include "qledindicator.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    
private:
    Ui::MainWindow *ui;
    // stages table contents
    QStandardItemModel *std_table_model;
    // virtual led indicators
    QList<QLedIndicator*> serialport_leds;
    QList<QLedIndicator*> positioning_leds;
    
    // led initialization
    void led_init();
};
#endif // MAINWINDOW_H
