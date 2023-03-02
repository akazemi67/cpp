#include <QApplication>
#include <QPushButton>
#include "library.h"

int main(int argc, char *argv[]) {
    hello();
    QApplication a(argc, argv);
    QPushButton button("Hello world!", nullptr);
    button.resize(200, 100);
    button.show();
    return QApplication::exec();
}

