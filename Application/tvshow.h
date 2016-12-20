#ifndef TVSHOW_H
#define TVSHOW_H
#include "modelnode.h"

class TVShow : public ModelNode
{
public:
    static TVShow *getTVShow(int tvId);
    QString getTitleText();
    QString getDate();
private:
    TVShow(int tvId);
};

#endif // TVSHOW_H
