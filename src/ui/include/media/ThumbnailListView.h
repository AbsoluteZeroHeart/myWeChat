#ifndef THUMBNAILLISTVIEW
#define THUMBNAILLISTVIEW

#include "CustomListView.h"

class ThumbnailListView: public CustomListView
{
    Q_OBJECT

public:

    explicit ThumbnailListView(QWidget *parent = nullptr);
    ~ThumbnailListView();

};

#endif //CUSTOMLISTVIEW_H
