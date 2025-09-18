#include <QApplication>
#include "AgentWindow.h"

// Подключать сигналы сокета один раз
int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    AgentWindow w;
    w.resize(600, 120);
    w.show();
    return app.exec();
}
