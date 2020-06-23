#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QBoxLayout>
#include <QMessageBox>
#include <QStandardItem>
#include <QKeyEvent>

#include "qledindicator.h"
#include "targetdevice.h"
#include "keyenterreceiver.h"

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
    // table initialization
    void table_init();
    // install focus policy
    void widgets_focus_setting();
    void obtain_comport_list();
    // set labels and ledits for positioning and motion states
    void set_position_feedback_labels(double x, double y, double z);
    void set_position_state_labels(QString state);
    // move cnc router
    void move_x(double direction, double speed);
    void move_y(double direction, double speed);
    void move_z(double direction, double speed);
    // get central widgets
    QWidget* get_central_widget();
private slots:
    // cnc router comport connection
    void on_btn_cncconnect_clicked();
    
    void on_btn_up_clicked();
    
    void on_chb_consecutiveenable_clicked(bool checked);
    
    void on_btn_appenditem_clicked();
    
    void on_btn_removeitem_clicked();
    
private:
    Ui::MainWindow *ui;
    
    // motion magnitude switch
    int mms = 0;
    // constant stepping speed
    double step_speed = 50.0;
    // keypress and release capture
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    // change stepping/vel chb labels
    void change_chb_labels(bool is_consecutive_mode);
};
#endif // MAINWINDOW_H
