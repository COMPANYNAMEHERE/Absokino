#ifndef CHAPTERMODEL_H
#define CHAPTERMODEL_H

#include <QAbstractListModel>
#include <QVariantList>

/**
 * @brief ChapterModel - Model for chapter navigation
 */
class ChapterModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
    Q_PROPERTY(int currentChapter READ currentChapter WRITE setCurrentChapter NOTIFY currentChapterChanged)

public:
    enum Roles {
        IndexRole = Qt::UserRole + 1,
        TitleRole,
        TimeRole,
        TimeStringRole,
        CurrentRole
    };

    explicit ChapterModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    int currentChapter() const { return m_currentChapter; }
    void setCurrentChapter(int chapter);

public slots:
    void updateChapters(const QVariantList &chapters);

signals:
    void countChanged();
    void currentChapterChanged();

private:
    QString formatTime(double seconds) const;

    struct Chapter {
        int index;
        QString title;
        double time;
    };

    QList<Chapter> m_chapters;
    int m_currentChapter = -1;
};

#endif // CHAPTERMODEL_H
