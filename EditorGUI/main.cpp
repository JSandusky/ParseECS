#include "editorgui.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    EditorGUI w;
    w.show();
    return a.exec();
}
