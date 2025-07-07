#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "libtest.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
    int *p = nullptr;
        *p = 42; // вызовет SIGSEGV
}

void MainWindow::on_pushButton_2_clicked()
{
    Libtest lib;
    lib.generateError();
}
