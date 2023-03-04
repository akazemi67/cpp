#include "chatwindow.h"
#include "logging.h"

#include <QApplication>
#include <QGuiApplication>
#include <QScreen>

int main(int argc, char *argv[]) {
    init_logging();
    QApplication a(argc, argv);
    ChatWindow w;
    QRect screenGeometry = QGuiApplication::primaryScreen()->geometry();
    int x = (screenGeometry.width()-w.width()) / 2;
    int y = (screenGeometry.height()-w.height()) / 2;
    w.move(x,y);
    logger->info("Starting chat gui in position ({},{})", x, y);
    w.show();
    return a.exec();
}
