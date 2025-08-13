#include "mainwindow.h"
#include <QScreen>
#include <QGuiApplication>
#include <QToolBar>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include "image_functions.h"
#include "iostream"

MyMainWindow::MyMainWindow() : QMainWindow(){
    label = new QLabel;
    label->setAlignment(Qt::AlignCenter);

    // Get primary screen size and calculate 80%
    QScreen* screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int w = static_cast<int>(screenGeometry.width() * 0.8);
    int h = static_cast<int>(screenGeometry.height() * 0.8);

    // Set placeholder pixmap
    QPixmap defaultPixmap(w, h);
    defaultPixmap.fill(Qt::gray);
    label->setPixmap(defaultPixmap);

    setCentralWidget(label);


    QToolBar* toolbar = new QToolBar("Main Toolbar", this);
    QAction* openFileAction = new QAction("Open File", this);
    connect(openFileAction, &QAction::triggered,this, [this](){
            QString fileName = QFileDialog::getOpenFileName(this, "Open Image File", QString(), "Images (*.jpg)");
            if (!fileName.isEmpty()) {
                        QPixmap pix(fileName);
                        if (pix.isNull()) {
                            QMessageBox::warning(this, "Error", "Could not load image!");
                        } else {
                            FilePath = fileName;
                            QByteArray byteArray = fileName.toUtf8();
                            imageData = Load_Image(byteArray.constData(), width, height, channels);
                            qimg = QImage(imageData, width, height, width * channels, QImage::Format_RGB888);
                            std::cout << "testststst";
                            label->setPixmap(QPixmap::fromImage(qimg));
                        }
                    }
            });
    toolbar-> addAction(openFileAction);
    addToolBar(toolbar);


}


