#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // set leds
    this->led_init();
    // invisible consecutive chb
    ui->chb_consecutiveenable->setVisible(false);
    // set constraints on inputs
    this->set_validators();
    // install widgets focusing policy
    this->widgets_focus_setting();
    // set running stage table 
    this->table_init();
    // set progress bar
    ui->prograssbar->setValue(0);
    // set btn color
    ui->btn_halt->setStyleSheet("QPushButton {background-color : #DFDFDF; color : #FF3F16}");
    
    // set up target device component
    this->target_device = new TargetDevice(this);
    // scan available ports at the boot of the software (needs modification)
    this->target_device->ComportScan();
    this->obtain_comport_list();
    
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::led_init()
{
    // set up leds for serial port connection
    for(int i = 0; i < 2; i++)
    {
        this->serialport_leds.append(new QLedIndicator(this));
        // indicator, therefore disable clicking
        this->serialport_leds[i]->setEnabled(false);
    }
    ui->layout_cncserial->addWidget(this->serialport_leds[0]);
    ui->layout_arduinoserial->addWidget(this->serialport_leds[1]);
    // set up leds for prepositioning tab
    for(int i = 0; i < 3; i++)
    {
        this->positioning_leds.append(new QLedIndicator(this));
        // indicator, therefore disable clicking
        this->positioning_leds[i]->setEnabled(false);
    }
    ui->layout_positioning1->addWidget(this->positioning_leds[0]);
    ui->layout_positioning2->addWidget(this->positioning_leds[1]);
    ui->layout_positioning3->addWidget(this->positioning_leds[2]);
    this->positioning_leds[this->mms]->setChecked(true);
}

void MainWindow::table_init()
{
    this->std_table_model = new QStandardItemModel();
    this->std_table_model->setColumnCount(3);
    this->std_table_model->setRowCount(1);
    this->std_table_model->setHorizontalHeaderLabels(QStringList() << "wait (s)" << "heat" << "velocity" << "distance");
    ui->table_runningstages->setModel(this->std_table_model);
    for(int iter = 0; iter < 4; iter++)
    {
        ui->table_runningstages->setColumnWidth(iter, 160);
        ui->table_runningstages->model()->setData(ui->table_runningstages->model()->index(0, iter), 
                                                  Qt::AlignCenter, Qt::TextAlignmentRole);
        this->std_table_model->item(0, iter)->setFont(QFont("Consolas", 12, QFont::Normal));
    }
    ui->table_runningstages->setItemDelegate(new InputDelegate(this));
    ui->table_runningstages->horizontalHeader()->setFont(QFont("Consolas", 12, QFont::Bold));
    ui->table_runningstages->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    ui->table_runningstages->verticalHeader()->setFont(QFont("Consolas", 12, QFont::Bold));
    ui->table_runningstages->verticalHeader()->setDefaultAlignment(Qt::AlignCenter);
}

void MainWindow::set_validators()
{
    ui->ledit_xstep1->setValidator(new QDoubleValidator());
    ui->ledit_xstep2->setValidator(new QDoubleValidator());
    ui->ledit_xstep3->setValidator(new QDoubleValidator());
    ui->ledit_yzstep1->setValidator(new QDoubleValidator());
    ui->ledit_yzstep2->setValidator(new QDoubleValidator());
    ui->ledit_yzstep3->setValidator(new QDoubleValidator());
    ui->ledit_prequiescent->setValidator(new QDoubleValidator(0, 100, 1, this));
}

void MainWindow::widgets_focus_setting()
{
    ui->ledit_yzstep1->installEventFilter(new KeyEnterReceiver(this));
    ui->ledit_yzstep2->installEventFilter(new KeyEnterReceiver(this));
    ui->ledit_yzstep3->installEventFilter(new KeyEnterReceiver(this));
    ui->ledit_xstep1->installEventFilter(new KeyEnterReceiver(this));
    ui->ledit_xstep2->installEventFilter(new KeyEnterReceiver(this));
    ui->ledit_xstep3->installEventFilter(new KeyEnterReceiver(this));
    ui->ledit_prequiescent->installEventFilter(new KeyEnterReceiver(this));
    ui->ledit_inertgas->installEventFilter(new KeyEnterReceiver(this));
    ui->ledit_propanegas->installEventFilter(new KeyEnterReceiver(this));
    ui->ledit_butane->installEventFilter(new KeyEnterReceiver(this));
    ui->table_runningstages->installEventFilter(new KeyEnterReceiver(this));
}

void MainWindow::obtain_comport_list()
{
    int port_num = this->target_device->get_port_name_list().size();
    for(int iter = 0; iter < port_num; iter++)
    {
        ui->cbox_cncserialports->addItem(this->target_device->get_port_name_list()[iter]);
        ui->cbox_arduinoserialports->addItem(this->target_device->get_port_name_list()[iter]);
    }
}

