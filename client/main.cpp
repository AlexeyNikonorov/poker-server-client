#include <QApplication>
#include "kernel/kernel.h"
//#include "tables/stud/stud.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    kernel ker;
    ker.run();
    //Table::Stud stud(nullptr);
    //stud.show();
    return app.exec();
}