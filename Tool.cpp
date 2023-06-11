#include "Tool.h"
#include <QDir>
#include <QMutex>
#include <QMutexLocker>

QMutex g_logMutex;

void QTool::Log(QString strLog)
{
    QMutexLocker locker(&g_logMutex);
    QDir logDir("");
    if (!logDir.exists("./Log/"))
        logDir.mkpath("./Log/");
    QString strFileName = QDateTime::currentDateTime().toString("yyyy_MM_dd") + ".txt";
    QFile file("./Log/" + strFileName);
    if (file.open(QFile::Append | QFile::ReadWrite))
    {
        file.write(QTime::currentTime().toString("hh:mm:ss.zzz：%1").arg(strLog).toLocal8Bit());
        file.write("\n");
        file.close();
    }
    return;
}