void MainWindow::set_position_feedback_labels(double x, double y, double z)
{
    ui->ledit_xpos->setText(QString::number(x));
    ui->ledit_ypos->setText(QString::number(y));
    ui->ledit_zpos->setText(QString::number(z));
}

void MainWindow::set_position_state_labels(QString state)
{
    ui->motionstate->setText(state);
}

void MainWindow::move_x(double direction, double speed)
{
    double x_unit = 0;
    double flip = ui->chb_flipx->isChecked() ? -1 : 1;
    switch(this->mms)
    {
    case 0:
    {
        x_unit = ui->ledit_xstep1->text().toDouble() * direction * flip;
        break;
    }
    case 1:
    {
        x_unit = ui->ledit_xstep2->text().toDouble() * direction * flip;
        break;
    }
    case 2:
    {
        x_unit = ui->ledit_xstep3->text().toDouble() * direction * flip;
        break;
    }
    }
    this->target_device->move_cncrouter(ui->chb_consecutiveenable->isChecked(), x_unit, 0, 0, speed);
    QThread::msleep((int)(this->key_pressed_waitms * abs(x_unit)));
}

void MainWindow::move_y(double direction, double speed)
{
    double y_unit = 0;
    double flip = ui->chb_flipy->isChecked() ? -1 : 1;
    switch(this->mms)
    {
    case 0:
    {
        y_unit = ui->ledit_yzstep1->text().toDouble() * direction * flip;
        break;
    }
    case 1:
    {
        y_unit = ui->ledit_yzstep2->text().toDouble() * direction * flip;
        break;
    }
    case 2:
    {
        y_unit = ui->ledit_yzstep3->text().toDouble() * direction * flip;
        break;
    }
    }
    this->target_device->move_cncrouter(ui->chb_consecutiveenable->isChecked(), 0, y_unit, 0, speed);
    QThread::msleep((int)(this->key_pressed_waitms * abs(y_unit)));
}

void MainWindow::move_z(double direction, double speed)
{
    double z_unit = 0;
    double flip = ui->chb_flipz->isChecked() ? -1 : 1;
    switch(this->mms)
    {
    case 0:
    {
        z_unit = ui->ledit_yzstep1->text().toDouble() * direction * flip;
        break;
    }
    case 1:
    {
        z_unit = ui->ledit_yzstep2->text().toDouble() * direction * flip;
        break;
    }
    case 2:
    {
        z_unit = ui->ledit_yzstep3->text().toDouble() * direction * flip;
        break;
    }
    }
    this->target_device->move_cncrouter(ui->chb_consecutiveenable->isChecked(), 0, 0, z_unit, speed);
    QThread::msleep((int)(this->key_pressed_waitms * abs(z_unit)));
}

void MainWindow::set_routine_running_state(int state)
{
    ui->label_runningstages->setText(state == 0 ? "Idle" : "Running");
}

int MainWindow::obtain_table_contains(QVector<double> &routine_wait, QVector<double> &routine_heat, QVector<double> &routine_velocity, QVector<double> &routine_displacement)
{
    int row_num = this->std_table_model->rowCount();
    for(int iter = 0; iter < row_num; iter++)
    {
        routine_wait.append(this->std_table_model->index(iter, 0).data().toDouble());
        routine_heat.append(this->std_table_model->index(iter, 1).data().toDouble());
        routine_velocity.append(this->std_table_model->index(iter, 2).data().toDouble());
        routine_displacement.append(this->std_table_model->index(iter, 3).data().toDouble());
    }
    return row_num;
}

double MainWindow::obtain_prequiescent_data()
{
    return ui->ledit_prequiescent->text().toDouble();
}

QWidget* MainWindow::get_central_widget()
{
    return ui->centralwidget;
}


