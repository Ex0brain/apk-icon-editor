#include "drawarea.h"
#include <QDragEnterEvent>
#include <QMimeData>
#include <QPainter>
#include <QDir>

const QString STYLE_DEFAULT =
    "DrawArea {background: url(:/gfx/bg.png)"
    "no-repeat bottom left;"
    "border: 1px solid gray;}"
;

DrawArea::DrawArea(QWidget *parent) : QLabel(parent)
{
    setFont(QFont("Open Sans Light", 13));
    setAlignment(Qt::AlignCenter);
    setStyleSheet(STYLE_DEFAULT + "DrawArea::hover {background-color: white;}");

    setMouseTracking(true);
    setAutoFillBackground(true);
    setCursor(Qt::PointingHandCursor);

    background = palette().color(QPalette::Background);
    rect_w = 0;
    rect_h = 0;
    welcome = true;
}

void DrawArea::setIcon(Icon *_icon)
{
    icon = _icon;
    welcome = false;
    setCursor(Qt::ArrowCursor);
    setStyleSheet(STYLE_DEFAULT);
    repaint();
}

void DrawArea::mousePressEvent(QMouseEvent *event)
{
    if (welcome && event->button() == Qt::LeftButton) {
        emit clicked();
    }
    event->accept();
}

void DrawArea::paintEvent(QPaintEvent *event)
{
    if (!welcome) {
        QPainter painter(this);
        const int x = width()/2 - rect_w/2;
        const int y = height()/2 - rect_h/2;
        painter.fillRect(x+1, y+1, rect_w-1, rect_h-1, background);
        //if (background == palette().color(QPalette::Background)) {
            painter.fillRect(x+1, y+1, rect_w-1, rect_h-1, Qt::Dense7Pattern);
        //}
        painter.drawPixmap(width()/2 - icon->width()/2,
                           height()/2 - icon->height()/2,
                           icon->getPixmap());
        painter.drawRect(x, y, rect_w, rect_h);
        painter.setPen(Qt::lightGray);
        painter.drawRect(x-1, y-1, rect_w+2, rect_h+2);
    }
    else {
        QLabel::paintEvent(event);
    }
}
