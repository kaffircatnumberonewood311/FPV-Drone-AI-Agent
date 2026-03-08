#pragma once

#ifdef NANOHAWK_WITH_GUI
#include <QWidget>

namespace nanohawk::gui {

class TelemetryPane final : public QWidget {
public:
    TelemetryPane();
};

} // namespace nanohawk::gui
#endif

