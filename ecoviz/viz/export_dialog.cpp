#include "export_dialog.h"

#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QComboBox>
#include <QSpinBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QPushButton>
#include <QFileDialog>



ExportDialog::ExportDialog(QString baseDir, QStringList allProfiles, QWidget* parent) : QDialog(parent)
{
    setWindowTitle("Export settings");

    QVBoxLayout* layoutAll = new QVBoxLayout(this);

    // Scene settings
		QGroupBox* groupBoxScene = new QGroupBox("Scene data settings", this);
    QGridLayout* layoutScene = new QGridLayout(this);

    // - Scene lights
		/*QLabel* comboboxLightsLabel = new QLabel(QString("Scene lights"), this);
		layoutScene->addWidget(comboboxLightsLabel, 0, 0);

    comboboxSceneLights = new QComboBox(this);
    comboboxSceneLights->addItem("Daylight");
    comboboxSceneLights->addItem("Nightlight");
		layoutScene->addWidget(comboboxSceneLights, 0, 1);*/

    // - Biome Profile selection
    QLabel* comboboxProfilesLabel = new QLabel(QString("Biome profile"), this);
		layoutScene->addWidget(comboboxProfilesLabel, 1, 0);
    
    comboboxProfiles = new QComboBox(this);
    for (QString profile : allProfiles)
    {
        comboboxProfiles->addItem(profile);
    }
		layoutScene->addWidget(comboboxProfiles, 1, 1);

    {
      // - Left Scene
      QGroupBox* groupLeftScene = new QGroupBox("Left scene", this);
      layoutScene->addWidget(groupLeftScene, 2, 0);

      QGridLayout* layoutLeft = new QGridLayout(this);

      // -- Export left scene ?
      exportLeftScene = new  QCheckBox("Export left scene", this);
      exportLeftScene->setChecked(true);
      layoutLeft->addWidget(exportLeftScene, 0, 0, 1, 2);

      // -- Biome profile
      /*QLabel* biomeFramesL = new QLabel(QString("Biome profile"), this);
      layoutLeft->addWidget(biomeFramesL, 1, 0);

      QSpinBox* biomeFramesSpinBoxMinL = new QSpinBox(this);
      biomeFramesSpinBoxMinL->setRange(0, 1000);
      biomeFramesSpinBoxMinL->setValue(0);
      layoutLeft->addWidget(biomeFramesSpinBoxMinL, 1, 1);

      QSpinBox* biomeFramesSpinBoxMaxL = new QSpinBox(this);
      biomeFramesSpinBoxMaxL->setRange(0, 1000);
      biomeFramesSpinBoxMaxL->setValue(0);
      layoutLeft->addWidget(biomeFramesSpinBoxMaxL, 1, 2);*/

      // - Mitsuba scene path
      QLabel* labelSceneLeftName = new QLabel(QString("Scene name"), this);
      layoutLeft->addWidget(labelSceneLeftName, 1, 0);

      lineEditMitsubaSceneLeft = new QLineEdit(this);
      layoutLeft->addWidget(lineEditMitsubaSceneLeft,1,1);
      lineEditMitsubaSceneLeft->setText("sceneLeft");

      groupLeftScene->setLayout(layoutLeft);
    }

    {
      // - Right Scene
      QGroupBox* groupRightScene = new QGroupBox("Right scene", this);
      layoutScene->addWidget(groupRightScene, 2, 1);

      QGridLayout* layoutRight = new QGridLayout(this);

      // -- Export right scene ?
      exportRightScene = new  QCheckBox("Export right scene", this);
      exportRightScene->setChecked(true);
      layoutRight->addWidget(exportRightScene, 0, 0, 1, 3);

      // -- Biome profile
      /*QLabel* biomeFramesR = new QLabel(QString("Biome profile"), this);
      layoutRight->addWidget(biomeFramesR, 1, 0);

      QSpinBox* biomeFramesSpinBoxMinR = new QSpinBox(this);
      biomeFramesSpinBoxMinR->setRange(0, 1000);
      biomeFramesSpinBoxMinR->setValue(0);
      layoutRight->addWidget(biomeFramesSpinBoxMinR, 1, 1);

      QSpinBox* biomeFramesSpinBoxMaxR = new QSpinBox(this);
      biomeFramesSpinBoxMaxR->setRange(0, 1000);
      biomeFramesSpinBoxMaxR->setValue(0);
      layoutRight->addWidget(biomeFramesSpinBoxMaxR, 1, 2);*/

      // - Mitsuba scene path
      QLabel* labelSceneLeftName = new QLabel(QString("Scene name"), this);
      layoutRight->addWidget(labelSceneLeftName, 1, 0);

      lineEditMitsubaSceneRight = new QLineEdit(this);
      layoutRight->addWidget(lineEditMitsubaSceneRight, 1, 1);
      lineEditMitsubaSceneRight->setText("sceneRight");

      groupRightScene->setLayout(layoutRight);
    }

		groupBoxScene->setLayout(layoutScene);
		layoutAll->addWidget(groupBoxScene);

		// Mitsuba settings form
    QGroupBox* groupBoxMitsuba = new QGroupBox("Mitsuba settings", this);
    QFormLayout* layoutMitsuba = new QFormLayout(this);

		// - Mitsuba output path
		lineEditMitsubaOutputPath = new QLineEdit(this);
		layoutMitsuba->addRow("Output path", lineEditMitsubaOutputPath);
    lineEditMitsubaOutputPath->setText(baseDir ); // JG PATH FIX REQUIRED
   
    QPushButton* buttonBrowseOutput = new QPushButton("Browse...", this);
    layoutMitsuba->addRow(buttonBrowseOutput);
    connect(buttonBrowseOutput, &QPushButton::clicked, this, &ExportDialog::selectDirectoryOutput);

    // - Mitsuba resources path
    lineEditMitsubaResourcesPath = new QLineEdit(this);
    layoutMitsuba->addRow("Resources path", lineEditMitsubaResourcesPath);
    lineEditMitsubaResourcesPath->setText("../../resources/mitsuba"); // JG PATH FIX REQUIRED

    QPushButton* buttonBrowseResources = new QPushButton("Browse...", this);
    layoutMitsuba->addRow(buttonBrowseResources);
    connect(buttonBrowseResources, &QPushButton::clicked, this, &ExportDialog::selectDirectoryResources);

		// - Mitsuba resolution width
		spinBoxMitsubaResolutionW = new QSpinBox(this);
		spinBoxMitsubaResolutionW->setRange(1, 10000);
		spinBoxMitsubaResolutionW->setValue(720);
		layoutMitsuba->addRow("Resolution Width", spinBoxMitsubaResolutionW);

		// - Mitsuba resolution height
		spinBoxMitsubaResolutionH = new QSpinBox(this);
		spinBoxMitsubaResolutionH->setRange(1, 10000);
		spinBoxMitsubaResolutionH->setValue(720);
		layoutMitsuba->addRow("Resolution Height", spinBoxMitsubaResolutionH);

		// - Mitsuba samples
		comboboxMitsubaSamples = new QComboBox(this);
		comboboxMitsubaSamples->addItem("8");
    comboboxMitsubaSamples->addItem("16");
    comboboxMitsubaSamples->addItem("32");
    comboboxMitsubaSamples->addItem("64");
    comboboxMitsubaSamples->addItem("128");
		comboboxMitsubaSamples->addItem("256");
		layoutMitsuba->addRow("Mitsuba samples", comboboxMitsubaSamples);

		groupBoxMitsuba->setLayout(layoutMitsuba);
		layoutAll->addWidget(groupBoxMitsuba);


    // - Mitsuba threads
    spinBoxMitsubaThreads = new QSpinBox(this);
    spinBoxMitsubaThreads->setRange(1, 100);
    spinBoxMitsubaThreads->setValue(16);
    layoutMitsuba->addRow("Threads", spinBoxMitsubaThreads);


    /*
    // Scene index selection
    QHBoxLayout* hboxLayoutSceneIndex = new QHBoxLayout;


    entireSceneRadioButton = new QRadioButton("Entire scene");
    entireSceneRadioButton->setChecked(true);
    hboxLayoutSceneIndex->addWidget(entireSceneRadioButton);
    buttonGroup->addButton(entireSceneRadioButton);

    onlyTransectRadioButton = new QRadioButton("Only transect");
    hboxLayoutSceneIndex->addWidget(onlyTransectRadioButton);
    buttonGroup->addButton(onlyTransectRadioButton);

    QLabel* exportSelectionLabel = new QLabel(QString("Export selection:"), this);
    layout->addRow(exportSelectionLabel, hboxLayoutSceneIndex);*/

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    
    layoutAll->addWidget(buttonBox);

    bool conn = connect(buttonBox, &QDialogButtonBox::accepted, this, &ExportDialog::accept);
    Q_ASSERT(conn);
    conn = connect(buttonBox, &QDialogButtonBox::rejected, this, &ExportDialog::reject);
    Q_ASSERT(conn);

    setLayout(layoutAll);
}

