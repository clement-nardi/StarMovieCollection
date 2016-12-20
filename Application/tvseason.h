#ifndef TVSEASON_H
#define TVSEASON_H

#include "modelnode.h"

class TVSeason: public ModelNode
{
public:
    QString getTitleText();
    QString getDate();
    static TVSeason *getTVSeason(int TVId, int season);
private:
    TVSeason(int TVId, int season);
};

#endif // TVSEASON_H
