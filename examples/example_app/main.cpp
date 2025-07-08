#include "mainwindow.h"

#include "ErrorHandler.h"
#include <QApplication>

#include <iostream>
#include <fstream>
#include <csignal>
#include <cstdlib>
#include <QJsonDocument>
#include <QJsonObject>

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);

    ErrorHandler::setupSignalHandlers();

    MainWindow w;
    w.show();
    return a.exec();
}
