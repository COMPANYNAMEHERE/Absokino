#include "recentfilesmodel.h"
#include <QFileInfo>

RecentFilesModel *RecentFilesModel::s_instance = nullptr;

RecentFilesModel *RecentFilesModel::instance()
{
    if (!s_instance) {
        s_instance = new RecentFilesModel();
    }
    return s_instance;
}

RecentFilesModel::RecentFilesModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_settings("Absokino", "Absokino")
{
    load();
}

int RecentFilesModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_files.size();
}

QVariant RecentFilesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_files.size()) {
        return QVariant();
    }

    const RecentFile &file = m_files.at(index.row());

    switch (role) {
    case PathRole:
        return file.path;
    case FileNameRole:
        return file.fileName;
    case DisplayNameRole: {
        // Remove extension for display
        QString name = file.fileName;
        int lastDot = name.lastIndexOf('.');
        if (lastDot > 0) {
            name = name.left(lastDot);
        }
        return name;
    }
    case FileSizeRole:
        return formatFileSize(file.size);
    case LastPlayedRole:
        return file.lastPlayed.toString("MMM d, h:mm AP");
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> RecentFilesModel::roleNames() const
{
    return {
        {PathRole, "path"},
        {FileNameRole, "fileName"},
        {DisplayNameRole, "displayName"},
        {FileSizeRole, "fileSize"},
        {LastPlayedRole, "lastPlayed"}
    };
}

void RecentFilesModel::addFile(const QString &path)
{
    QFileInfo info(path);
    if (!info.exists() || !info.isFile()) {
        return;
    }

    // Remove if already exists
    for (int i = 0; i < m_files.size(); ++i) {
        if (m_files[i].path == path) {
            beginRemoveRows(QModelIndex(), i, i);
            m_files.removeAt(i);
            endRemoveRows();
            break;
        }
    }

    // Add to front
    RecentFile file;
    file.path = path;
    file.fileName = info.fileName();
    file.size = info.size();
    file.lastPlayed = QDateTime::currentDateTime();

    beginInsertRows(QModelIndex(), 0, 0);
    m_files.prepend(file);
    endInsertRows();

    // Trim if too many
    while (m_files.size() > MaxRecentFiles) {
        beginRemoveRows(QModelIndex(), m_files.size() - 1, m_files.size() - 1);
        m_files.removeLast();
        endRemoveRows();
    }

    save();
    emit countChanged();
}

void RecentFilesModel::removeFile(const QString &path)
{
    for (int i = 0; i < m_files.size(); ++i) {
        if (m_files[i].path == path) {
            beginRemoveRows(QModelIndex(), i, i);
            m_files.removeAt(i);
            endRemoveRows();
            save();
            emit countChanged();
            return;
        }
    }
}

void RecentFilesModel::clearAll()
{
    if (m_files.isEmpty()) {
        return;
    }

    beginResetModel();
    m_files.clear();
    endResetModel();

    save();
    emit countChanged();
}

QString RecentFilesModel::getPath(int index) const
{
    if (index >= 0 && index < m_files.size()) {
        return m_files[index].path;
    }
    return QString();
}

void RecentFilesModel::load()
{
    beginResetModel();
    m_files.clear();

    int count = m_settings.beginReadArray("recentFiles");
    for (int i = 0; i < count && i < MaxRecentFiles; ++i) {
        m_settings.setArrayIndex(i);
        QString path = m_settings.value("path").toString();

        QFileInfo info(path);
        if (info.exists()) {
            RecentFile file;
            file.path = path;
            file.fileName = info.fileName();
            file.size = info.size();
            file.lastPlayed = m_settings.value("lastPlayed").toDateTime();
            m_files.append(file);
        }
    }
    m_settings.endArray();

    endResetModel();
    emit countChanged();
}

void RecentFilesModel::save()
{
    m_settings.beginWriteArray("recentFiles");
    for (int i = 0; i < m_files.size(); ++i) {
        m_settings.setArrayIndex(i);
        m_settings.setValue("path", m_files[i].path);
        m_settings.setValue("lastPlayed", m_files[i].lastPlayed);
    }
    m_settings.endArray();
    m_settings.sync();
}

QString RecentFilesModel::formatFileSize(qint64 bytes) const
{
    if (bytes < 1024) {
        return QString("%1 B").arg(bytes);
    } else if (bytes < 1024 * 1024) {
        return QString("%1 KB").arg(bytes / 1024.0, 0, 'f', 1);
    } else if (bytes < 1024 * 1024 * 1024) {
        return QString("%1 MB").arg(bytes / (1024.0 * 1024.0), 0, 'f', 1);
    } else {
        return QString("%1 GB").arg(bytes / (1024.0 * 1024.0 * 1024.0), 0, 'f', 2);
    }
}
