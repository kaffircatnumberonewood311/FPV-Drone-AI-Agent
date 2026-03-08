#pragma once

#ifdef NANOHAWK_WITH_GUI
#include <QWidget>

namespace nanohawk::gui {

class PromptPane final : public QWidget {
public:
    PromptPane();
};

} // namespace nanohawk::gui
#endif

