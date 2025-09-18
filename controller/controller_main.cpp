#include <QApplication>
#include "ControllerWindow.h"

// класс фрейминга protobuf
int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    ControllerWindow w;
    w.resize(800, 400);
    w.show();
    return app.exec();
}
