#pragma once

#ifdef NANOHAWK_WITH_GUI
#include <QMainWindow>

namespace nanohawk::gui {

class MainWindow final : public QMainWindow {
public:
    MainWindow();
};

} // namespace nanohawk::gui
#endif

