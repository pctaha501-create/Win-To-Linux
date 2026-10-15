#include "process_monitor.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QVariantMap>
#include <algorithm>
#include <csignal>
#include <unistd.h>

namespace {
bool readStartTicks(qint64 pid, qint64 *ticks) {
 QFile file(QStringLiteral("/proc/%1/stat").arg(pid));
 if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
 const QByteArray line = file.readAll().trimmed();
 const qsizetype closeParen = line.lastIndexOf(')');
 if (closeParen < 0 || closeParen + 2 >= line.size()) return false;
 const QList<QByteArray> fields = line.mid(closeParen + 2).simplified().split(' ');
 // The first field here is proc stat field 3 (state); starttime is field 22.
 if (fields.size() <= 19) return false;
 bool ok = false;
 const qint64 value = fields.at(19).toLongLong(&ok);
 if (!ok) return false;
 *ticks = value;
 return true;
}
bool readRealUid(qint64 pid, uid_t *uid) {
 QFile file(QStringLiteral("/proc/%1/status").arg(pid));
 if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
 while (!file.atEnd()) {
  const QByteArray line = file.readLine();
  if (!line.startsWith("Uid:")) continue;
  const QList<QByteArray> values = line.mid(4).simplified().split(' ');
  if (values.isEmpty()) return false;
  bool ok = false;
  const uint value = values.first().toUInt(&ok);
  if (!ok) return false;
  *uid = static_cast<uid_t>(value);
  return true;
 }
 return false;
}
QString processName(qint64 pid) {
 QFile file(QStringLiteral("/proc/%1/comm").arg(pid));
 if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return {};
 return QString::fromUtf8(file.readAll()).trimmed();
}
qint64 residentKiB(qint64 pid) {
 QFile file(QStringLiteral("/proc/%1/status").arg(pid));
 if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return -1;
 while (!file.atEnd()) {
  const QByteArray line = file.readLine();
  if (!line.startsWith("VmRSS:")) continue;
  const QList<QByteArray> values = line.mid(6).simplified().split(' ');
  bool ok = false;
  const qint64 value = values.value(0).toLongLong(&ok);
  return ok ? value : -1;
 }
 return -1;
}
}
ProcessMonitor::ProcessMonitor(QObject *parent) : QObject(parent) { refreshProcesses(); }
QVariantList ProcessMonitor::processes() const { return m_processes; }
QString ProcessMonitor::status() const { return m_status; }
void ProcessMonitor::setStatus(const QString &s) { if (m_status == s) return; m_status = s; emit statusChanged(); }
void ProcessMonitor::refreshProcesses() {
 QVariantList result;
 QDir proc(QStringLiteral("/proc"));
 const QFileInfoList entries = proc.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
 const uid_t ownUid = getuid();
 for (const QFileInfo &entry : entries) {
  bool ok = false;
  const qint64 pid = entry.fileName().toLongLong(&ok);
  if (!ok || pid <= 0) continue;
  const QString name = processName(pid);
  if (name.isEmpty()) continue;
  qint64 ticks = 0;
  if (!readStartTicks(pid, &ticks)) continue;
  uid_t uid = static_cast<uid_t>(-1);
  const bool hasUid = readRealUid(pid, &uid);
  const qint64 rss = residentKiB(pid);
  result.append(QVariantMap{
   {QStringLiteral("pid"), pid},
   {QStringLiteral("name"), name},
   {QStringLiteral("memoryKiB"), rss},
   {QStringLiteral("memoryText"), rss < 0 ? QStringLiteral("—") : QStringLiteral("%1 MiB").arg(static_cast<double>(rss) / 1024.0, 0, 'f', 1)},
   {QStringLiteral("canTerminate"), hasUid && uid == ownUid && pid > 1 && pid != static_cast<qint64>(getpid())},
   {QStringLiteral("startTicks"), ticks}
  });
 }
 std::sort(result.begin(), result.end(), [](const QVariant &a, const QVariant &b) {
  return a.toMap().value(QStringLiteral("name")).toString().toCaseFolded()
   < b.toMap().value(QStringLiteral("name")).toString().toCaseFolded();
 });
 if (result.size() > 400) result = result.mid(0, 400);
 m_processes = result;
 emit processesChanged();
 setStatus(QStringLiteral("Showing %1 processes. Only processes owned by this user can be requested to stop.").arg(result.size()));
}
bool ProcessMonitor::terminateProcess(qint64 pid, qint64 expectedStartTicks) {
 if (pid <= 1 || pid == static_cast<qint64>(getpid()) || expectedStartTicks < 0) {
  setStatus(QStringLiteral("Refused to stop a protected or invalid process."));
  return false;
 }
 qint64 actualTicks = 0;
 if (!readStartTicks(pid, &actualTicks) || actualTicks != expectedStartTicks) {
  setStatus(QStringLiteral("Process changed or exited; refresh the list and try again."));
  refreshProcesses();
  return false;
 }
 uid_t uid = static_cast<uid_t>(-1);
 if (!readRealUid(pid, &uid) || uid != getuid()) {
  setStatus(QStringLiteral("Permission denied: only your own processes can be stopped here."));
  return false;
 }
 if (::kill(static_cast<pid_t>(pid), SIGTERM) != 0) {
  setStatus(QStringLiteral("Could not send a stop request. The process may have exited or be protected."));
  refreshProcesses();
  return false;
 }
 setStatus(QStringLiteral("Stop request sent to PID %1. The process may need time to exit.").arg(pid));
 refreshProcesses();
 return true;
}
