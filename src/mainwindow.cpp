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
#include <QDoubleSpinBox>
#include <QThread>
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
    connect(openFileAction, &QAction::triggered,this, [this,w,h](){
            QString fileName = QFileDialog::getOpenFileName(this, "Open Image File", QString(), "Images (*.jpg)");
            if (!fileName.isEmpty()) {
                        QPixmap pix(fileName);
                        if (pix.isNull()) {
                            QMessageBox::warning(this, "Error", "Could not load image!");
                        } else {
                            FilePath = fileName;
                            QByteArray byteArray = fileName.toUtf8();
                            images.clear();
                            images.push_back(Image(0, 0, 0));
                            Load_Image(byteArray.constData(), IMG_FLOAT, images.front());
                            if (imageEffects.empty()){
                                float displayRatio = (float)w / (float)h;
                                float imageRatio = (float)images.front().width / (float)images.front().height;
                                std::cout << "windows w:h = " << w << ":" << h << "\n";
                                if (displayRatio >= imageRatio){
                                    imageEffects.push_back(ImageEffect(Scale, std::vector<float>{(float)h*imageRatio,(float)h}));
                                    std::cout << "output w:h = " << h*imageRatio << ":" << h << "\n";
                                }else{
                                    imageEffects.push_back(ImageEffect(Scale, std::vector<float>{(float)w,(float)w/imageRatio}));
                                    std::cout << "output w:h = " << w << ":" << w*imageRatio << "\n";
                                }
                                layersList->addItem("Scale");

                            }

                            // Handle effects safely
                            if (!images.empty() && layersList->count() > 0) {
                                Handle_Effects(imageEffects, images, 0);
                            }

                            // Use correct stride for QImage
                            qimg = QImage(images.back().data, images.back().width, images.back().height, images.back().width * images.back().channels, QImage::Format_RGB888);
                            label->setPixmap(QPixmap::fromImage(qimg));
                        }
                    }
            });
    toolbar-> addAction(openFileAction);


    QAction* saveFileAction = new QAction("Save File", this);
    connect(saveFileAction, &QAction::triggered,this, [this](){
        QString fileName = QFileDialog::getSaveFileName(this, "Set localtion File", QString(), "Images (*.jpg)");
        if (!fileName.isEmpty()) {
            QByteArray byteArray = fileName.toUtf8();
            Export_Image(images.back(), byteArray.constData());
        }
    });
    toolbar-> addAction(saveFileAction);
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


    QPushButton* deleteButton = new QPushButton("Delete layer");
    deleteButton->setMinimumHeight(40);
    deleteButton->setStyleSheet("font-size: 18px;");
    layout->addWidget(deleteButton);


    QPushButton* scaleButton = new QPushButton("Scale layer");
    deleteButton->setMinimumHeight(40);
    deleteButton->setStyleSheet("font-size: 18px;");
    layout->addWidget(scaleButton);

    QPushButton* temperatureButton = new QPushButton("Temperature layer");
    deleteButton->setMinimumHeight(40);
    deleteButton->setStyleSheet("font-size: 18px;");
    layout->addWidget(temperatureButton);
    // Effect layers list
    layersList = new QListWidget;
    layersList->setDragDropMode(QAbstractItemView::InternalMove); // <<<<<< allows reordering!
    layersList->setSelectionMode(QAbstractItemView::SingleSelection);
    layout->addWidget(layersList);

    layout->addStretch();

    layersList->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(layersList, &QListWidget::itemClicked, this, [this](QListWidgetItem* item){
        // This item is now selected automatically by default
        auto iterator = imageEffects.begin();
        std::advance(iterator, layersList->row(item));
        qDebug() << "called";
        Set_Editor_Effect(*iterator);
        });

    connect(brightnessButton, &QPushButton::clicked, this, [this]() {
        layersList->addItem("Brightness Effect");
        imageEffects.push_back(
            ImageEffect(Effect_Type(Brightness), std::vector<float>{0})
        );
        if (images.size() > 0 && layersList) {
            Handle_Effects(imageEffects, images, 0);
            Image& lastImage = images.back();

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

    connect(temperatureButton, &QPushButton::clicked, this, [this]() {
        layersList->addItem("Temperature Effect");
        imageEffects.push_back(
            ImageEffect(Effect_Type(Temperature), std::vector<float>{0})
        );
        if (images.size() > 0 && layersList) {
            Handle_Effects(imageEffects, images, 0);
            Image& lastImage = images.back();

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
            ImageEffect(Effect_Type(Contrast), std::vector<float>{1})
        );
        if (images.size() > 0 && layersList) {
            Handle_Effects(imageEffects, images, 0);
            Image& lastImage = images.back();

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

    connect(scaleButton, &QPushButton::clicked, this, [this]() {
        layersList->addItem("Scale Effect");
        imageEffects.push_back(
            ImageEffect(Effect_Type(Scale), std::vector<float>{256,256})
        );
        if (images.size() > 0 && layersList) {
            Handle_Effects(imageEffects, images, 0);
            Image& lastImage = images.back();

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
        if (imageEffects.begin()->effect == Scale){
            layersList->insertItem(1,"Rotate Counter");
            auto itterator = imageEffects.begin();
            std::cout << "it works\n";
            ++itterator;
            imageEffects.insert(itterator, ImageEffect(Effect_Type(RotateCounterClock), std::vector<float>{1, 2, 3}));

        }else{
        layersList->insertItem(0,"Rotate Counter");
        imageEffects.push_front(
            ImageEffect(Effect_Type(RotateCounterClock), std::vector<float>{1, 2, 3})
        );}
        if (images.size() > 0 && layersList) {
            Handle_Effects(imageEffects, images, 0);
            Image& lastImage = images.back();

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
    connect(deleteButton, &QPushButton::clicked, this, [this]() {
        QListWidgetItem *selectedItem = layersList->currentItem();
        if (selectedItem) {
            // Get index of selected item
            int index = layersList->row(selectedItem);

            // Remove item from QListWidget
            delete layersList->takeItem(index);

            // Remove corresponding element from std::list
            auto it = imageEffects.begin();
            std::advance(it, index); // move iterator to the correct position
            if (it != imageEffects.end()) {
                it = imageEffects.erase(it);
                it->changed = true;
                it->imageCached = false;
            }

        if (images.size() > 0 && layersList) {
            Handle_Effects(imageEffects, images, 0);
            Image& lastImage = images.back();

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

    editorDock = new QDockWidget("Effect Editor", this);
    editorDock->setFeatures(QDockWidget::DockWidgetMovable);

    editorWidget = new QWidget;
    editorLayout = new QVBoxLayout();        // don’t pass editorWidget here
    editorWidget->setLayout(editorLayout);   // set it explicitly
    editorDock->setWidget(editorWidget);
    editorWidget->setFixedWidth(screenGeometry.width()*0.1);

    // Dock it bottom-left
    addDockWidget(Qt::LeftDockWidgetArea, editorDock);

}





void MyMainWindow::Set_Editor_Effect(ImageEffect& effect){

        qDebug() << "called";
        QLayoutItem* child;
        while ((child = editorLayout->takeAt(0)) != nullptr) {
            delete child->widget();
            delete child;
        }
        switch (effect.effect){
            case Crop : 
                break;
            case RotateClock : 
                break;
            case RotateCounterClock :
                break;
            case FlipX :
                break;

            case Temperature :{
                    QLabel* label = new QLabel("Temperature:");
                            QSlider* slider = new QSlider(Qt::Horizontal);
                            slider->setRange(-100, 100);
                            slider->setValue(static_cast<int>(effect.args[0]));
                            QDoubleSpinBox* spinBox = new QDoubleSpinBox();
                            spinBox->setValue(effect.args[0]);
                            spinBox->setRange(-1,1);
                            spinBox->setSingleStep(0.01);
                            editorLayout->addWidget(label,0,Qt::AlignTop);
                            editorLayout->addWidget(slider,0,Qt::AlignTop);
                            editorLayout->addWidget(spinBox,0,Qt::AlignTop);

                            editorLayout->addStretch();
                            connect(slider, &QSlider::sliderReleased, this, [this, &effect, slider, spinBox]() {
                                int value = slider->value();
                                spinBox->setValue(float(value)/100+0.01);

                            });

                            connect(slider, &QSlider::valueChanged, this, [this, &effect, slider, spinBox]() {
                                int value = slider->value();
                                spinBox->blockSignals(true);
                                spinBox->setValue(float(value)/100);
                                spinBox->blockSignals(false);

                            });
                                connect(spinBox, &QDoubleSpinBox::valueChanged, this, [this, &effect, slider, spinBox](){
                                float value = (float)spinBox->value();
                                slider->blockSignals(true);         // Temporarily block signals
                                slider->setValue(static_cast<int>(value * 100));
                                slider->blockSignals(false);
                                qDebug() << "changed\n";
                                qDebug() << "test = " << effect.args.size() << "\n";

                                if (effect.args.size() > 0) {
                                    effect.args[0] = value;
                                } else {
                                    qDebug() << "Error: effect has no args!";
                                }
                                effect.changed = true;
                                effect.imageCached = false;
                                Handle_Effects(imageEffects, images, 0);
                                Update_Image();
                                 });

                break;}
            case FlipY :
                break;
            case Brightness :{
                    QLabel* label = new QLabel("Brightness:");
                            QSlider* slider = new QSlider(Qt::Horizontal);
                            slider->setRange(-100, 100);
                            slider->setValue(static_cast<int>(effect.args[0]));
                            QDoubleSpinBox* spinBox = new QDoubleSpinBox();
                            spinBox->setValue(effect.args[0]);
                            spinBox->setRange(-100,100);
                            spinBox->setSingleStep(0.01);
                            editorLayout->addWidget(label,0,Qt::AlignTop);
                            editorLayout->addWidget(slider,0,Qt::AlignTop);
                            editorLayout->addWidget(spinBox,0,Qt::AlignTop);

                            editorLayout->addStretch();
                            connect(slider, &QSlider::sliderReleased, this, [this, &effect, slider, spinBox]() {
                                int value = slider->value();
                                spinBox->setValue(float(value)+1);

                            });

                            connect(slider, &QSlider::valueChanged, this, [this, &effect, slider, spinBox]() {
                                int value = slider->value();
                                spinBox->blockSignals(true);
                                spinBox->setValue(float(value));
                                spinBox->blockSignals(false);

                            });
                                connect(spinBox, &QDoubleSpinBox::valueChanged, this, [this, &effect, slider, spinBox](){
                                float value = (float)spinBox->value();

                                slider->blockSignals(true);
                                slider->setValue(static_cast<int>(value));
                                slider->blockSignals(false);
                                qDebug() << "changed\n";
                                qDebug() << "test = " << effect.args.size() << "\n";

                                if (effect.args.size() > 0) {
                                    effect.args[0] = value;
                                } else {
                                    qDebug() << "Error: effect has no args!";
                                }
                                effect.changed = true;
                                effect.imageCached = false;
                                Handle_Effects(imageEffects, images, 0);
                                Update_Image();
                                 });

                break;}
            case Contrast :{
                    QLabel* label = new QLabel("Brightness:");
                            QSlider* slider = new QSlider(Qt::Horizontal);
                            slider->setRange(0, 300);
                            slider->setValue(static_cast<int>(effect.args[0]*100));
                            QDoubleSpinBox* spinBox = new QDoubleSpinBox();
                            spinBox->setValue(effect.args[0]);
                            spinBox->setRange(0,3);
                            spinBox->setSingleStep(0.01);
                            editorLayout->addWidget(label,0,Qt::AlignTop);
                            editorLayout->addWidget(slider,0,Qt::AlignTop);
                            editorLayout->addWidget(spinBox,0,Qt::AlignTop);

                            editorLayout->addStretch();
                            connect(slider, &QSlider::sliderReleased, this, [this, &effect, slider, spinBox]() {
                                int value = slider->value();
                                spinBox->setValue(float(value)/100.0+0.1);

                            });

                            connect(slider, &QSlider::valueChanged, this, [this, &effect, slider, spinBox]() {
                                int value = slider->value();
                                spinBox->blockSignals(true);
                                spinBox->setValue(float(value)/100);
                                spinBox->blockSignals(false);

                            });
                                connect(spinBox, &QDoubleSpinBox::valueChanged, this, [this, &effect, slider, spinBox](){
                                float value = (float)spinBox->value();
                                slider->setValue(static_cast<int>(value*100));
                                qDebug() << "changed\n";
                                qDebug() << "test = " << effect.args.size() << "\n";

                                if (effect.args.size() > 0) {
                                    effect.args[0] = value;
                                } else {
                                    qDebug() << "Error: effect has no args!";
                                }
                                effect.changed = true;
                                effect.imageCached = false;
                                Handle_Effects(imageEffects, images, 0);
                                Update_Image();
                                 });

                break;}
                           
            case Saturation :
                break;
            case Vibrancy : 
                break;
        }
}


void MyMainWindow::Update_Image() {
    QThread* workerThread = QThread::create([this] {
        {
            QMutexLocker locker(&imagesMutex);
            Handle_Effects(imageEffects, images, 0);
        }

        QMetaObject::invokeMethod(this, [this] {
            QMutexLocker locker(&imagesMutex);

            if (!images.empty()) {
                Image& lastImage = images.back();

                qimg = QImage(
                    lastImage.data,
                    lastImage.width,
                    lastImage.height,
                    lastImage.width * lastImage.channels,
                    QImage::Format_RGB888
                );
                qimg = qimg.copy();
                label->setPixmap(QPixmap::fromImage(qimg));
                label->update();
            } else {
                qDebug() << "MyMainWindow::Update_Image()__no images to update\n";
            }
        }, Qt::QueuedConnection);
    });

    workerThread->start();
}
