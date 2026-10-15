#pragma once
#include <QObject>
#include <QVariantList>
class FileManager final : public QObject {
 Q_OBJECT
 Q_PROPERTY(QString currentPath READ currentPath NOTIFY currentPathChanged)
 Q_PROPERTY(QVariantList entries READ entries NOTIFY entriesChanged)
 Q_PROPERTY(QString status READ status NOTIFY statusChanged)
 Q_PROPERTY(bool canGoUp READ canGoUp NOTIFY currentPathChanged)
public:
 explicit FileManager(QObject *parent=nullptr);
 QString currentPath() const;
 QVariantList entries() const;
 QString status() const;
 bool canGoUp() const;
 Q_INVOKABLE bool navigate(const QString &path);
 Q_INVOKABLE bool goUp();
 Q_INVOKABLE void refresh();
 Q_INVOKABLE bool openEntry(const QString &path);
 Q_INVOKABLE bool createFolder(const QString &name);
 Q_INVOKABLE bool renameEntry(const QString &path, const QString &newName);
 Q_INVOKABLE bool deleteEntry(const QString &path, bool confirmDirectoryRemoval=false);
signals:
 void currentPathChanged();
 void entriesChanged();
 void statusChanged();
private:
 QString m_currentPath,m_status;
 QVariantList m_entries;
 void setStatus(const QString &status);
 bool isDirectChild(const QString &path, QFileInfo *info=nullptr) const;
 bool isSafeName(const QString &name) const;
};
