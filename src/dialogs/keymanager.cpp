#include "keymanager.h"
#include "settings.h"
#include "globals.h"
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QProcess>
#include <QDebug>

KeyManager::KeyManager(QWidget *parent) : QDialog(parent)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowIcon(QIcon(":/gfx/actions/key.png"));
    resize(540 * Gui::Screen::dpi() / 100.0, 0);

    keyCreator = new KeyCreator(this);

    radioPem = new QRadioButton(this);
    radioKey = new QRadioButton(this);

    radioPem->setText("PEM/PK8");

    boxPem = new FileBox(this);
    boxPk8 = new FileBox(this);
    boxKey = new FileBox(this);

    boxPem->setTitle("PEM:");
    boxPk8->setTitle("PK8:");

    boxPem->setTitleWidth(26);
    boxPk8->setTitleWidth(26);

    boxPem->setFormats("PEM (*.pem);;");
    boxPk8->setFormats("PK8 (*.pk8);;");
    boxKey->setFormats("KeyStore (*.keystore);;");

    btnNew = new QPushButton(this);
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    labelAlias = new QLabel(this);
    labelAliasPass = new QLabel(this);
    labelStorePass = new QLabel(this);
    editAlias = new QLineEdit(this);
    editAliasPass = new QLineEdit(this);
    editStorePass = new QLineEdit(this);

    editAliasPass->setEchoMode(QLineEdit::Password);
    editStorePass->setEchoMode(QLineEdit::Password);

    groupPem = new QGroupBox(this);
    QVBoxLayout *layoutPem = new QVBoxLayout;
    layoutPem->addWidget(boxPem);
    layoutPem->addWidget(boxPk8);
    groupPem->setLayout(layoutPem);

    groupKey = new QGroupBox(this);
    QGridLayout *layoutKey = new QGridLayout;
    layoutKey->addWidget(boxKey, 0, 0, 1, 0);
    layoutKey->addWidget(labelStorePass, 1, 0);
    layoutKey->addWidget(editStorePass, 1, 1);
    layoutKey->addWidget(labelAlias, 2, 0, Qt::AlignLeft);
    layoutKey->addWidget(editAlias, 2, 1);
    layoutKey->addWidget(labelAliasPass, 3, 0, Qt::AlignLeft);
    layoutKey->addWidget(editAliasPass, 3, 1);
    layoutKey->addWidget(btnNew, 4, 0, 1, 0);
    groupKey->setLayout(layoutKey);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(radioPem);
    layout->addWidget(radioKey);
    layout->addWidget(groupPem);
    layout->addWidget(groupKey);
    layout->addWidget(buttons);

    connect(radioPem, SIGNAL(clicked()), this, SLOT(setOptionPem()));
    connect(radioKey, SIGNAL(clicked()), this, SLOT(setOptionKey()));
    connect(btnNew, SIGNAL(clicked()), keyCreator, SLOT(open()));
    connect(keyCreator, SIGNAL(created(QString)), boxKey, SLOT(setValue(QString)));
    connect(keyCreator, SIGNAL(success(QString, QString)), this, SIGNAL(success(QString, QString)));
    connect(keyCreator, SIGNAL(warning(QString, QString)), this, SIGNAL(warning(QString, QString)));
    connect(keyCreator, SIGNAL(error(QString, QString)), this, SIGNAL(error(QString, QString)));
    connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));
}

void KeyManager::retranslate()
{
    setWindowTitle(tr("Key Manager"));
    radioKey->setText(QString("KeyStore (%1)").arg(tr("Requires JDK")));
    boxKey->setTitle(QString("%1:").arg(tr("KeyStore")));
    btnNew->setText(tr("Create &New KeyStore or Alias"));
    labelAlias->setText(QString("%1:").arg(tr("Alias")));
    labelAliasPass->setText(QString("%1:").arg(tr("Alias Password")));
    labelStorePass->setText(QString("%1:").arg(tr("KeyStore Password")));
    keyCreator->retranslate();
}

void KeyManager::setOptionPem()
{
    groupPem->setEnabled(true);
    groupKey->setEnabled(false);
}

void KeyManager::setOptionKey()
{
    groupPem->setEnabled(false);
    groupKey->setEnabled(true);
}

void KeyManager::accept()
{
    Settings::set_use_keystore(radioKey->isChecked());
    Settings::set_pem(boxPem->value());
    Settings::set_pk8(boxPk8->value());
    Settings::set_keystore(boxKey->value());
    Settings::set_alias(editAlias->text());
    Settings::set_keystore_pass(editStorePass->text());
    Settings::set_alias_pass(editAliasPass->text());
    reset();
    QDialog::accept();
}

