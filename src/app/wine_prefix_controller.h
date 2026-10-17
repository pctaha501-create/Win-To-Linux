#pragma once

#include <QObject>
#include <QProcess>
#include <QStringList>

class WinePrefixController final : public QObject {
    Q_OBJECT
    Q_PROPERTY(QStringList prefixes READ prefixes NOTIFY prefixesChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(bool available READ available CONSTANT)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString output READ output NOTIFY outputChanged)
    Q_PROPERTY(QString rootDirectory READ rootDirectory CONSTANT)

public:
    explicit WinePrefixController(QObject *parent = nullptr);
    WinePrefixController(const QString &rootDirectory,
                         const QString &winebootExecutable,
                         const QString &wineExecutable,
                         QObject *parent = nullptr);

    QStringList prefixes() const;
    bool busy() const;
    bool available() const;
    QString status() const;
    QString output() const;
    QString rootDirectory() const;

    Q_INVOKABLE bool validPrefixName(const QString &name) const;
    Q_INVOKABLE void refresh();
    Q_INVOKABLE bool createPrefix(const QString &name);
    Q_INVOKABLE bool removePrefix(const QString &name);
    Q_INVOKABLE bool launchWindowsFile(const QString &name, const QString &filePath);

signals:
    void prefixesChanged();
    void busyChanged();
    void statusChanged();
    void outputChanged();

private:
    QString m_rootDirectory;
    QString m_winebootExecutable;
    QString m_wineExecutable;
    QStringList m_prefixes;
    QString m_status;
    QString m_output;
    QString m_pendingPrefix;
    bool m_busy = false;
    QProcess m_process;

    void setStatus(const QString &status);
    void setBusy(bool busy);
    void appendOutput(const QString &text);
    QString safeExistingRoot() const;
    QString safePrefixPath(const QString &name) const;
    void removeIncompletePrefix(const QString &name);
};
