#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <QDialog>
#include <QCheckBox>
#include <QLineEdit>
#include <QRadioButton>
#include <QSpinBox>
#include <string>

using namespace std;

struct ExportSettings
{
    string profile;     // Name of the chosen export profile
    string sceneLight;  // Name of the chosen scene light
    bool sceneLeft;     // Define whether the left scene should be exported
    bool sceneRight;    // Define whether the right scene should be exported
    bool transect;      // Define whether the export concerns the transect view or the entire scene
    string filenameLeft;    // Name of the file to export
    string filenameRight;    // Name of the file to export
    bool screenshotLeft; // save also a screenshot of Ecoviz
    bool screenshotRight; // save also a screenshot of Ecoviz
    bool viewfileLeft; // save also a view file of the location
    bool viewfileRight; // save also a view file of the location
    string path;        // Path to the file to export
    int resolutionW;    // Width of the exported image
    int resolutionH;    // Height of the exported image
    int samples;        // Number of samples to use for the export
    int threads;				// Number of threads to use for the export
};

class QComboBox;
class QRadioButton;

class ExportDialog : public QDialog
{
    Q_OBJECT
public:
    ExportDialog(QStringList allProfiles, QString base_path, QWidget* parent = nullptr);

    static ExportSettings getExportSettings(QWidget* parent, QString base_path, QStringList allProfiles, bool& ok);

private:
    // base path
    QString exportPath;
    // Scene data
    QComboBox* comboboxProfiles;
    QComboBox* comboboxSceneLights;
    QCheckBox* exportLeftScene;
    QCheckBox* exportRightScene;
    QLineEdit* lineEditMitsubaSceneLeft;
    QLineEdit* lineEditMitsubaSceneRight;
    QCheckBox* exportLeftScreenshot;
    QCheckBox* exportLeftViewfile;
    QCheckBox* exportRightScreenshot;
    QCheckBox* exportRightViewfile;

    /*QRadioButton* leftSceneIndexRadioButton;
    QRadioButton* rightSceneIndexRadioButton;
    QRadioButton* entireSceneRadioButton;
    QRadioButton* onlyTransectRadioButton;*/

		// Mitsuba data
    QLineEdit* lineEditMitsubaOutputPath;
    QComboBox* comboboxMitsubaSamples;
		QSpinBox* spinBoxMitsubaResolutionW;
    QSpinBox* spinBoxMitsubaResolutionH;
    QSpinBox* spinBoxMitsubaThreads;

};

#endif // EXPORTDIALOG_H
