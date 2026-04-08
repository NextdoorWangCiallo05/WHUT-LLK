#pragma once
#ifndef WINDOWDRAG_H
#define WINDOWDRAG_H

#include <QWidget>
#include <QMouseEvent>
#include <QPoint>
#include <QAbstractButton>
#include <QAbstractSlider>
#include <QComboBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QPlainTextEdit>

struct WindowDragState
{
    bool dragging = false;
    QPoint dragPos;
};

// 最小改动：点击交互控件时，不启动窗口拖动
static inline bool shouldBlockWindowDrag(QWidget* w, QMouseEvent* event)
{
    if (!w || !event) return true;

    QWidget* child = w->childAt(event->position().toPoint());
    if (!child) return false;

    return qobject_cast<QAbstractButton*>(child) ||
        qobject_cast<QAbstractSlider*>(child) ||
        qobject_cast<QComboBox*>(child) ||
        qobject_cast<QSpinBox*>(child) ||
        qobject_cast<QLineEdit*>(child) ||
        qobject_cast<QTextEdit*>(child) ||
        qobject_cast<QPlainTextEdit*>(child);
}

static inline bool handleWindowDragMousePress(
    QWidget* w, QMouseEvent* event, WindowDragState& s)
{
    if (!w || !event) return false;
    if (event->button() == Qt::LeftButton) {
        if (shouldBlockWindowDrag(w, event)) return false; // 关键新增
        s.dragging = true;
        s.dragPos = event->globalPosition().toPoint() - w->frameGeometry().topLeft();
        event->accept();
        return true;
    }
    return false;
}

static inline bool handleWindowDragMouseMove(
    QWidget* w, QMouseEvent* event, WindowDragState& s)
{
    if (!w || !event) return false;
    if (s.dragging && (event->buttons() & Qt::LeftButton)) {
        w->move(event->globalPosition().toPoint() - s.dragPos);
        event->accept();
        return true;
    }
    return false;
}

static inline void handleWindowDragMouseRelease(
    QMouseEvent* event, WindowDragState& s)
{
    Q_UNUSED(event);
    s.dragging = false;
}

#endif