#pragma once
#ifndef RESULTDIALOG_H
#define RESULTDIALOG_H

#include <QDialog>

class QLabel;
class QPushButton;

class ResultDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ResultDialog(QWidget* parent = nullptr);

    void setResult(bool win, int totalSec, int leftSec, int removedPairs);

private:
    QLabel* titleLabel;
    QLabel* detailLabel;
    QLabel* gradeLabel;
    QPushButton* okBtn;

    QString calcGrade(bool win, int usedSec, int leftSec) const;
};

#endif