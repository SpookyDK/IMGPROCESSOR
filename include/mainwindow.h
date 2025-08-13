#pragma once

#include <QLabel>
#include <QPixmap>
#include <QMainWindow>
#include <QString>

class MyMainWindow : public QMainWindow{
    public:
        explicit MyMainWindow();
    private:
        QLabel* label;
        unsigned char* imageData;
        int width, height, channels;
        QImage qimg;
        QString FilePath;
};
