#include "process_monitor.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QVariantMap>
#include <QHash>
#include <QSet>
#include <functional>
#include <algorithm>
#include <csignal>
#include <cerrno>
#include <sys/syscall.h>
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
bool readMemoryStats(qint64 *totalKiB, qint64 *availableKiB) {
 QFile file(QStringLiteral("/proc/meminfo"));
 if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
 bool foundTotal = false, foundAvailable = false;
 while (!file.atEnd()) {
  const QByteArray line = file.readLine();
  if (!line.startsWith("MemTotal:") && !line.startsWith("MemAvailable:")) continue;
  const QList<QByteArray> fields = line.simplified().split(' ');
  if (fields.size() < 2) continue;
  bool ok = false;
  const qint64 value = fields.at(1).toLongLong(&ok);
  if (!ok || value <= 0) continue;
  if (fields.first() == "MemTotal:") { *totalKiB = value; foundTotal = true; }
  else { *availableKiB = value; foundAvailable = true; }
 }
 return foundTotal && foundAvailable && *totalKiB > 0 && *availableKiB >= 0;
}
bool readCpuStats(qint64 *totalTicks, qint64 *idleTicks) {
 QFile file(QStringLiteral("/proc/stat"));
 if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
 const QList<QByteArray> fields = file.readLine().simplified().split(' ');
 if (fields.size() < 9 || fields.first() != "cpu") return false;
 qint64 total = 0;
 qint64 idle = 0;
 for (qsizetype i = 1; i < 9; ++i) {
  bool ok = false;
  const qint64 value = fields.at(i).toLongLong(&ok);
  if (!ok || value < 0) return false;
  total += value;
  if (i == 4 || i == 5) idle += value; // idle + iowait
 }
 *totalTicks = total;
 *idleTicks = idle;
 return true;
}
bool readProcessCpuTicks(qint64 pid, qint64 *ticks) {
 QFile file(QStringLiteral("/proc/%1/stat").arg(pid));
 if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
 const QByteArray line = file.readAll().trimmed();
 const qsizetype closeParen = line.lastIndexOf(')');
 if (closeParen < 0 || closeParen + 2 >= line.size()) return false;
 const QList<QByteArray> fields = line.mid(closeParen + 2).simplified().split(' ');
 // After the process name, utime and stime are fields 14 and 15 (indices 11 and 12).
 if (fields.size() <= 12) return false;
 bool userOk = false, systemOk = false;
 const qint64 userTicks = fields.at(11).toLongLong(&userOk);
 const qint64 systemTicks = fields.at(12).toLongLong(&systemOk);
 if (!userOk || !systemOk || userTicks < 0 || systemTicks < 0) return false;
 *ticks = userTicks + systemTicks;
 return true;
}
qint64 parentProcessId(qint64 pid) {
 QFile file(QStringLiteral("/proc/%1/stat").arg(pid));
 if(!file.open(QIODevice::ReadOnly|QIODevice::Text))return -1;
 const QByteArray line=file.readAll().trimmed();
 const qsizetype closeParen=line.lastIndexOf(')');
 if(closeParen<0||closeParen+2>=line.size())return -1;
 const QList<QByteArray> fields=line.mid(closeParen+2).simplified().split(' ');
 if(fields.size()<2)return -1;
 bool ok=false;const qint64 value=fields.at(1).toLongLong(&ok);
 return ok&&value>=0?value:-1;
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
double ProcessMonitor::totalCpuPercent() const { return m_totalCpuPercent; }
double ProcessMonitor::memoryPercent() const { return m_memoryPercent; }
QString ProcessMonitor::memoryText() const { return m_memoryText; }
void ProcessMonitor::setStatus(const QString &s) { if (m_status == s) return; m_status = s; emit statusChanged(); }
void ProcessMonitor::refreshProcesses() {
 QVariantList result;
 QDir proc(QStringLiteral("/proc"));
 const QFileInfoList entries = proc.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
 const uid_t ownUid = getuid();
 qint64 totalCpuTicks = -1, idleCpuTicks = -1;
 const bool hasCpuStats = readCpuStats(&totalCpuTicks, &idleCpuTicks);
 qint64 totalMemoryKiB = 0, availableMemoryKiB = 0;
 if (readMemoryStats(&totalMemoryKiB, &availableMemoryKiB)) {
  const qint64 usedMemoryKiB = qMax<qint64>(0, totalMemoryKiB - availableMemoryKiB);
  m_memoryPercent = qBound(0.0, 100.0 * static_cast<double>(usedMemoryKiB) / static_cast<double>(totalMemoryKiB), 100.0);
  m_memoryText = QStringLiteral("%1 / %2 MiB").arg(static_cast<double>(usedMemoryKiB) / 1024.0, 0, 'f', 0).arg(static_cast<double>(totalMemoryKiB) / 1024.0, 0, 'f', 0);
 } else { m_memoryPercent = 0.0; m_memoryText = QStringLiteral("Unavailable"); }

 QHash<qint64,qint64> currentCpuTicks;
 QHash<qint64,qint64> currentStartTicks;
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
  const qint64 ppid = parentProcessId(pid);
  qint64 processCpuTicks = 0;
  const bool hasCpuTicks = readProcessCpuTicks(pid, &processCpuTicks);
  double cpuPercent = 0.0;
  if (hasCpuTicks && totalCpuTicks > m_previousTotalCpuTicks && m_previousStartTicks.value(pid, -1) == ticks) {
   const qint64 previousTicks = m_previousCpuTicks.value(pid, -1);
   if (previousTicks >= 0 && processCpuTicks >= previousTicks) {
    cpuPercent = qBound(0.0, 100.0 * static_cast<double>(processCpuTicks - previousTicks)
      / static_cast<double>(totalCpuTicks - m_previousTotalCpuTicks), 100.0);
   }
  }
  if (hasCpuTicks) { currentCpuTicks.insert(pid, processCpuTicks); currentStartTicks.insert(pid, ticks); }
  result.append(QVariantMap{
   {QStringLiteral("pid"), pid},
   {QStringLiteral("parentPid"), ppid},
   {QStringLiteral("name"), name},
   {QStringLiteral("memoryKiB"), rss},
   {QStringLiteral("cpuPercent"), cpuPercent},
   {QStringLiteral("cpuText"), QStringLiteral("%1%").arg(cpuPercent, 0, 'f', 1)},
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

 QHash<qint64,QVariantMap> byPid;
 for(const QVariant &entry:result){const QVariantMap item=entry.toMap();byPid.insert(item.value(QStringLiteral("pid")).toLongLong(),item);}
 QHash<qint64,QList<qint64>> children;
 QList<qint64> roots;
 for(const QVariant &entry:result){
  const QVariantMap item=entry.toMap();
  const qint64 pid=item.value(QStringLiteral("pid")).toLongLong();
  const qint64 ppid=item.value(QStringLiteral("parentPid")).toLongLong();
  if(ppid>0&&ppid!=pid&&byPid.contains(ppid))children[ppid].append(pid);else roots.append(pid);
 }
 auto sortPids=[&byPid](QList<qint64> &pids){
  std::sort(pids.begin(),pids.end(),[&byPid](qint64 a,qint64 b){
   return byPid.value(a).value(QStringLiteral("name")).toString().toCaseFolded()
    < byPid.value(b).value(QStringLiteral("name")).toString().toCaseFolded();
  });
 };
 sortPids(roots);
 for(auto it=children.begin();it!=children.end();++it)sortPids(it.value());
 QVariantList tree;
 QSet<qint64> visited;
 std::function<void(qint64,int)> appendTree=[&](qint64 pid,int depth){
  if(visited.contains(pid)||depth>32||tree.size()>=400)return;
  visited.insert(pid);
  QVariantMap item=byPid.value(pid);
  item.insert(QStringLiteral("depth"),depth);
  tree.append(item);
  for(qint64 child:children.value(pid))appendTree(child,depth+1);
 };
 for(qint64 pid:roots)appendTree(pid,0);
 for(const QVariant &entry:result){
  const qint64 pid=entry.toMap().value(QStringLiteral("pid")).toLongLong();
  if(!visited.contains(pid))appendTree(pid,0);
 }
 if (hasCpuStats && m_previousTotalCpuTicks > 0 && totalCpuTicks > m_previousTotalCpuTicks
     && idleCpuTicks >= m_previousIdleCpuTicks) {
  const qint64 elapsed = totalCpuTicks - m_previousTotalCpuTicks;
  const qint64 idleElapsed = idleCpuTicks - m_previousIdleCpuTicks;
  m_totalCpuPercent = qBound(0.0, 100.0 * static_cast<double>(elapsed - qMin(elapsed, idleElapsed))
    / static_cast<double>(elapsed), 100.0);
 } else if (!hasCpuStats) {
  m_totalCpuPercent = 0.0;
 }
 m_processes = tree;
 m_previousCpuTicks = currentCpuTicks;
 m_previousStartTicks = currentStartTicks;
 if (hasCpuStats) {
  m_previousTotalCpuTicks = totalCpuTicks;
  m_previousIdleCpuTicks = idleCpuTicks;
 }
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
#if defined(SYS_pidfd_open) && defined(SYS_pidfd_send_signal)
 // Bind the signal to the process identity, not just its numeric PID. A process
 // can exit and its PID can be reused between a /proc check and kill(pid, ...).
 const int processHandle=static_cast<int>(::syscall(SYS_pidfd_open,static_cast<pid_t>(pid),0));
 if(processHandle<0){
  setStatus(errno==ESRCH?QStringLiteral("Process already exited; refresh the list and try again."):
   QStringLiteral("Safe process handles are unavailable; refusing to risk signaling a reused PID."));
  refreshProcesses();return false;
 }
 qint64 boundTicks=0;uid_t boundUid=static_cast<uid_t>(-1);
 const bool identityMatches=readStartTicks(pid,&boundTicks)&&boundTicks==expectedStartTicks&&readRealUid(pid,&boundUid)&&boundUid==getuid();
 if(!identityMatches){::close(processHandle);setStatus(QStringLiteral("Process changed or exited; no signal was sent."));refreshProcesses();return false;}
 const long signalResult=::syscall(SYS_pidfd_send_signal,processHandle,SIGTERM,nullptr,0);
 const int signalError=errno;::close(processHandle);
 if(signalResult!=0){
  setStatus(signalError==ESRCH?QStringLiteral("Process already exited; refresh the list and try again."):
   QStringLiteral("Could not send a safe stop request. The process may be protected."));
  refreshProcesses();return false;
 }
#else
 setStatus(QStringLiteral("This kernel lacks safe PID-bound signaling; refusing to risk stopping a reused PID."));
 return false;
#endif
 setStatus(QStringLiteral("Stop request sent to PID %1. The process may need time to exit.").arg(pid));
 refreshProcesses();
 return true;
}
