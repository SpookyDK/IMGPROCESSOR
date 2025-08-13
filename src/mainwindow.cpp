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
                            imageData = Load_Image(byteArray.constData(), width, height, channels);
                            qimg = QImage(imageData, width, height, width * channels, QImage::Format_RGB888);
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
    QPushButton* addButton = new QPushButton("Add Layer");
    addButton->setMinimumHeight(40);
    addButton->setStyleSheet("font-size: 18px;");
    layout->addWidget(addButton);

    // Effect layers list
    QListWidget* layersList = new QListWidget;
    layersList->setDragDropMode(QAbstractItemView::InternalMove); // <<<<<< allows reordering!
    layersList->setSelectionMode(QAbstractItemView::SingleSelection);
    layout->addWidget(layersList);

    layout->addStretch();
    connect(addButton, &QPushButton::clicked, this, [layersList]() {
        layersList->addItem("New Effect Layer");
    });
    dock->setWidget(sidebarWidget);
    addDockWidget(Qt::RightDockWidgetArea, dock);

}


