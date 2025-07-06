#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <QFile>
#include <QMutex>
#include <QTextStream>
#include <QDateTime>


class Logger
{
public:
    static Logger& instance()
    {
        static Logger logger;
        return logger;
    }

    void setLogFile(const QString& path)
    {
        QMutexLocker locker(&mutex);
        if (logFile.isOpen())
            logFile.close();

        logFile.setFileName(path);
        logFile.open(QIODevice::Append | QIODevice::Text);
    }

    void appendLine(const QString& line)
    {
        QMutexLocker locker(&mutex);
        if (logFile.isOpen())
        {
            QTextStream out(&logFile);
            out << QDateTime::currentDateTime().toString("[yyyy-MM-dd HH:mm:ss.zzz] ")
                << line << "\n";
            logFile.flush();
        }
    }

private:
    Logger() {}
    ~Logger() { logFile.close(); }

    QFile logFile;
    QMutex mutex;
};

#endif // LOGGER_H
