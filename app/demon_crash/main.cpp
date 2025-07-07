#include "demon_crash.h"

#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QProcess>
#include <QStringList>
#include <QRegularExpression>
#include <QFileInfo>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    if (argc < 3) {
        QTextStream(stderr) << "Usage: LogViewer <input_log> <output_log> [<path_exe>] [<show_system_frames '0'|1>]\n";
        return 1;
    }

    QString inputLogPath = argv[1];
    QString outputLogPath = argv[2];
    QString exePath = (argc > 3) ? argv[3] : QString(); // если нет — пустая строка
    bool showSystemFrames = (argc > 4) ? (QString(argv[4]) == "1") : false; // по умолчанию false

    DemonCrash demon(inputLogPath, outputLogPath, exePath, showSystemFrames);

    return 0;
}
