#ifndef TRACKMODEL_H
#define TRACKMODEL_H

#include <QAbstractListModel>
#include <QVariantList>

/**
 * @brief TrackModel - Model for audio/subtitle track selection
 */
class TrackModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
    Q_PROPERTY(QString trackType READ trackType WRITE setTrackType NOTIFY trackTypeChanged)

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        TitleRole,
        LangRole,
        CodecRole,
        DefaultRole,
        ExternalRole,
        SelectedRole
    };

    explicit TrackModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString trackType() const { return m_trackType; }
    void setTrackType(const QString &type);

public slots:
    void updateTracks(const QVariantList &tracks);
    void setCurrentTrack(int trackId);
    QString getTrackLabel(int trackId) const;

signals:
    void countChanged();
    void trackTypeChanged();

private:
    struct Track {
        int id;
        QString title;
        QString lang;
        QString codec;
        bool isDefault;
        bool isExternal;
        bool selected;
    };

    QList<Track> m_tracks;
    QString m_trackType;  // "audio" or "sub"
    int m_currentTrackId = 0;
};

#endif // TRACKMODEL_H
