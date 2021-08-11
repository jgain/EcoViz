#include "export_dialog.h"

#include <QLabel>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QComboBox>
#include <QRadioButton>
#include <QButtonGroup>

ExportDialog::ExportDialog(QStringList allProfiles, QWidget* parent) : QDialog(parent)
{
    setWindowTitle("Export settings");

    QFormLayout* layout = new QFormLayout(this);

    // Profile selection
    comboboxProfiles = new QComboBox(this);
    for (QString profile : allProfiles)
    {
        comboboxProfiles->addItem(profile);
    }

    QLabel* comboboxProfilesLabel = new QLabel(QString("Export profile:"), this);
    layout->addRow(comboboxProfilesLabel, comboboxProfiles);

    // Scene index selection
    QHBoxLayout* hboxLayoutSceneIndex = new QHBoxLayout;
    QButtonGroup* buttonGroup = new QButtonGroup(this);
    
    leftSceneIndexRadioButton = new QRadioButton("0 (Left)");
    leftSceneIndexRadioButton->setChecked(true);
    hboxLayoutSceneIndex->addWidget(leftSceneIndexRadioButton);
    buttonGroup->addButton(leftSceneIndexRadioButton);

    rightSceneIndexRadioButton = new QRadioButton("1 (Right)");
    hboxLayoutSceneIndex->addWidget(rightSceneIndexRadioButton);
    buttonGroup->addButton(rightSceneIndexRadioButton);

    QLabel* sceneIndexLabel = new QLabel(QString("Scene index:"), this);
    layout->addRow(sceneIndexLabel, hboxLayoutSceneIndex);

    // Export selection
    hboxLayoutSceneIndex = new QHBoxLayout;
    buttonGroup = new QButtonGroup(this);

    entireSceneRadioButton = new QRadioButton("Entire scene");
    entireSceneRadioButton->setChecked(true);
    hboxLayoutSceneIndex->addWidget(entireSceneRadioButton);
    buttonGroup->addButton(entireSceneRadioButton);

    onlyTransectRadioButton = new QRadioButton("Only transect");
    hboxLayoutSceneIndex->addWidget(onlyTransectRadioButton);
    buttonGroup->addButton(onlyTransectRadioButton);

    QLabel* exportSelectionLabel = new QLabel(QString("Export selection:"), this);
    layout->addRow(exportSelectionLabel, hboxLayoutSceneIndex);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
    layout->addWidget(buttonBox);

    bool conn = connect(buttonBox, &QDialogButtonBox::accepted, this, &ExportDialog::accept);
    Q_ASSERT(conn);
    conn = connect(buttonBox, &QDialogButtonBox::rejected, this, &ExportDialog::reject);
    Q_ASSERT(conn);

    setLayout(layout);
}

ExportSettings ExportDialog::getExportSettings(QWidget* parent, QStringList allProfiles, bool* ok)
{
    ExportSettings ret;
    ExportDialog* dialog = new ExportDialog(allProfiles, parent);

    if (dialog->exec())
    {
        if (ok)
        {
            *ok = true;
        }

        ret.profile = dialog->comboboxProfiles->currentText().toUtf8().data();
        ret.sceneIndex = dialog->leftSceneIndexRadioButton->isChecked() ? 0 : 1;
        ret.transect = dialog->onlyTransectRadioButton->isChecked();
    }

    dialog->deleteLater();

    return ret;
}