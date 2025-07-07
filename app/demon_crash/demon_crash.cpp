#include "demon_crash.h"

DemonCrash::DemonCrash(const QString& inputLog, const QString& outLog, const QString& pathExe, const bool& showSystemsFrame):
    _inputLog(inputLog),
    _outLog(outLog),
    _showSystemsFrame(showSystemsFrame),
    _pathExe(pathExe)
{
    QFile inputFile(_inputLog);
    if (!inputFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream(stderr) << "Cannot open input log file: " << _inputLog << "\n";
        return;
    }

    QFile outputFile(_outLog);
    if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream(stderr) << "Cannot open output log file: " << _outLog << "\n";
        return;
    }

    QTextStream in(&inputFile);
    QTextStream out(&outputFile);

    QRegularExpression addrRegex("0x[0-9a-fA-F]+"); // для поиска адрессов
    QString nameParent = "";

    QRegularExpression lineIdRegex(R"(^(\d+):\s*(.*))");

        bool inBacktrace = false;

        while (!in.atEnd()) {
            QString line = in.readLine();

            QRegularExpressionMatch match = lineIdRegex.match(line);
            if (!match.hasMatch()) {
//                if (inBacktrace) {
//                    // В backtrace идут адреса без префикса
//                    QString addr = line.trimmed();
//                    if (!addr.isEmpty()) {
//                        // Обработка адреса
//                        resolveAddress(_pathExe, addr, out, _showSystemsFrame);
//                    }
//                }
                continue;
            }

            int id = match.captured(1).toInt();
            QString content = match.captured(2);

            switch (id) {
                case 1:
                    // дата/время
                    out << "------------------------------------------\n";
                    out << "1: " << content << "\n";
                    inBacktrace = false;
                    break;
                case 2:
                    // файл (exe/lib)
                    out << "2: " << content << "\n";
                    _pathExe = content; // или логика выбора
                    if(isExecutablePath(_pathExe) != NULE_NAME){
                        nameParent = findDebugFileForExe(_pathExe);
                        if (nameParent == QString())
                            nameParent = findDebugFileForLib(_pathExe);
                        if (nameParent != QString())
                            out << "   " << nameParent << "\n";
                        else
                            out << "   debug_info Null" << "\n";
                    }

                    inBacktrace = false;
                    break;
                case 3:
                    // сигнал
                    out << "3: " << content << "\n";

                    inBacktrace = false;
                    break;
                case 4:
                    {
                        // описание
                        out << "4: " << content << "\n";
                        auto matchIterator = addrRegex.globalMatch(content);
                        while (matchIterator.hasNext()) {
                            auto match = matchIterator.next();
                            QString addr = match.captured(0);
                            resolveAddress(nameParent, addr, out, _showSystemsFrame);
                        }
                        inBacktrace = false;
                        break;
                    }
                case 5:
                    {
                        // instruction pointer
                        out << "5: " << content << "\n";
                        auto matchIterator = addrRegex.globalMatch(content);
                        while (matchIterator.hasNext()) {
                            auto match = matchIterator.next();
                            QString addr = match.captured(0);
                            resolveAddress(nameParent, addr, out, _showSystemsFrame);
                        }
                        inBacktrace = false;
                        out << "------------------------------------------\n";
                        break;
                    }
                case 6:
                    // backtrace начинается
                    out << id << "\n";
                    inBacktrace = true;
                    break;
                default:
                    out << line << "\n";
                    inBacktrace = false;
                    break;
            }
        }
}

//Это используеться если у нас showSystemsFrame = false, когда включен бэкртейс мы отсеивает строки принадлижащие системным библиотекам потому что скорее всего нет .debug инфы по ним ну или наоборот может не отсеивать (showSystemsFrame = true)
bool DemonCrash::isSystemFrame(const QString &line) {
    return line.contains("/usr/lib/") || line.contains("/lib/");
}

//Оределяет либа ли это
TypeFile DemonCrash::isLibraryFile(const QString &path) {
    QFileInfo fi(path);
    QString baseName = fi.fileName().toLower();

    if (baseName.endsWith(".so") || baseName.contains(".so.") ||
        baseName.endsWith(".dll") || baseName.endsWith(".dylib")) {
        return TypeFile::LIB_FILE;
    }
    return TypeFile::NULE_NAME;
}

//определяет путь это или нет. Файл или либа
TypeFile DemonCrash::isExecutablePath(const QString &line) {
    QFileInfo fi(line.trimmed());
    if (!fi.exists())
        return TypeFile::NULE_NAME;
    if (fi.isFile())
        return TypeFile::EXE_FILE;
    if (isLibraryFile(line.trimmed()) == TypeFile::LIB_FILE)
        return TypeFile::LIB_FILE;

    return TypeFile::NULE_NAME;
}

