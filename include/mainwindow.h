#pragma once

#include <QLabel>
#include <QPixmap>
#include <QMainWindow>
#include <QString>
#include "image_functions.h"
#include <vector>
#include <list>
#include <QListWidget>

class MyMainWindow : public QMainWindow{
    public:
        explicit MyMainWindow();
    private:
        QLabel* label;
        std::vector<Image> images;
        QImage qimg;
        QString FilePath;
        std::list<ImageEffect> imageEffects;
        QListWidget* layersList;

};
