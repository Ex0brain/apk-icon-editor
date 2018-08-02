#include "dialogs.h"
#include "globals.h"
#include <QApplication>
#include <QBoxLayout>
#include <QTextEdit>
#include <QClipboard>
#include <QTextStream>
#include <QDesktopServices>
#include <QFile>
#include <QUrl>

QString Dialogs::getString(QString text, QString title, QPixmap icon, QWidget *parent)
{
    InputDialog dialog(text, title, false, icon, parent);
    return dialog.exec() ? dialog.getString() : QString();
}

QString Dialogs::getPassword(QString text, QString title, QPixmap icon, QWidget *parent)
{
    InputDialog dialog(text, title, true, icon, parent);
    return dialog.exec() ? dialog.getString() : QString();
}

QSize Dialogs::getSize(QString title, int width, int height, QWidget *parent)
{
    ResizeDialog dialog(title, width, height, parent);
    bool result = dialog.exec();
    const int WIDTH = dialog.getWidth();
    const int HEIGHT = dialog.getHeight();
    return result ? QSize(WIDTH, HEIGHT) : QSize();
}

// Input Dialog

InputDialog::InputDialog(QString text, QString title, bool password, QPixmap icon, QWidget *parent) : QDialog(parent)
{
    setWindowTitle(title);
    setModal(true);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSizeConstraint(QLayout::SetFixedSize);

    this->icon = new QLabel(this);
    this->label = new QLabel(this);
    this->input = new QLineEdit(this);
    this->buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    this->icon->setPixmap(icon);
    this->icon->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    this->label->setText(text);
    this->input->setEchoMode(password ? QLineEdit::Password : QLineEdit::Normal);

    QHBoxLayout *row1 = new QHBoxLayout();
    if (!icon.isNull()) row1->addWidget(this->icon);
    row1->addWidget(this->label);

    layout->addLayout(row1);
    layout->addWidget(this->input);
    layout->addWidget(this->buttons);

    checkInput("");

    connect(this->input, SIGNAL(textChanged(QString)), this, SLOT(checkInput(QString)));
    connect(this->buttons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(this->buttons, SIGNAL(rejected()), this, SLOT(reject()));
}

void InputDialog::paste()
{
    input->paste();
}

void InputDialog::checkInput(QString text)
{
    buttons->clear();
    if (!text.isEmpty()) {
        buttons->addButton(QDialogButtonBox::Ok)->setDefault(true);
    }
    else {
        QPushButton *btnPaste = new QPushButton(tr("Paste"), this);
        buttons->addButton(btnPaste, QDialogButtonBox::ActionRole);
        btnPaste->setDefault(true);
        connect(btnPaste, SIGNAL(clicked()), this, SLOT(paste()));
    }
    buttons->addButton(QDialogButtonBox::Cancel);
}

// Progress Dialog

ProgressDialog::ProgressDialog(QWidget *parent) : QDialog(parent)
{
    resize(Gui::Screen::scaled(220, 100));
    setModal(true);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    allowCancel = true;

    QVBoxLayout *layout = new QVBoxLayout(this);

    label = new QLabel(this);
    icon = new QLabel(this);
    progress = new QProgressBar(this);
    buttons = new QDialogButtonBox(QDialogButtonBox::Cancel, this);

    icon->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    progress->setRange(0, 100);

    QHBoxLayout *row1 = new QHBoxLayout();
    row1->addWidget(icon);
    row1->addWidget(label);

    layout->addLayout(row1);
    layout->addWidget(progress);
    layout->addWidget(buttons);

    setFixedHeight((sizeHint().height() < 100) ? 100 : sizeHint().height());

#ifdef WINEXTRAS
    isWinExtras = QSysInfo::windowsVersion() >= QSysInfo::WV_VISTA;
    if (isWinExtras) {
        taskbar = new QWinTaskbarButton(this);
    }
#endif

    connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));
}

void ProgressDialog::setProgress(short percentage, QString text)
{
    label->setText(text);
    progress->setValue(percentage);
#ifdef WINEXTRAS
    if (isWinExtras) {
        taskbar->setWindow(static_cast<QWidget*>(parent())->windowHandle());
        if (icon->pixmap()) { taskbar->setOverlayIcon(*icon->pixmap()); }
        QWinTaskbarProgress *taskProgress = taskbar->progress();
        taskProgress->setValue(percentage);
        taskProgress->setVisible(true);
    }
#endif
    show();
}

void ProgressDialog::setText(QString text)
{
    label->setText(text);
}

void ProgressDialog::setIcon(QPixmap pixmap)
{
    pixmap = pixmap.scaled(16, 16, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    icon->setPixmap(pixmap);
}

void ProgressDialog::setAllowCancel(bool allow)
{
    allowCancel = allow;
    setWindowFlags(windowFlags() & (allow ? Qt::WindowCloseButtonHint : ~Qt::WindowCloseButtonHint));
    buttons->button(QDialogButtonBox::Cancel)->setEnabled(allow);
}

void ProgressDialog::accept()
{
    QDialog::accept();
    reset();
}

void ProgressDialog::reject()
{
    if (allowCancel) {
        QDialog::reject();
        reset();
    }
}

void ProgressDialog::reset()
{
    progress->setValue(0);
#ifdef WINEXTRAS
    if (isWinExtras) {
        taskbar->clearOverlayIcon();
        QWinTaskbarProgress *taskProgress = taskbar->progress();
        taskProgress->setVisible(false);
        taskProgress->reset();
    }
#endif
}

// ResizeDialog

ResizeDialog::ResizeDialog(QString title, int width, int height, QWidget *parent) : QDialog(parent)
{
    setWindowTitle(title);
    setModal(true);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QGridLayout *grid = new QGridLayout(this);
    grid->setSizeConstraint(QLayout::SetFixedSize);

    QLabel *wLabel = new QLabel(tr("Width:"), this);
    QLabel *hLabel = new QLabel(tr("Height:"), this);
    wInput = new QSpinBox(this);
    hInput = new QSpinBox(this);
    btnLock = new QToolButton(this);
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    wInput->setRange(1, 4096);
    hInput->setRange(1, 4096);
    wInput->setValue(width);
    hInput->setValue(height);
    btnLock->setIcon(QIcon(":/gfx/actions/lock.png"));
    btnLock->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnLock->setCheckable(true);

    grid->addWidget(wLabel, 0, 0);
    grid->addWidget(wInput, 0, 1);
    grid->addWidget(hLabel, 1, 0);
    grid->addWidget(hInput, 1, 1);
    grid->addWidget(btnLock, 0, 2, 2, 1);
    grid->addWidget(buttons, 4, 1, 2, 1);

    connect(wInput, SIGNAL(valueChanged(int)), this, SLOT(widthChanged(int)));
    connect(hInput, SIGNAL(valueChanged(int)), this, SLOT(heightChanged(int)));
    connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));
}
