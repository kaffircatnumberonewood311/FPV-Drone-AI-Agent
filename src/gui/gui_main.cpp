#include "gui/MainWindow.hpp"

#ifdef NANOHAWK_WITH_GUI
#include <QApplication>

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    nanohawk::gui::MainWindow window;
    window.show();
    return app.exec();
}
#endif

