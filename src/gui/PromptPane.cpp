#include "gui/PromptPane.hpp"

#ifdef NANOHAWK_WITH_GUI
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

namespace nanohawk::gui {

PromptPane::PromptPane() {
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(new QTextEdit(this));
    layout->addWidget(new QPushButton("Compile Mission", this));
}

} // namespace nanohawk::gui
#endif

