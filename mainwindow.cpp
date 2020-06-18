#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // initializers
    this->std_table_model = new QStandardItemModel();
    // set leds
    this->led_init();
    // set constraints on inputs
    ui->ledit_prequiescent->setValidator(new QDoubleValidator(0, 100, 2, this));
    // set running stage table counts
    this->std_table_model->setColumnCount(4);
    this->std_table_model->setRowCount(1);
    ui->table_runningstages->setModel(this->std_table_model);
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
    this->positioning_leds[0]->toggle();
}

