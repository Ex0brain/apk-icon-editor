#include "effects.h"
#include <QBoxLayout>

EffectsDialog::EffectsDialog(QWidget *parent) : QDialog(parent)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    resize(260, 300);

    QVBoxLayout *layout = new QVBoxLayout(this);
    QGridLayout *layoutRotate = new QGridLayout();
    QVBoxLayout *layoutColor = new QVBoxLayout();
    QHBoxLayout *layoutColorDepth = new QHBoxLayout();
    QVBoxLayout *layoutBlur = new QVBoxLayout();

    groupRotate = new QGroupBox(this);
    groupColor = new QGroupBox(this);
    groupBlur = new QGroupBox(this);
    QString degree = "%1" + QString(QChar(0xB0));
    btnRotate0 = new QPushButton(degree.arg("0"), this);
    btnRotate90 = new QPushButton(degree.arg("90"), this);
    btnRotate180 = new QPushButton(degree.arg("180"), this);
    btnRotate270 = new QPushButton(degree.arg("270"), this);
    btnFlipX = new QPushButton(this);
    btnFlipY = new QPushButton(this);
    btnColor = new QPushButton(this);
    labelColor = new QLabel(this);
    slideColor = new QSlider(Qt::Horizontal, this);
    slideBlur = new QSlider(Qt::Horizontal, this);
    colorDialog = new QColorDialog(this);
    buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

    btnRotate0->setCheckable(true);
    btnRotate90->setCheckable(true);
    btnRotate180->setCheckable(true);
    btnRotate270->setCheckable(true);
    btnFlipX->setCheckable(true);
    btnFlipY->setCheckable(true);
    btnRotate0->setAutoExclusive(true);
    btnRotate90->setAutoExclusive(true);
    btnRotate180->setAutoExclusive(true);
    btnRotate270->setAutoExclusive(true);
    slideBlur->setRange(10, 500);
    slideColor->setRange(0, 100);
    groupColor->setCheckable(true);
    groupBlur->setCheckable(true);
    groupRotate->setLayout(layoutRotate);
    groupColor->setLayout(layoutColor);
    groupBlur->setLayout(layoutBlur);
    btnColor->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    layout->addWidget(groupRotate);
    layout->addWidget(groupColor);
    layout->addWidget(groupBlur);
    layoutRotate->addWidget(btnRotate0, 0, 0);
    layoutRotate->addWidget(btnRotate90, 0, 1);
    layoutRotate->addWidget(btnRotate180, 0, 2);
    layoutRotate->addWidget(btnRotate270, 0, 3);
    layoutRotate->addWidget(btnFlipX, 1, 0, 1, 2);
    layoutRotate->addWidget(btnFlipY, 1, 2, 1, 2);
    layoutColor->addWidget(btnColor);
    layoutColor->addLayout(layoutColorDepth);
    layoutColorDepth->addWidget(labelColor);
    layoutColorDepth->addWidget(slideColor);
    layoutBlur->addWidget(slideBlur);
    layout->addWidget(buttons);

    mapRotate = new QSignalMapper(this);
    connect(mapRotate, SIGNAL(mapped(int)), this, SIGNAL(rotate(int)));
    connect(btnRotate0, SIGNAL(clicked()), mapRotate, SLOT(map()));
    connect(btnRotate90, SIGNAL(clicked()), mapRotate, SLOT(map()));
    connect(btnRotate180, SIGNAL(clicked()), mapRotate, SLOT(map()));
    connect(btnRotate270, SIGNAL(clicked()), mapRotate, SLOT(map()));
    mapRotate->setMapping(btnRotate0, 0);
    mapRotate->setMapping(btnRotate90, 90);
    mapRotate->setMapping(btnRotate180, 180);
    mapRotate->setMapping(btnRotate270, 270);

    connect(groupColor, SIGNAL(clicked(bool)), this, SIGNAL(colorActivated(bool)));
    connect(groupBlur, SIGNAL(clicked(bool)), this, SIGNAL(blurActivated(bool)));
    connect(btnFlipX, SIGNAL(clicked(bool)), this, SIGNAL(flipX(bool)));
    connect(btnFlipY, SIGNAL(clicked(bool)), this, SIGNAL(flipY(bool)));
    connect(btnColor, SIGNAL(clicked()), colorDialog, SLOT(exec()));
    connect(slideColor, SIGNAL(valueChanged(int)), this, SLOT(setColorDepth(int)));
    connect(slideBlur, SIGNAL(valueChanged(int)), this, SLOT(setBlur(int)));
    connect(colorDialog, SIGNAL(colorSelected(QColor)), this, SLOT(setColor(QColor)));
    connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));
}

void EffectsDialog::retranslate()
{
    setWindowTitle(tr("Effects"));
    groupRotate->setTitle(tr("Rotation"));
    btnFlipX->setText(tr("Flip Horizontal"));
    btnFlipY->setText(tr("Flip Vertical"));
    groupColor->setTitle(tr("Color Overlay"));
    labelColor->setText(tr("Intensity:"));
    groupBlur->setTitle(tr("Blur"));
    btnColor->setText(tr("Select Color"));
}

void EffectsDialog::setRotation(int rot)
{
    switch (rot) {
    case 0:
        btnRotate0->setChecked(true);
        break;
    case 90:
        btnRotate90->setChecked(true);
        break;
    case 180:
        btnRotate180->setChecked(true);
        break;
    case 270:
        btnRotate270->setChecked(true);
        break;
    }
}

void EffectsDialog::setColor(QColor color)
{
    QPixmap icon(32, 32);
    icon.fill(color);
    btnColor->setIcon(QIcon(icon));
    colorDialog->setCurrentColor(color);
    emit colorize(color);
}

void EffectsDialog::setColorDepth(int depth)
{
    slideColor->setSliderPosition(depth);
    emit colorDepth(qreal(depth) / 100);
}

void EffectsDialog::setBlur(int value)
{
    slideBlur->setSliderPosition(value);
    emit blur(qreal(value) / 10);
}
