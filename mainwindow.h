#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QBoxLayout>
#include <QMessageBox>
#include <QStandardItem>

#include "qledindicator.h"
#include "targetdevice.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    
    // stages table contents
    QStandardItemModel *std_table_model;
    // virtual led indicators
    QList<QLedIndicator*> serialport_leds;
    QList<QLedIndicator*> positioning_leds;
    
    // target device
    TargetDevice *target_device;
    
    // led initialization
    void led_init();
    void obtain_comport_list();
    // set labels and ledits for positioning and motion states
    void set_position_feedback_labels(double x, double y, double z);
    void set_position_state_labels(QString state);
private slots:
    // cnc router comport connection
    void on_btn_cncconnect_clicked();
    
private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
