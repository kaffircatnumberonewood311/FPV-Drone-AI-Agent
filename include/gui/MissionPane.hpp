#pragma once

#ifdef NANOHAWK_WITH_GUI
#include <QWidget>

namespace nanohawk::gui {

class MissionPane final : public QWidget {
public:
    MissionPane();
};

} // namespace nanohawk::gui
#endif

