#include <QApplication>
#include <QPushButton>
#include <QLabel>
#include <QPixmap>
#include <numbers>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QLabel label;
    QPixmap pixmap("../test.bmp");
    if (!pixmap.isNull()){
        label.setPixmap(pixmap);
        label.show();
    } else {
        qDebug() << "Failed to load image";
    }
    return app.exec();
}
