#pragma once
#ifndef WINDOWROUND_H
#define WINDOWROUND_H

#include <QWidget>
#include <QPainterPath>
#include <QRegion>
#include <QResizeEvent>

static inline void applyRoundedWindow(QWidget* w, int radius = 10)
{
    if (!w) return;
    w->setAttribute(Qt::WA_TranslucentBackground, true);

    QPainterPath path;
    path.addRoundedRect(w->rect(), radius, radius);
    w->setMask(QRegion(path.toFillPolygon().toPolygon()));
}

#endif