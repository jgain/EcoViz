#ifndef HELP_DIALOG_H
#define HELP_DIALOG_H

#include <QDialog>
#include <QTextEdit>
#include <QVBoxLayout>

class HelpDialog : public QDialog {
    Q_OBJECT

public:
    HelpDialog(QWidget* parent = nullptr) : QDialog(parent) {
        setWindowTitle("Controls Help");
        setModal(false);
        resize(400, 500);

        QTextEdit* helpText = new QTextEdit(this);
        helpText->setReadOnly(true);
        helpText->setHtml(
            "<h2>Keyboard Controls</h2>"
            "<ul>"
            "<li><b>F2:</b> Toggle between Orbit and Flyover view modes.</li>"
            "<li><b>W/Up Arrow:</b> Fly forward (Flyover mode).</li>"
            "<li><b>S/Down Arrow:</b> Fly backward (Flyover mode).</li>"
            "<li><b>A/Left Arrow:</b> Fly left (Flyover mode).</li>"
            "<li><b>D/Right Arrow:</b> Fly right (Flyover mode).</li>"
            "<li><b>Page Up:</b> Increase altitude (Flyover mode).</li>"
            "<li><b>Page Down:</b> Decrease altitude (Flyover mode).</li>"
            "<li><b>R:</b> Reset flyover view.</li>"
            "<li><b>V:</b> Top-down view (Orbit mode).</li>"
            "<li><b>F:</b> Toggle focus stick visibility (Orbit mode).</li>"
            "<li><b>M:</b> Toggle minimap visibility.</li>"
            "<li><b>Ctrl + Mouse Wheel:</b> Adjust transect width.</li>"
            "</ul>"
            "<h2>Mouse Controls</h2>"
            "<ul>"
            "<li><b>Right Mouse Button + Drag:</b> Rotate view (Orbit mode) or look around (Flyover mode).</li>"
            "<li><b>Mouse Wheel:</b> Zoom in/out (Orbit mode) or fly forward/backward (Flyover mode).</li>"
            "<li><b>Double-Click:</b> Set focus point (Orbit mode).</li>"
            "<li><b>Ctrl + Left-Click:</b> Place transect point.</li>"
            "</ul>"
        );

        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->addWidget(helpText);
    }
};

#endif // HELP_DIALOG_H
