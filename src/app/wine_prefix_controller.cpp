#include "wine_prefix_controller.h"

#include <QDir>
#include <QFileInfo>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QStandardPaths>

WinePrefixController::WinePrefixController(QObject *parent)
    : WinePrefixController(
          QDir(QDir::homePath()).filePath(QStringLiteral(".local/share/win-to-linux/wine-prefixes")),
          QStandardPaths::findExecutable(QStringLiteral("wineboot")),
          QStandardPaths::findExecutable(QStringLiteral("wine")), parent)
{
}

WinePrefixController::WinePrefixController(const QString &rootDirectory,
                                           const QString &winebootExecutable,
                                           const QString &wineExecutable,
                                           QObject *parent)
    : QObject(parent),
      m_rootDirectory(QDir(rootDirectory).absolutePath()),
      m_winebootExecutable(winebootExecutable),
      m_wineExecutable(wineExecutable)
{
    m_status = available()
        ? QStringLiteral("No isolated Wine prefixes found.")
        : QStringLiteral("Wine and wineboot are both required for isolated prefixes.");

    connect(&m_process, &QProcess::finished, this,
            [this](int exitCode, QProcess::ExitStatus exitStatus) {
        const QString name = m_pendingPrefix;
        m_pendingPrefix.clear();
        setBusy(false);
        const QString path = safePrefixPath(name);
        const QFileInfo registry(QDir(path).filePath(QStringLiteral("system.reg")));
        if (exitStatus == QProcess::NormalExit && exitCode == 0 &&
            !path.isEmpty() && registry.isFile() && !registry.isSymLink()) {
            refresh();
            setStatus(QStringLiteral("Isolated Wine prefix '%1' is ready.").arg(name));
        } else {
            removeIncompletePrefix(name);
            refresh();
            setStatus(QStringLiteral("Could not initialize Wine prefix '%1'. The incomplete prefix was cleaned up.").arg(name));
        }
    });

    connect(&m_process, &QProcess::errorOccurred, this,
            [this](QProcess::ProcessError error) {
        if (error != QProcess::FailedToStart || !m_busy)
            return;
        const QString name = m_pendingPrefix;
        m_pendingPrefix.clear();
        setBusy(false);
        removeIncompletePrefix(name);
        refresh();
        setStatus(QStringLiteral("Could not start wineboot: %1").arg(m_process.errorString()));
    });

    refresh();
}

QStringList WinePrefixController::prefixes() const { return m_prefixes; }
bool WinePrefixController::busy() const { return m_busy; }
bool WinePrefixController::available() const
{
    return !m_winebootExecutable.isEmpty() && QFileInfo(m_winebootExecutable).isExecutable() &&
           !m_wineExecutable.isEmpty() && QFileInfo(m_wineExecutable).isExecutable();
}
QString WinePrefixController::status() const { return m_status; }
QString WinePrefixController::rootDirectory() const { return m_rootDirectory; }

bool WinePrefixController::validPrefixName(const QString &name) const
{
    static const QRegularExpression pattern(QStringLiteral("^[A-Za-z0-9][A-Za-z0-9._-]{0,39}$"));
    return pattern.match(name).hasMatch() && name != QStringLiteral(".") && name != QStringLiteral("..");
}

void WinePrefixController::setStatus(const QString &status)
{
    if (m_status == status)
        return;
    m_status = status;
    emit statusChanged();
}

void WinePrefixController::setBusy(bool busy)
{
    if (m_busy == busy)
        return;
    m_busy = busy;
    emit busyChanged();
}

QString WinePrefixController::safeExistingRoot() const
{
    const QFileInfo rootInfo(m_rootDirectory);
    if (!rootInfo.exists() || !rootInfo.isDir() || rootInfo.isSymLink())
        return {};
    const QString canonical = rootInfo.canonicalFilePath();
    if (canonical.isEmpty() || canonical == QStringLiteral("/") ||
        canonical == QDir::homePath())
        return {};
    return canonical;
}

QString WinePrefixController::safePrefixPath(const QString &name) const
{
    if (!validPrefixName(name))
        return {};
    const QString root = safeExistingRoot();
    if (root.isEmpty())
        return {};
    const QString path = QDir(root).filePath(name);
    const QFileInfo prefixInfo(path);
    if (!prefixInfo.exists() || !prefixInfo.isDir() || prefixInfo.isSymLink() ||
        prefixInfo.canonicalPath() != root)
        return {};
    return prefixInfo.canonicalFilePath();
}

void WinePrefixController::refresh()
{
    QStringList discovered;
    const QString root = safeExistingRoot();
    if (!root.isEmpty()) {
        const QFileInfoList entries = QDir(root).entryInfoList(
            QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name | QDir::IgnoreCase);
        for (const QFileInfo &entry : entries) {
            if (entry.isSymLink() || !validPrefixName(entry.fileName()) ||
                entry.canonicalPath() != root)
                continue;
            const QFileInfo registry(QDir(entry.absoluteFilePath()).filePath(QStringLiteral("system.reg")));
            if (registry.isFile() && !registry.isSymLink())
                discovered.append(entry.fileName());
        }
    }
    if (discovered != m_prefixes) {
        m_prefixes = discovered;
        emit prefixesChanged();
    } else {
        emit prefixesChanged();
    }
}

