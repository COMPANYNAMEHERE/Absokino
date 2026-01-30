#ifndef RECENTFILESMODEL_H
#define RECENTFILESMODEL_H

#include <QAbstractListModel>
#include <QSettings>
#include <QFileInfo>

/**
 * @brief RecentFilesModel - Model for the Library drawer showing recent files
 */
class RecentFilesModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
    enum Roles {
        PathRole = Qt::UserRole + 1,
        FileNameRole,
        DisplayNameRole,
        FileSizeRole,
        LastPlayedRole
    };

    static RecentFilesModel *instance();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

public slots:
    void addFile(const QString &path);
    void removeFile(const QString &path);
    void clearAll();
    QString getPath(int index) const;

signals:
    void countChanged();

private:
    explicit RecentFilesModel(QObject *parent = nullptr);
    ~RecentFilesModel() override = default;

    void load();
    void save();
    QString formatFileSize(qint64 bytes) const;

    struct RecentFile {
        QString path;
        QString fileName;
        qint64 size;
        QDateTime lastPlayed;
    };

    static RecentFilesModel *s_instance;
    QList<RecentFile> m_files;
    QSettings m_settings;
    static constexpr int MaxRecentFiles = 50;
};

#endif // RECENTFILESMODEL_H
