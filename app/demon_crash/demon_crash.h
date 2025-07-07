#ifndef DEMON_CRASH_H
#define DEMON_CRASH_H

#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QProcess>
#include <QStringList>
#include <QRegularExpression>
#include <QFileInfo>
#include <QDir>

enum TypeFile {
    NULE_NAME,
    EXE_FILE,
    LIB_FILE,
};

class DemonCrash
{
public:
    DemonCrash(const QString& inputLog, const QString& outLog, const QString& pathExe = "", const bool& showSystemsFrame = false);

private:
    QString _inputLog;
    QString _outLog;
    bool _showSystemsFrame;
    QString _pathExe;


    bool isSystemFrame(const QString &line);
    void printCodeSnippet(const QString &filePath, int errorLine, QTextStream &out, int contextLines = 2);

    void resolveAddress(const QString &exePath, const QString &address, QTextStream &out, bool writeSystemFrames);

    bool hasDebugSections(const QString &exePath);
    //Оределяет либа ли это
    TypeFile isLibraryFile(const QString &path);

    //определяет путь это или нет. Файл или либа
    TypeFile isExecutablePath(const QString &line);

    QString findDebugFileForExe(const QString &pathExe);

    QString findDebugFileForLib(const QString &pathExe);
};
#endif // DEMON_CRASH_H
