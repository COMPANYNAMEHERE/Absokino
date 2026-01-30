#include "trackmodel.h"

TrackModel::TrackModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int TrackModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_tracks.size();
}

QVariant TrackModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_tracks.size()) {
        return QVariant();
    }

    const Track &track = m_tracks.at(index.row());

    switch (role) {
    case IdRole:
        return track.id;
    case TitleRole:
        return track.title;
    case LangRole:
        return track.lang;
    case CodecRole:
        return track.codec;
    case DefaultRole:
        return track.isDefault;
    case ExternalRole:
        return track.isExternal;
    case SelectedRole:
        return track.selected;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> TrackModel::roleNames() const
{
    return {
        {IdRole, "trackId"},
        {TitleRole, "title"},
        {LangRole, "lang"},
        {CodecRole, "codec"},
        {DefaultRole, "isDefault"},
        {ExternalRole, "isExternal"},
        {SelectedRole, "selected"}
    };
}

void TrackModel::setTrackType(const QString &type)
{
    if (m_trackType != type) {
        m_trackType = type;
        emit trackTypeChanged();
    }
}

void TrackModel::updateTracks(const QVariantList &tracks)
{
    beginResetModel();
    m_tracks.clear();

    for (const QVariant &trackVar : tracks) {
        QVariantMap map = trackVar.toMap();
        QString type = map.value("type").toString();

        if (type != m_trackType) {
            continue;
        }

        Track track;
        track.id = map.value("id").toInt();
        track.title = map.value("title").toString();
        track.lang = map.value("lang").toString();
        track.codec = map.value("codec").toString();
        track.isDefault = map.value("default").toBool();
        track.isExternal = map.value("external").toBool();
        track.selected = map.value("selected").toBool();

        // Generate title if empty
        if (track.title.isEmpty()) {
            if (!track.lang.isEmpty()) {
                track.title = track.lang.toUpper();
            } else {
                track.title = QString("Track %1").arg(track.id);
            }
        }

        m_tracks.append(track);
    }

    endResetModel();
    emit countChanged();
}

void TrackModel::setCurrentTrack(int trackId)
{
    m_currentTrackId = trackId;

    for (int i = 0; i < m_tracks.size(); ++i) {
        bool shouldBeSelected = (m_tracks[i].id == trackId);
        if (m_tracks[i].selected != shouldBeSelected) {
            m_tracks[i].selected = shouldBeSelected;
            QModelIndex idx = index(i);
            emit dataChanged(idx, idx, {SelectedRole});
        }
    }
}

QString TrackModel::getTrackLabel(int trackId) const
{
    for (const Track &track : m_tracks) {
        if (track.id == trackId) {
            if (!track.lang.isEmpty()) {
                return track.lang.toUpper();
            }
            if (!track.title.isEmpty()) {
                return track.title;
            }
            return QString("Track %1").arg(trackId);
        }
    }
    return m_trackType == "audio" ? "Audio" : "Off";
}
