#ifndef MOVIEFILE_H
#define MOVIEFILE_H

#include <QObject>
#include <QFileInfo>
#include <QJsonDocument>
#include "video.h"

static QStringList videoExtensions = QString("3g2,3gp,amv,asf,avi,drc,f4a,f4b,f4p,f4v,flv,gif,gifv,m1v,m2t,m2ts,m2v,m4p,m4v,mkv,mng,mod,mov,mp2,mp2v,mp4,mpe,mpeg,mpg,mpv,mpv2,mts,mxf,nsv,ogg,ogv,qt,rm,rmvb,roq,svi,ts,tts,vob,webm,wm,wmv,yuv").split(",");
/**
 * @brief The MovieFile class represents a movie file
 * It gives access to filesystem related meta-data like attached subtitle files
 * or mkv/mp4/avi meta-data (audio/subtitle streams details, duration, etc.)
 * Some information can be guessed from the name of the file or the name of the enclosing folders.
 */
class VideoFile : public QObject
{
    Q_OBJECT
public:
    explicit VideoFile(QString path, int row_, bool searchDataNow = true, QObject *parent = 0);

    QFileInfo fileInfo;
    QString getTitle();
    QString year;
    Video *getVideo();
    int row;

    static bool isVideoFile(QString filename);

    QString titleSubString; // substring of the file's complete path

    bool isTvEpisode;
    int seasonNumber;
    int episodeNumber;

signals:
    void hasChanged(int);

private:
    void extractYear();

    //extracts titleSubString and isTvEpisode
    void extractInfoFromFilename();

    void extractSeasonEpisodeFromFilename();

    QString extractTitle(QString filename);
    Video *video;
    bool searchQuerySent;

    int getLastYearOffset(QString s);

    void extractSeasonEpisodeFromMatch(const QRegularExpressionMatch &match);
    void findCounterInFilename(QFileInfo fi, QString &counter, int &nbCharsToTrim);
private slots:
    void handleSearchResults(QJsonDocument doc);
    void movieHasChanged();
};

#endif // MOVIEFILE_H
