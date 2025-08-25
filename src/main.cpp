#include <QApplication>
#include <QPushButton>
#include <QLabel>
#include <QPixmap>
#include <numbers>
#include <iostream>
#include <iostream>
#include <chrono>
#include "mainwindow.h"
#include "image_functions.h"
#include <QPalette>
#include <QStyleFactory>
#include <stdio.h>
#include <cudahelper.h>


int main(int argc, char *argv[]) {
    int nCudaDevices = getCudaDeviceCount();
    std::cout << "cuda devices = " << nCudaDevices << "\n";
    QApplication app(argc, argv);
    QApplication::setStyle(QStyleFactory::create("Fusion"));

    // Create and set dark palette
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(42, 42, 42));
    darkPalette.setColor(QPalette::AlternateBase, QColor(66, 66, 66));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);

    app.setPalette(darkPalette);
    MyMainWindow window;
    window.show();
    return app.exec();

}
