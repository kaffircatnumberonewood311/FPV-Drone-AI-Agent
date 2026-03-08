#include "gui/MainWindow.hpp"

#ifdef NANOHAWK_WITH_GUI
#include "gui/VideoPane.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

namespace nanohawk::gui {

MainWindow::MainWindow() {
    setWindowTitle("Nanohawk Agent");
    resize(1200, 720);

    auto* central = new QWidget(this);
    auto* layout = new QHBoxLayout(central);

    auto* videoPane = new VideoPane();
    layout->addWidget(videoPane, 3);

    auto* sidePanel = new QWidget(central);
    auto* sideLayout = new QVBoxLayout(sidePanel);
    sideLayout->addWidget(new QLabel("Telemetry", sidePanel));
    sideLayout->addWidget(new QLabel("Prompt", sidePanel));
    sideLayout->addWidget(new QLabel("Mission", sidePanel));
    sideLayout->addWidget(new QLabel("Safety", sidePanel));
    sideLayout->addStretch();

    layout->addWidget(sidePanel, 1);
    setCentralWidget(central);
}

} // namespace nanohawk::gui
#endif