void KeyManager::reject()
{
    reset();
    QDialog::reject();
}

void KeyManager::reset()
{
    if (Settings::get_use_keystore()) {
        radioKey->setChecked(true);
        setOptionKey();
    }
    else {
        radioPem->setChecked(true);
        setOptionPem();
    }
    boxPem->setValue(Settings::get_pem());
    boxPk8->setValue(Settings::get_pk8());
    boxKey->setValue(Settings::get_keystore());
    editAlias->setText(Settings::get_alias());
    editStorePass->setText(Settings::get_keystore_pass());
    editAliasPass->setText(Settings::get_alias_pass());
}

// KeyCreator

KeyCreator::KeyCreator(QWidget *parent) : QDialog(parent)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    resize(400 * Gui::Screen::dpi() / 100.0, 0);

    QGridLayout *layout = new QGridLayout(this);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    labelStorePass1 = new QLabel(this);
    labelStorePass2 = new QLabel(this);
    labelAlias = new QLabel(this);
    labelAliasPass1 = new QLabel(this);
    labelAliasPass2 = new QLabel(this);
    labelYears = new QLabel(this);
    labelName = new QLabel(this);
    labelUnit = new QLabel(this);
    labelOrgan = new QLabel(this);
    labelCity = new QLabel(this);
    labelState = new QLabel(this);
    labelCountry = new QLabel(this);
    editStorePass1 = new QLineEdit(this);
    editStorePass2 = new QLineEdit(this);
    editAlias = new QLineEdit(this);
    editAliasPass1 = new QLineEdit(this);
    editAliasPass2 = new QLineEdit(this);
    editYears = new QSpinBox(this);
    editName = new QLineEdit(this);
    editUnit = new QLineEdit(this);
    editOrgan = new QLineEdit(this);
    editCity = new QLineEdit(this);
    editState = new QLineEdit(this);
    editCountry = new QLineEdit(this);

    editStorePass1->setFocus();
    editStorePass1->setEchoMode(QLineEdit::Password);
    editStorePass2->setEchoMode(QLineEdit::Password);
    editAliasPass1->setEchoMode(QLineEdit::Password);
    editAliasPass2->setEchoMode(QLineEdit::Password);
    editYears->setRange(1, 999999);

    clear();

    layout->addWidget(labelStorePass1, 0, 0);
    layout->addWidget(labelStorePass2, 1, 0);
    layout->addWidget(labelAlias, 2, 0);
    layout->addWidget(labelAliasPass1, 3, 0);
    layout->addWidget(labelAliasPass2, 4, 0);
    layout->addWidget(labelYears, 5, 0);
    layout->addWidget(labelName, 6, 0);
    layout->addWidget(labelUnit, 7, 0);
    layout->addWidget(labelOrgan, 8, 0);
    layout->addWidget(labelCity, 9, 0);
    layout->addWidget(labelState, 10, 0);
    layout->addWidget(labelCountry, 11, 0);
    layout->addWidget(editStorePass1, 0, 1);
    layout->addWidget(editStorePass2, 1, 1);
    layout->addWidget(editAlias, 2, 1);
    layout->addWidget(editAliasPass1, 3, 1);
    layout->addWidget(editAliasPass2, 4, 1);
    layout->addWidget(editYears, 5, 1);
    layout->addWidget(editName, 6, 1);
    layout->addWidget(editUnit, 7, 1);
    layout->addWidget(editOrgan, 8, 1);
    layout->addWidget(editCity, 9, 1);
    layout->addWidget(editState, 10, 1);
    layout->addWidget(editCountry, 11, 1);
    layout->addWidget(buttons, 12, 1);

    connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));
}

void KeyCreator::retranslate()
{
    labelStorePass1->setText(QString("%1:").arg(tr("Password (KeyStore)")));
    labelStorePass2->setText(QString("%1:").arg(tr("Confirm Password")));
    labelAlias->setText(QString("%1:").arg(tr("Alias")));
    labelAliasPass1->setText(QString("%1:").arg(tr("Password (Alias)")));
    labelAliasPass2->setText(QString("%1:").arg(tr("Confirm Password")));
    labelYears->setText(QString("%1:").arg(tr("Validity (Years)")));
    labelName->setText(QString("%1:").arg(tr("First and Last Name")));
    labelUnit->setText(QString("%1:").arg(tr("Organizational Unit")));
    labelOrgan->setText(QString("%1:").arg(tr("Organization")));
    labelCity->setText(QString("%1:").arg(tr("City or Locality")));
    labelState->setText(QString("%1:").arg(tr("State or Province")));
    labelCountry->setText(QString("%1:").arg(tr("Country Code")));
}

