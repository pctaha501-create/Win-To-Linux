#pragma once
#include <QObject>
#include <QVariantList>
#include <QHash>
class ProcessMonitor final : public QObject {
 Q_OBJECT
 Q_PROPERTY(QVariantList processes READ processes NOTIFY processesChanged)
 Q_PROPERTY(QString status READ status NOTIFY statusChanged)
 Q_PROPERTY(double totalCpuPercent READ totalCpuPercent NOTIFY processesChanged)
 Q_PROPERTY(double memoryPercent READ memoryPercent NOTIFY processesChanged)
 Q_PROPERTY(QString memoryText READ memoryText NOTIFY processesChanged)
public:
 explicit ProcessMonitor(QObject *parent=nullptr);
 QVariantList processes() const;
 QString status() const;
 double totalCpuPercent() const;
 double memoryPercent() const;
 QString memoryText() const;
 Q_INVOKABLE void refreshProcesses();
 Q_INVOKABLE bool terminateProcess(qint64 pid, qint64 expectedStartTicks);
signals:
 void processesChanged();
 void statusChanged();
private:
 QVariantList m_processes;
 QString m_status;
 QHash<qint64,qint64> m_previousCpuTicks;
 QHash<qint64,qint64> m_previousStartTicks;
 qint64 m_previousTotalCpuTicks = 0;
 qint64 m_previousIdleCpuTicks = 0;
 double m_totalCpuPercent = 0.0;
 double m_memoryPercent = 0.0;
 QString m_memoryText = QStringLiteral("Unavailable");
 void setStatus(const QString &status);
};
