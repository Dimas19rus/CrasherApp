#include "demon_crash.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    if (argc < 2) {
        QTextStream(stderr) << "Usage: LogViewer <crash_folder> [<path_exe>] [<show_system_frames '0'|1>]\n";
        return 1;
    }

    QString crashDirPath = argv[1];
    QString exePath = (argc > 2) ? argv[2] : QString();
    bool showSystemFrames = (argc > 3) ? (QString(argv[3]) == "1") : false;

    QDir crashDir(crashDirPath);
    if (!crashDir.exists()) {
        QTextStream(stderr) << "Crash directory does not exist: " << crashDirPath << "\n";
        return 1;
    }

    // Создаем папку out рядом с исполняемым файлом
    QString outDirPath = QCoreApplication::applicationDirPath() + "/out";
    QDir().mkpath(outDirPath);

    QStringList jsonFiles = crashDir.entryList(QStringList() << "crash_*.json", QDir::Files);
    if (jsonFiles.isEmpty()) {
        QTextStream(stderr) << "No crash_*.json files found in directory: " << crashDirPath << "\n";
        return 1;
    }

    for (const QString& fileName : jsonFiles) {
        QString inputPath = crashDir.absoluteFilePath(fileName);
        QString baseName = QFileInfo(fileName).completeBaseName(); // без .json
        QString outputPath = outDirPath + "/" + baseName + ".txt";

        DemonCrash demon(inputPath, outputPath, exePath, showSystemFrames);
    }

    return 0;
}
