#include "chaptermodel.h"
#include <cmath>

ChapterModel::ChapterModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int ChapterModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_chapters.size();
}

QVariant ChapterModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_chapters.size()) {
        return QVariant();
    }

    const Chapter &chapter = m_chapters.at(index.row());

    switch (role) {
    case IndexRole:
        return chapter.index;
    case TitleRole:
        return chapter.title.isEmpty() ? QString("Chapter %1").arg(chapter.index + 1) : chapter.title;
    case TimeRole:
        return chapter.time;
    case TimeStringRole:
        return formatTime(chapter.time);
    case CurrentRole:
        return chapter.index == m_currentChapter;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> ChapterModel::roleNames() const
{
    return {
        {IndexRole, "chapterIndex"},
        {TitleRole, "title"},
        {TimeRole, "time"},
        {TimeStringRole, "timeString"},
        {CurrentRole, "isCurrent"}
    };
}

void ChapterModel::setCurrentChapter(int chapter)
{
    if (m_currentChapter != chapter) {
        int oldChapter = m_currentChapter;
        m_currentChapter = chapter;

        // Update old chapter
        if (oldChapter >= 0 && oldChapter < m_chapters.size()) {
            QModelIndex idx = index(oldChapter);
            emit dataChanged(idx, idx, {CurrentRole});
        }

        // Update new chapter
        if (chapter >= 0 && chapter < m_chapters.size()) {
            QModelIndex idx = index(chapter);
            emit dataChanged(idx, idx, {CurrentRole});
        }

        emit currentChapterChanged();
    }
}

void ChapterModel::updateChapters(const QVariantList &chapters)
{
    beginResetModel();
    m_chapters.clear();

    int idx = 0;
    for (const QVariant &chapterVar : chapters) {
        QVariantMap map = chapterVar.toMap();

        Chapter chapter;
        chapter.index = idx++;
        chapter.title = map.value("title").toString();
        chapter.time = map.value("time").toDouble();

        m_chapters.append(chapter);
    }

    endResetModel();
    emit countChanged();
}

QString ChapterModel::formatTime(double seconds) const
{
    if (std::isnan(seconds) || seconds < 0) {
        return "--:--";
    }

    int totalSeconds = static_cast<int>(seconds);
    int hours = totalSeconds / 3600;
    int minutes = (totalSeconds % 3600) / 60;
    int secs = totalSeconds % 60;

    if (hours > 0) {
        return QString("%1:%2:%3")
            .arg(hours)
            .arg(minutes, 2, 10, QChar('0'))
            .arg(secs, 2, 10, QChar('0'));
    } else {
        return QString("%1:%2")
            .arg(minutes)
            .arg(secs, 2, 10, QChar('0'));
    }
}
