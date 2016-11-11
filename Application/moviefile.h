#ifndef MOVIEFILE_H
#define MOVIEFILE_H

#include <QObject>
#include <QFileInfo>

static QStringList videoExtensions = QString("3g2,3gp,amv,asf,avi,drc,f4a,f4b,f4p,f4v,flv,gif,gifv,m1v,m2t,m2ts,m2v,m4p,m4v,mkv,mng,mod,mov,mp2,mp2v,mp4,mpe,mpeg,mpg,mpv,mpv2,mts,mxf,nsv,ogg,ogv,qt,rm,rmvb,roq,svi,ts,tts,vob,webm,wm,wmv,yuv").split(",");
/**
 * @brief The MovieFile class represents a movie file
 * It gives access to filesystem related meta-data like attached subtitle files
 * or mkv/mp4/avi meta-data (audio/subtitle streams details, duration, etc.)
 * Some information can be guessed from the name of the file or the name of the enclosing folders.
 */
class MovieFile : public QObject
{
    Q_OBJECT
public:
    explicit MovieFile(QString path, QObject *parent = 0);

    QFileInfo fileInfo;

    QString extractTitleFromFilename();
    QString extractAlternateTitle();
    QString getYear();

    QString prettyName();

    static bool isMovieFile(QString filename);

signals:

public slots:
};

#endif // MOVIEFILE_H
