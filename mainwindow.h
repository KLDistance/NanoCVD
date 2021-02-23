#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QBoxLayout>
#include <QMessageBox>
#include <QStandardItem>
#include <QKeyEvent>
#include <QFileDialog>

#include "qledindicator.h"
#include "targetdevice.h"
#include "keyenterreceiver.h"
#include "inputdelegate.h"

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
    
    // process is running?
    QMutex routine_running_bool_mutex;
    bool is_routine_running = false;
    
    // led initialization
    void led_init();
    // table initialization
    void table_init();
    // set validations
    void set_validators();
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
    // set running state
    void set_routine_running_state(int state);
    // obtain table contains
    int obtain_table_contains(QVector<double> &routine_wait, QVector<double> &routine_heat, 
                               QVector<double> &routine_velocity, QVector<double> &routine_displacement);
    // obtain prequiescent widget data
    double obtain_prequiescent_data();
    // get central widgets
    QWidget* get_central_widget();
private slots:
    // cnc router comport connection
    void on_btn_cncconnect_clicked();
    void on_btn_up_clicked();
    void on_chb_consecutiveenable_clicked(bool checked);
    void on_btn_appenditem_clicked();
    void on_btn_removeitem_clicked();
    void on_btn_halt_clicked();
    void on_btn_left_clicked();
    void on_btn_down_clicked();
    void on_btn_right_clicked();
    void on_btn_forward_clicked();
    void on_btn_backward_clicked();
    void on_btn_arduinoconnect_clicked();
    void on_btn_cvdrun_clicked();
    void on_btn_tblsave_clicked();
    void on_btn_tblload_clicked();
    void on_btn_refreshcomport_clicked();
    void on_btn_flowmeter_clicked();
    
private:
    Ui::MainWindow *ui;
    // key pressed rate control
    int key_pressed_waitms = 100;
    // motion magnitude switch
    int mms = 0;
    // constant stepping speed
    double step_speed = 50.0;
    
    // QFileDialog last position
    QString last_load_file_pos = "C://";
    // QSettings parameter
    QSettings *settings = nullptr;
    
    // keypress and release capture
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    // change stepping/vel chb labels
    void change_chb_labels(bool is_consecutive_mode);
public slots:
    void run_signal_from_target();
    void trigger_volt_write(double volt);
    void init_arduino_serial_port();
    void cncrouter_msgbox();
    void psuarduino_msgbox();
    void set_flowrate(double propf = 0, double butaf = 0, double argof = 0);
};
#endif // MAINWINDOW_H
