#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <QDialog>
#include <string>

using namespace std;

struct ExportSettings
{
    string profile;     // Name of the chosen export profile
    bool transect;      // Define whether the export concerns the transect view or the entire scene
    int sceneIndex;     // Index of the scene/transect to export
};

class QComboBox;
class QRadioButton;

class ExportDialog : public QDialog
{
    Q_OBJECT
public:
    ExportDialog(QStringList allProfiles, QWidget* parent = nullptr);

    static ExportSettings getExportSettings(QWidget* parent, QStringList allProfiles, bool* ok = nullptr);

private:
    QComboBox* comboboxProfiles;
    QRadioButton* leftSceneIndexRadioButton;
    QRadioButton* rightSceneIndexRadioButton;
    QRadioButton* entireSceneRadioButton;
    QRadioButton* onlyTransectRadioButton;
};

#endif // EXPORTDIALOG_H
