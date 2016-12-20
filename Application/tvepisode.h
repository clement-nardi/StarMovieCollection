#ifndef TVEPISODE_H
#define TVEPISODE_H

#include "modelnode.h"


class TVEpisode: public ModelNode
{
    Q_OBJECT
public:
    static TVEpisode* getTVEpisode(int TVId, int season, int episode);

    QString getTitleText();
    QString getDate();

private:
    TVEpisode(int tvId, int season, int episode);

};


#endif // TVEPISODE_H
