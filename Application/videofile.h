#ifndef MOVIEFILE_H
#define MOVIEFILE_H

#include <QObject>
#include <QFileInfo>
#include <QJsonDocument>
#include "modelnode.h"

static QStringList videoExtensions = QString("3g2,3gp,amv,asf,avi,drc,f4a,f4b,f4p,f4v,flv,gif,gifv,m1v,m2t,m2ts,m2v,m4p,m4v,mkv,mng,mod,mov,mp2,mp2v,mp4,mpe,mpeg,mpg,mpv,mpv2,mts,mxf,nsv,ogg,ogv,qt,rm,rmvb,roq,svi,ts,tts,vob,webm,wm,wmv,yuv").split(",");
/**
 * @brief The MovieFile class represents a movie file
 * It gives access to filesystem related meta-data like attached subtitle files
 * or mkv/mp4/avi meta-data (audio/subtitle streams details, duration, etc.)
 * Some information can be guessed from the name of the file or the name of the enclosing folders.
 */
class VideoFile : public ModelNode
{
    Q_OBJECT
public:
    explicit VideoFile(QString path, bool searchDataNow = true);

    QFileInfo fileInfo;
    QString getTitleText();
    QString getDate();
    QString getPath();
    QString getHtmlPath();

    static bool isVideoFile(QString filename);

    QString titleSubString; // substring of the file's complete path

    bool isTvEpisode;
    int seasonNumber;
    int episodeNumber;

private:
    QString year;
    void extractYear();

    //extracts titleSubString and isTvEpisode
    void extractInfoFromFilename();

    void extractSeasonEpisodeFromFilename();

    QString extractTitle(QString filename);
    bool searchQuerySent;

    int getLastYearOffset(QString s);

    void extractSeasonEpisodeFromMatch(const QRegularExpressionMatch &match);
    void findCounterInFilename(QFileInfo fi, QString &counter, int &nbCharsToTrim);
private slots:
    void handleResults(QJsonDocument doc);
};

#endif // MOVIEFILE_H
