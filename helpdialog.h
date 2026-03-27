#pragma once
#ifndef HELPDIALOG_H
#define HELPDIALOG_H

#include <QDialog>

class QLabel;
class QPushButton;

class HelpDialog : public QDialog
{
    Q_OBJECT
public:
    explicit HelpDialog(QWidget* parent = nullptr);

    // 自定义标题和正文
    void setHelpTitle(const QString& title);
    void setHelpText(const QString& text);

private:
    QLabel* titleLabel;
    QLabel* contentLabel;
    QPushButton* closeBtn;
};

#endif