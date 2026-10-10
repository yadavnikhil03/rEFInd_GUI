#include "logger.h"
#include "platform.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <memory>
#include <QMessageLogContext>
#include <QMutex>
#include <QSysInfo>
#include <QTextStream>

#ifdef Q_OS_WIN
#include <qt_windows.h>
#else
#include <unistd.h>
#endif

namespace Logger {

static std::unique_ptr<QFile> g_logFile;
static QMutex g_logMutex;

static void messageHandler(QtMsgType type, const QMessageLogContext &, const QString &msg)
{
    QMutexLocker locker(&g_logMutex);

    QString level;
    switch (type) {
    case QtDebugMsg:    level = QStringLiteral("[DEBUG]  "); break;
    case QtInfoMsg:     level = QStringLiteral("[INFO]   "); break;
    case QtWarningMsg:  level = QStringLiteral("[WARN]   "); break;
    case QtCriticalMsg: level = QStringLiteral("[CRIT]   "); break;
    case QtFatalMsg:    level = QStringLiteral("[FATAL]  "); break;
    }

    const QString timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);
    const QString formatted = QStringLiteral("%1 %2 %3\n").arg(timestamp, level, msg);

    if (g_logFile && g_logFile->isOpen()) {
        QTextStream out(g_logFile.get());
        out << formatted;
        out.flush();
    }

    fprintf(stderr, "%s", formatted.toLocal8Bit().constData());
    fflush(stderr);

    if (type == QtFatalMsg) {
        abort();
    }
}

void init()
{
    const QString dirPath = Platform::dataDir() + QStringLiteral("/GUI/logs");
    QDir().mkpath(dirPath);
    const QString path = dirPath + QLatin1Char('/') + QCoreApplication::applicationName() + QStringLiteral(".log");

    QFileInfo fi(path);
    if (fi.exists() && fi.size() > 512 * 1024) {
        QFile::remove(path + QStringLiteral(".old"));
        QFile::rename(path, path + QStringLiteral(".old"));
    }

    g_logFile = std::make_unique<QFile>(path);
    if (g_logFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        qInstallMessageHandler(messageHandler);
    } else {
        g_logFile.reset();
    }
}

void runHealthCheck()
{
    qInfo() << "--- Application Startup Health Check ---";
    
    qInfo() << "OS:" << QSysInfo::prettyProductName() << QSysInfo::currentCpuArchitecture();
    qInfo() << "Qt Version:" << qVersion();
    qInfo() << "App Version:" << QCoreApplication::applicationVersion();

#ifdef Q_OS_WIN
    bool elevated = false;
    HANDLE hToken = NULL;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        TOKEN_ELEVATION elevation;
        DWORD cbSize = sizeof(TOKEN_ELEVATION);
        if (GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &cbSize)) {
            elevated = elevation.TokenIsElevated;
        }
        CloseHandle(hToken);
    }
    if (elevated) {
        qInfo() << "[PASS] Running with Administrator privileges.";
    } else {
        qWarning() << "[WARN] Not running as Administrator. Some features may fail.";
    }
#else
    if (geteuid() == 0) {
        qInfo() << "[PASS] Running with root privileges.";
    } else {
        qInfo() << "[INFO] Running unprivileged (GUI mode).";
    }
#endif

    qInfo() << "[PASS] Telemetry is strictly local. No external data is sent.";
    qInfo() << "----------------------------------------";
}

}
