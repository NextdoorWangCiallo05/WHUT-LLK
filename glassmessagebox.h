#pragma once
#ifndef GLASSMESSAGEBOX_H
#define GLASSMESSAGEBOX_H

#include <QDialog>
#include <QString>

class GlassMessageBox
{
public:
    static void information(QWidget* parent, const QString& title, const QString& text);
    static void warning(QWidget* parent, const QString& title, const QString& text);
};

#endif