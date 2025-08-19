#include "mainwindow.h"
#include <QScreen>
#include <QGuiApplication>
#include <QToolBar>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QDockWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QListWidget>
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
                            images.clear();
                            int width = 0, height = 0, channels = 0;
                            unsigned char* data = Load_Image(byteArray.constData(), width, height, channels);

                            // Force 3 channels for safety
                            const int channelsRGB = 3;

                            images.push_back(Image(data, width, height, channelsRGB));

                            // Handle effects safely
                            if (!images.empty() && layersList->count() > 0) {
                                Handle_Effects(imageEffects, images, 0);
                            }

                            // Use correct stride for QImage
                            qimg = QImage(images.back().data, images.back().width, images.back().height, images.back().width * images.back().channels, QImage::Format_RGB888);
                            std::cout << "testststst";
                            label->setPixmap(QPixmap::fromImage(qimg));
                        }
                    }
            });
    toolbar-> addAction(openFileAction);
    toolbar->setIconSize(QSize(32,32));
    addToolBar(toolbar);
    // QDockWidget* dock = new QDockWidget("Sidebar", this);
    // dock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    // dock->setMinimumWidth(300);
    // addDockWidget(Qt::RightDockWidgetArea, dock);
    QDockWidget* dock = new QDockWidget("Effect Layers", this);
    dock->setFeatures(QDockWidget::DockWidgetMovable); // allow moving sidebar

    // Sidebar content
    QWidget* sidebarWidget = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout(sidebarWidget);

    // Add button at the top (optional)
    QPushButton* brightnessButton = new QPushButton("bright Layer");
    brightnessButton->setMinimumHeight(40);
    brightnessButton->setStyleSheet("font-size: 18px;");
    layout->addWidget(brightnessButton);

    QPushButton* contrastButton = new QPushButton("contrast Layer");
    contrastButton->setMinimumHeight(40);
    contrastButton->setStyleSheet("font-size: 18px;");
    layout->addWidget(contrastButton);

    QPushButton* rotateCButton = new QPushButton("Rotate Layer");
    contrastButton->setMinimumHeight(40);
    contrastButton->setStyleSheet("font-size: 18px;");
    layout->addWidget(rotateCButton);
    // Effect layers list
    layersList = new QListWidget;
    layersList->setDragDropMode(QAbstractItemView::InternalMove); // <<<<<< allows reordering!
    layersList->setSelectionMode(QAbstractItemView::SingleSelection);
    layout->addWidget(layersList);

    layout->addStretch();

    connect(brightnessButton, &QPushButton::clicked, this, [this]() {
        layersList->addItem("Brightness Effect");
        imageEffects.push_back(
            ImageEffect(Effect_Type(Brightness), std::vector<float>{1, 2, 3})
        );
        if (images.size() > 0 && layersList) {
            Handle_Effects(imageEffects, images, 0);
            Image& lastImage = images.back();
            std::cout << "check im image update" << lastImage.data[1] << "\n";

            qimg = QImage(
                lastImage.data,
                lastImage.width,
                lastImage.height,
                lastImage.width * lastImage.channels, // bytes per line, NOT width*height*channels
                QImage::Format_RGB888
            );
            qimg = qimg.copy();
            label->setPixmap(QPixmap::fromImage(qimg));
            label->update();
        }
    });
    connect(contrastButton, &QPushButton::clicked, this, [this]() {
        layersList->addItem("Contrast Effect");
        imageEffects.push_back(
            ImageEffect(Effect_Type(Contrast), std::vector<float>{1, 2, 3})
        );
        if (images.size() > 0 && layersList) {
            Handle_Effects(imageEffects, images, 0);
            Image& lastImage = images.back();
            std::cout << "check im image update" << lastImage.data[1] << "\n";

            qimg = QImage(
                lastImage.data,
                lastImage.width,
                lastImage.height,
                lastImage.width * lastImage.channels, // bytes per line, NOT width*height*channels
                QImage::Format_RGB888
            );
            qimg = qimg.copy();
            label->setPixmap(QPixmap::fromImage(qimg));
            label->update();
        }
    });

    connect(rotateCButton, &QPushButton::clicked, this, [this]() {
        layersList->addItem("Rotate Counter");
        imageEffects.push_back(
            ImageEffect(Effect_Type(RotateCounterClock), std::vector<float>{1, 2, 3})
        );
        if (images.size() > 0 && layersList) {
            Handle_Effects(imageEffects, images, 0);
            Image& lastImage = images.back();
            std::cout << "check im image update" << lastImage.data[1] << "\n";

            qimg = QImage(
                lastImage.data,
                lastImage.width,
                lastImage.height,
                lastImage.width * lastImage.channels, // bytes per line, NOT width*height*channels
                QImage::Format_RGB888
            );
            qimg = qimg.copy();
            label->setPixmap(QPixmap::fromImage(qimg));
            label->update();
        }
    });
connect(layersList->model(), &QAbstractItemModel::rowsMoved, this,
    [this](const QModelIndex&, int start, int end,
           const QModelIndex&, int destinationRow) {

        if (start == end) {
            auto fromIt = std::next(imageEffects.begin(), start);
            auto toIt   = std::next(imageEffects.begin(), destinationRow);

            if (destinationRow > start)
                ++toIt;
            
            fromIt->changed = true;
            fromIt->imageCached = false;
            if (destinationRow > start) {
                for (auto it = std::next(imageEffects.begin(), start + 1); 
                     it != imageEffects.end(); ++it) {
                    it->changed = true;
                }
            }

            imageEffects.splice(toIt, imageEffects, fromIt);
        }
        Print_Effects(imageEffects);
        if (images.size() > 0 && layersList) {
            Handle_Effects(imageEffects, images, 0);
            Image& lastImage = images.back();
            std::cout << "check im image update" << lastImage.data[1] << "\n";

            qimg = QImage(
                lastImage.data,
                lastImage.width,
                lastImage.height,
                lastImage.width * lastImage.channels, // bytes per line, NOT width*height*channels
                QImage::Format_RGB888
            );
            qimg = qimg.copy();
            label->setPixmap(QPixmap::fromImage(qimg));
            label->update();
        }
    });

    dock->setWidget(sidebarWidget);
    addDockWidget(Qt::RightDockWidgetArea, dock);

}


