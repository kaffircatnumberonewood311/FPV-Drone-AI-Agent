#pragma once

#ifdef NANOHAWK_WITH_GUI
#include <QWidget>

namespace nanohawk::gui {

class SafetyPane final : public QWidget {
public:
    SafetyPane();
};

} // namespace nanohawk::gui
#endif