void KeyCreator::clear()
{
    editStorePass1->clear();
    editStorePass2->clear();
    editAlias->clear();
    editAliasPass1->clear();
    editAliasPass2->clear();
    editYears->setValue(25);
    editName->clear();
    editUnit->clear();
    editOrgan->clear();
    editCity->clear();
    editState->clear();
    editCountry->clear();
}

void KeyCreator::accept()
{
    // Validate data:

    if (editStorePass1->text().length() < 6) {
        emit warning(NULL, tr("Password must be at least 6 characters."));
        editStorePass1->setFocus();
        editStorePass1->selectAll();
        return;
    }

    if (editStorePass1->text() != editStorePass2->text()) {
        emit warning(NULL, tr("Passwords do not match."));
        editStorePass2->setFocus();
        editStorePass2->selectAll();
        return;
    }

    if (editAlias->text().isEmpty()) {
        emit warning(NULL, tr("Enter alias name."));
        editAlias->setFocus();
        editAlias->selectAll();
        return;
    }

    if (editAliasPass1->text() != editAliasPass2->text()) {
        emit warning(NULL, tr("Passwords do not match."));
        editAliasPass2->setFocus();
        editAliasPass2->selectAll();
        return;
    }

    if (editAliasPass1->text().length() < 6) {
        emit warning(NULL, tr("Password must be at least 6 characters."));
        editAliasPass1->setFocus();
        editAliasPass1->selectAll();
        return;
    }

    // Create KeyStore and Alias:

    const QString FILENAME = QFileDialog::getSaveFileName(this, NULL, NULL, "KeyStore (*.keystore)");
    if (FILENAME.isEmpty()) {
        return;
    }
    qDebug() << "Creating KeyStore...";
    const QString ENV_PATH = qgetenv("PATH");
    const QString JAVA_HOME = qgetenv("JAVA_HOME");
    const QString KEYTOOL_CMD =
            QString("keytool -genkeypair -v -keystore \"%1\" -storepass \"%10\""
                    " -alias \"%2\" -keyalg RSA -keysize 2048"
                    " -dname \"CN=%3, OU=%4, O=%5, L=%6, S=%7, C=%8\""
                    " -validity %9 -keypass \"%11\"")
                        .arg(FILENAME)
                        .arg(editAlias->text())
                        .arg(editName->text())
                        .arg(editUnit->text())
                        .arg(editOrgan->text())
                        .arg(editCity->text())
                        .arg(editState->text())
                        .arg(editCountry->text())
                        .arg(editYears->text().toInt() * 365);
    qputenv("PATH", ENV_PATH.toStdString().c_str());
    qputenv("PATH", QString("%1;%2/bin").arg(ENV_PATH, JAVA_HOME).toStdString().c_str());
    qDebug() << qPrintable(KEYTOOL_CMD.arg("*****", "*****"));
    QProcess p;
    p.start(KEYTOOL_CMD.arg(editStorePass1->text(), editAliasPass1->text()));
    qputenv("PATH", ENV_PATH.toStdString().c_str());

    if (p.waitForStarted(-1)) {
        p.waitForFinished(10000);
        if (p.exitCode() != 0) {
            QString error_text = p.readAllStandardError().trimmed();
            if (error_text.isEmpty()) error_text = p.readAllStandardOutput().trimmed();
            qDebug() << qPrintable(QString("Keytool exit code: %1").arg(p.exitCode()));
            qDebug() << error_text;
            emit warning("Keytool", tr("%1: invalid parameters").arg("Keytool"));
            return;
        }
    }
    else {
        const QString ERROR_TEXT = tr("Error starting %1.\n"
                                      "Check your JDK installation and "
                                      "PATH environment variable.").arg("Keytool");
        emit error("Keytool", ERROR_TEXT);
        return;
    }
    qDebug() << "Done.\n";
    emit success("Keytool", tr("KeyStore successfully created/updated!"));
    emit created(FILENAME);
    clear();
    QDialog::accept();
}

void KeyCreator::reject()
{
    clear();
    QDialog::reject();
}
