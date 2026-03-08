#include "gui/MissionPane.hpp"

#ifdef NANOHAWK_WITH_GUI
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace nanohawk::gui {

MissionPane::MissionPane() {
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("Mission preview placeholder", this));
    layout->addWidget(new QPushButton("Execute", this));
    layout->addWidget(new QPushButton("Abort", this));
}

} // namespace nanohawk::gui
#endif