bool WinePrefixController::createPrefix(const QString &name)
{
    if (m_busy) {
        setStatus(QStringLiteral("Wait for the current Wine-prefix operation to finish."));
        return false;
    }
    if (!validPrefixName(name)) {
        setStatus(QStringLiteral("Use 1–40 letters, numbers, dots, underscores or hyphens; the first character must be a letter or number."));
        return false;
    }
    if (!available()) {
        setStatus(QStringLiteral("Wine and wineboot must both be installed and available on PATH."));
        return false;
    }
    if (!QDir().mkpath(m_rootDirectory)) {
        setStatus(QStringLiteral("Could not create the Wine-prefix storage directory."));
        return false;
    }
    const QFileInfo rootInfo(m_rootDirectory);
    const QString root = rootInfo.canonicalFilePath();
    if (!rootInfo.isDir() || rootInfo.isSymLink() || root.isEmpty() ||
        root == QStringLiteral("/") || root == QDir::homePath()) {
        setStatus(QStringLiteral("The Wine-prefix storage path failed its safety checks."));
        return false;
    }
    const QString path = QDir(root).filePath(name);
    const QFileInfo target(path);
    if (target.exists() || target.isSymLink()) {
        setStatus(QStringLiteral("That prefix name already exists. Choose a different name."));
        return false;
    }

    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    environment.insert(QStringLiteral("WINEPREFIX"), path);
    m_process.setProcessEnvironment(environment);
    m_process.setWorkingDirectory(QDir::homePath());
    m_process.setProgram(m_winebootExecutable);
    m_process.setArguments({QStringLiteral("-u")});
    m_process.setProcessChannelMode(QProcess::MergedChannels);
    m_pendingPrefix = name;
    setBusy(true);
    setStatus(QStringLiteral("Creating isolated Wine prefix '%1'…").arg(name));
    m_process.start();
    return true;
}

void WinePrefixController::removeIncompletePrefix(const QString &name)
{
    if (!validPrefixName(name))
        return;
    const QString root = safeExistingRoot();
    if (root.isEmpty())
        return;
    const QString path = QDir(root).filePath(name);
    const QFileInfo info(path);
    if (!info.exists() || !info.isDir() || info.isSymLink() || info.canonicalPath() != root)
        return;
    QDir(path).removeRecursively();
}

bool WinePrefixController::removePrefix(const QString &name)
{
    if (m_busy) {
        setStatus(QStringLiteral("Wait for the current Wine-prefix operation to finish."));
        return false;
    }
    if (!m_prefixes.contains(name)) {
        setStatus(QStringLiteral("Select an existing managed Wine prefix first."));
        return false;
    }
    const QString path = safePrefixPath(name);
    const QFileInfo registry(QDir(path).filePath(QStringLiteral("system.reg")));
    if (path.isEmpty() || !registry.isFile() || registry.isSymLink()) {
        setStatus(QStringLiteral("The selected prefix failed its safety checks."));
        return false;
    }
    if (!QDir(path).removeRecursively()) {
        setStatus(QStringLiteral("Could not remove the selected prefix."));
        return false;
    }
    refresh();
    setStatus(QStringLiteral("Removed isolated Wine prefix '%1'.").arg(name));
    return true;
}

bool WinePrefixController::launchWindowsFile(const QString &name, const QString &filePath)
{
    if (m_busy) {
        setStatus(QStringLiteral("Wait for the current Wine-prefix operation to finish."));
        return false;
    }
    if (m_wineExecutable.isEmpty() || !QFileInfo(m_wineExecutable).isExecutable()) {
        setStatus(QStringLiteral("Wine was not found on PATH."));
        return false;
    }
    if (!m_prefixes.contains(name)) {
        setStatus(QStringLiteral("Select an initialized isolated Wine prefix first."));
        return false;
    }
    const QString prefixPath = safePrefixPath(name);
    const QFileInfo registry(QDir(prefixPath).filePath(QStringLiteral("system.reg")));
    if (prefixPath.isEmpty() || !registry.isFile() || registry.isSymLink()) {
        setStatus(QStringLiteral("The selected prefix failed its safety checks."));
        return false;
    }
    const QFileInfo selected(filePath);
    const QString canonicalFile = selected.canonicalFilePath();
    const QString suffix = selected.suffix().toLower();
    if (canonicalFile.isEmpty() || !selected.isFile() ||
        (suffix != QStringLiteral("exe") && suffix != QStringLiteral("msi"))) {
        setStatus(QStringLiteral("Choose an existing .exe or .msi file."));
        return false;
    }

    QStringList arguments;
    if (suffix == QStringLiteral("msi"))
        arguments = {QStringLiteral("msiexec"), QStringLiteral("/i"), canonicalFile};
    else
        arguments = {canonicalFile};

    QProcess launcher;
    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    environment.insert(QStringLiteral("WINEPREFIX"), prefixPath);
    launcher.setProcessEnvironment(environment);
    launcher.setProgram(m_wineExecutable);
    launcher.setArguments(arguments);
    launcher.setWorkingDirectory(QDir::homePath());
    if (!launcher.startDetached()) {
        setStatus(QStringLiteral("Wine could not start the selected file in prefix '%1'.").arg(name));
        return false;
    }
    setStatus(QStringLiteral("Started the selected Windows file in isolated prefix '%1'. Compatibility is not guaranteed.").arg(name));
    return true;
}
