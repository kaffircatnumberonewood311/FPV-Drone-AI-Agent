#include "gui/SafetyPane.hpp"

#ifdef NANOHAWK_WITH_GUI
#include <QLabel>
#include <QVBoxLayout>

namespace nanohawk::gui {

SafetyPane::SafetyPane() {
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("Safety status placeholder", this));
}

} // namespace nanohawk::gui
#endif