QString DemonCrash::findDebugFileForExe(const QString &pathExe) {
    QFileInfo exeInfo(pathExe);

    // 1. Сам exe с debug
    if (hasDebugSections(pathExe)) {
        return pathExe;
    }

    // 2. exe.debug рядом с exe
    QString debugFile1 = exeInfo.absoluteFilePath() + ".debug";
    if (QFileInfo::exists(debugFile1) && hasDebugSections(debugFile1)) {
        return debugFile1;
    }

    // 3. exe без расширения + ".debug"
        QString debugFile2 = exeInfo.absolutePath() + "/" + exeInfo.completeBaseName() + ".debug";
        if (QFileInfo::exists(debugFile2) && hasDebugSections(debugFile2)) {
            return debugFile2;
        }

    // 4. debug рядом с текущим приложением в его папке
    QString currentAppDir = QFileInfo(QCoreApplication::applicationFilePath()).absolutePath();
    QString debugFile3 = currentAppDir + "/" + exeInfo.completeBaseName() + ".debug";
    if (QFileInfo::exists(debugFile3) && hasDebugSections(debugFile3)) {
        return debugFile3;
    }

    // Если ничего не найдено
    return QString();
}

QString DemonCrash::findDebugFileForLib(const QString &libPath) {
    QFileInfo libInfo(libPath);
    QString baseName = libInfo.completeBaseName();
    QString libDir = libInfo.absolutePath();

    // 1. Проверяем сам файл
    if (hasDebugSections(libPath)) {
        return libPath;
    }

    // 2. Поиск похожих debug-файлов в директории библиотеки
    QDir dir(libDir);
    QStringList candidates = dir.entryList(QStringList() << (baseName + "*"), QDir::Files);
    for (const QString &fileName : candidates) {
        QString fullPath = libDir + "/" + fileName;
        if (hasDebugSections(fullPath)) {
            return fullPath;
        }
    }

    // 3. Поиск debug-файлов в каталоге _inputLog
    QFileInfo inputLogInfo(_inputLog);
    QString inputLogDir = inputLogInfo.absolutePath();
    QDir logDir(inputLogDir);
    QStringList logCandidates = logDir.entryList(QStringList() << (baseName + "*"), QDir::Files);
    for (const QString &fileName : logCandidates) {
        QString fullPath = inputLogDir + "/" + fileName;
        if (hasDebugSections(fullPath)) {
            return fullPath;
        }
    }

    // 4. Поиск debug рядом с текущим приложением
    QString appDir = QFileInfo(QCoreApplication::applicationFilePath()).absolutePath();
    QString debugFile = appDir + "/" + baseName + ".debug";
    if (QFileInfo::exists(debugFile) && hasDebugSections(debugFile)) {
        return debugFile;
    }

    // Если ничего не найдено
    return QString();
}


void DemonCrash::printCodeSnippet(const QString &filePath, int errorLine, QTextStream &out, int contextLines ) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        out << "    Code: Cannot open source file\n";
        return;
    }
    QTextStream in(&file);

    int startLine = qMax(1, errorLine - contextLines);
    int endLine = errorLine + contextLines;
    int currentLine = 0;

    while (!in.atEnd()) {
        QString line = in.readLine();
        ++currentLine;
        if (currentLine < startLine) continue;
        if (currentLine > endLine) break;

        if (currentLine == errorLine)
            out << " -> " << currentLine << ": " << line << "   <-- ERROR HERE\n";
        else
            out << "    " << currentLine << ": " << line << "\n";
    }
}

void DemonCrash::resolveAddress(const QString &exePath, const QString &address, QTextStream &out, bool writeSystemFrames) {
    QProcess p;
    QStringList args = {"-e", exePath, "-f", "-C", address};
    p.start("addr2line", args);
    p.waitForFinished();
    QString result = p.readAllStandardOutput().trimmed();

    if (result.contains("??") || result.contains("??:0") || result.contains("unknown", Qt::CaseInsensitive)) {
        out << " -> " << address << " -> Unknown location\n";
        return;
    }

    QStringList lines = result.split('\n');
    if (lines.size() < 2) {
        out << " -> " << address << " -> Unknown location\n";
        return;
    }

    if (!writeSystemFrames && isSystemFrame(lines[1])) {
        return; // Пропускаем системные вызовы
    }

    out << " -> " << address << " resolved to:\n";
    out << "    Function: " << lines[0] << "\n";
    out << "    Location: " << lines[1] << "\n";

    // Парсим номер строки из строки вида /path/to/file.cpp:123
    QRegularExpression re(":(\\d+)$");
    QRegularExpressionMatch match = re.match(lines[1]);
    if (match.hasMatch()) {
        int lineNumber = match.captured(1).toInt();
        printCodeSnippet(lines[1].section(':', 0, 0), lineNumber, out);
    } else {
        out << "    Code: Cannot determine line number\n";
    }
}

bool DemonCrash::hasDebugSections(const QString &exePath) {
    QProcess p;
    p.start("readelf", {"-S", exePath});
    p.waitForFinished(3000);
    QString output = p.readAllStandardOutput();

    // Проверяем наличие секций .debug_info или подобных
    return output.contains(".debug_info");
}



