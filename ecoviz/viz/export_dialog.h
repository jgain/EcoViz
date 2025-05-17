/*******************************************************************************
 *
 * EcoViz -  a tool for visual analysis and photo‐realistic rendering of forest
 * landscape model simulations
 * Copyright (C) 2025  A. Peytavie  (adrien.peytavie@univ-lyon1.fr)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 ********************************************************************************/

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
		string pathOutput;        // Path to the file to export
    string pathResources;        // Path to the file to export
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
    ExportDialog(QString baseDir, QStringList allProfiles, QWidget* parent = nullptr);

    static ExportSettings getExportSettings(QWidget* parent, QString baseDir, QStringList allProfiles, bool& ok);

private slots:
    void selectDirectoryOutput();
    void selectDirectoryResources();

private:
    // Scene data
    QComboBox* comboboxProfiles;
		QComboBox* comboboxSceneLights;
	  QCheckBox* exportLeftScene;
		QCheckBox* exportRightScene;
    QLineEdit* lineEditMitsubaSceneLeft;
    QLineEdit* lineEditMitsubaSceneRight;

    /*QRadioButton* leftSceneIndexRadioButton;
    QRadioButton* rightSceneIndexRadioButton;
    QRadioButton* entireSceneRadioButton;
    QRadioButton* onlyTransectRadioButton;*/

		// Mitsuba data
    QLineEdit* lineEditMitsubaOutputPath;
    QLineEdit* lineEditMitsubaResourcesPath;
    QComboBox* comboboxMitsubaSamples;
		QSpinBox* spinBoxMitsubaResolutionW;
    QSpinBox* spinBoxMitsubaResolutionH;
    QSpinBox* spinBoxMitsubaThreads;

};

#endif // EXPORTDIALOG_H