void ExportDialog::selectDirectoryOutput() {
  QString dir = QFileDialog::getExistingDirectory(this, "Select Directory", lineEditMitsubaOutputPath->text(),
    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  if (!dir.isEmpty()) {
    lineEditMitsubaOutputPath->setText(dir);
  }
}


void ExportDialog::selectDirectoryResources() {
  QString toDir = QFileDialog::getExistingDirectory(this, "Select Directory", lineEditMitsubaResourcesPath->text(),
    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  if (!toDir.isEmpty()) {
    lineEditMitsubaResourcesPath->setText(toDir+"/");
  }
}

ExportSettings ExportDialog::getExportSettings(QWidget* parent, QString baseDir, QStringList allProfiles, bool& ok)
{
    ExportSettings ret;
    ExportDialog* dialog = new ExportDialog(baseDir, allProfiles, parent);

    if (dialog->exec())
    {
      ok = true;

      ret.profile = dialog->comboboxProfiles->currentText().toStdString();
      ret.sceneLeft = dialog->exportLeftScene->isChecked();
      ret.sceneRight = dialog->exportRightScene->isChecked();
      //ret.sceneLight = dialog->comboboxSceneLights->currentText().toUtf8().data();

      ret.filenameLeft = dialog->lineEditMitsubaSceneLeft->text().toStdString();
      ret.filenameRight = dialog->lineEditMitsubaSceneRight->text().toStdString();

      ret.pathOutput = dialog->lineEditMitsubaOutputPath->text().toStdString();
      ret.pathResources = dialog->lineEditMitsubaResourcesPath->text().toStdString();
      ret.resolutionW = dialog->spinBoxMitsubaResolutionW->value();
      ret.resolutionH = dialog->spinBoxMitsubaResolutionH->value();
      ret.samples = dialog->comboboxMitsubaSamples->currentText().toInt();
      ret.threads = dialog->spinBoxMitsubaThreads->value();

      //ret.transect = dialog->onlyTransectRadioButton->isChecked();
    }

    dialog->deleteLater();

    return ret;
}
