#include "gui/TelemetryPane.hpp"

#ifdef NANOHAWK_WITH_GUI
#include <QLabel>
#include <QVBoxLayout>

namespace nanohawk::gui {

TelemetryPane::TelemetryPane() {
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("Telemetry placeholder", this));
}

} // namespace nanohawk::gui
#endif

