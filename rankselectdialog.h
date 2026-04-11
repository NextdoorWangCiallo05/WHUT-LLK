#pragma once
#ifndef RANKSELECTDIALOG_H
#define RANKSELECTDIALOG_H

#include <QDialog>

class QPushButton;

class RankSelectDialog : public QDialog
{
    Q_OBJECT
public:
    explicit RankSelectDialog(QWidget* parent = nullptr);

    // 1=计时榜 2=经典榜 0=未选择/取消
    int selectedType() const { return m_selectedType; }

private:
    QPushButton* btnTimed;
    QPushButton* btnClassic;
    QPushButton* btnCancel;

    int m_selectedType = 0;
};

#endif