void MainWindow::on_btn_cncconnect_clicked()
{
    this->target_device->PickCNCRouterPort(
                this->target_device->get_port_name_list()[ui->cbox_cncserialports->currentIndex()]
                );
    this->target_device->CheckCNCRouterPortValid();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    // keys controlled by chb
    switch(event->key())
    {
    case Qt::Key_W:
    {
        this->move_z(1.0, 100.0);
        break;
    }
    case Qt::Key_S:
    {
        this->move_z(-1.0, 100.0);
        break;
    }
    case Qt::Key_A:
    {
        this->move_y(1.0, 100.0);
        break;
    }
    case Qt::Key_D:
    {
        this->move_y(-1.0, 100.0);
        break;
    }
    case Qt::Key_E:
    {
        this->move_x(1.0, 100.0);
        break;
    }
    case Qt::Key_Q:
    {
        this->move_x(-1.0, 100.0);
        break;
    }
    case Qt::Key_F:
    {
        // halt cncrouter
        this->target_device->halt_cncrouter();
        break;
    }
    case Qt::Key_1:
    {
        this->positioning_leds[this->mms]->setChecked(false);
        this->mms = 0;
        this->positioning_leds[this->mms]->setChecked(true);
        break;
    }
    case Qt::Key_2:
    {
        this->positioning_leds[this->mms]->setChecked(false);
        this->mms = 1;
        this->positioning_leds[this->mms]->setChecked(true);
        break;
    }
    case Qt::Key_3:
    {
        this->positioning_leds[this->mms]->setChecked(false);
        this->mms = 2;
        this->positioning_leds[this->mms]->setChecked(true);
        break;
    }
    case Qt::Key_Minus:
    {
        int select_row = this->std_table_model->rowCount(QModelIndex());
        if(select_row > 0)
        {
            this->std_table_model->removeRow(select_row - 1);
        }
        break;
    }
    case Qt::Key_Equal:
    {
        int row_num = this->std_table_model->rowCount(QModelIndex());
        this->std_table_model->insertRow(row_num);
        for(int iter = 0; iter < 4; iter++)
        {
            ui->table_runningstages->model()->setData(ui->table_runningstages->model()->index(row_num, iter), 
                                                      Qt::AlignCenter, Qt::TextAlignmentRole);
            this->std_table_model->item(row_num, iter)->setFont(QFont("Consolas", 12, QFont::Normal));
        }
        break;
    }
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    if(!event->isAutoRepeat())
    {
        switch(event->key())
        {
        case Qt::Key_W:
        case Qt::Key_S:
        case Qt::Key_A:
        case Qt::Key_D:
        case Qt::Key_Q:
        case Qt::Key_E:
        {
            this->target_device->position_query();
            break;
        }
        }
    }
}

void MainWindow::change_chb_labels(bool is_consecutive_mode)
{
    if(is_consecutive_mode)
    {
        ui->label_yzstep1->setText("YZ Vel #1");
        ui->label_yzstep2->setText("YZ Vel #2");
        ui->label_yzstep3->setText("YZ Vel #3");
        ui->label_xstep1->setText("X Vel #1");
        ui->label_xstep2->setText("X Vel #2");
        ui->label_xstep3->setText("X Vel #3");
    }
    else
    {
        ui->label_yzstep1->setText("YZ Step #1");
        ui->label_yzstep2->setText("YZ Step #2");
        ui->label_yzstep3->setText("YZ Step #3");
        ui->label_xstep1->setText("X Step #1");
        ui->label_xstep2->setText("X Step #2");
        ui->label_xstep3->setText("X Step #3");
    }
}

void MainWindow::on_chb_consecutiveenable_clicked(bool checked)
{
    this->change_chb_labels(checked);
}

void MainWindow::on_btn_appenditem_clicked()
{
    int row_num = this->std_table_model->rowCount(QModelIndex());
    this->std_table_model->insertRow(row_num);
    for(int iter = 0; iter < 4; iter++)
    {
        ui->table_runningstages->model()->setData(ui->table_runningstages->model()->index(row_num, iter), 
                                                  Qt::AlignCenter, Qt::TextAlignmentRole);
        this->std_table_model->item(row_num, iter)->setFont(QFont("Consolas", 12, QFont::Normal));
    }
}

void MainWindow::on_btn_removeitem_clicked()
{
    int select_row = this->std_table_model->rowCount(QModelIndex());
    if(select_row > 0)
    {
        this->std_table_model->removeRow(select_row - 1);
    }
}

void MainWindow::on_btn_halt_clicked()
{
    this->target_device->halt_cncrouter();
}

void MainWindow::on_btn_up_clicked()
{
    this->move_z(1.0, 50.0);
}

void MainWindow::on_btn_left_clicked()
{
    this->move_y(1.0, 50.0);
}

void MainWindow::on_btn_down_clicked()
{
    this->move_z(-1.0, 50.0);
}

void MainWindow::on_btn_right_clicked()
{
    this->move_y(-1.0, 50.0);
}

void MainWindow::on_btn_forward_clicked()
{
    this->move_x(1.0, 50.0);
}

void MainWindow::on_btn_backward_clicked()
{
    this->move_x(-1.0, 50.0);
}

void MainWindow::on_btn_arduinoconnect_clicked()
{
    this->target_device->PickPSUArduinoPort(
                this->target_device->get_port_name_list()[ui->cbox_arduinoserialports->currentIndex()]
                );
    this->target_device->CheckPSUArduinoPortValid();
}
