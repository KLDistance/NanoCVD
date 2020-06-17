#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // initializers
    this->std_table_model = new QStandardItemModel();
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

