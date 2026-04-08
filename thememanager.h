#pragma once
#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QObject>
#include <QString>

class ThemeManager : public QObject
{
    Q_OBJECT
public:
    static ThemeManager& instance();

    QString currentTheme() const;
    void setCurrentTheme(const QString& name);

    QString backgroundPathMain() const;
    QString backgroundPathGame() const;
    QString backgroundPathDialog() const;
    QString tileIconPath(int value) const;

    int tileTypeCount() const;

signals:
    void themeChanged();

private:
    explicit ThemeManager(QObject* parent = nullptr);
    void load();
    void save();

    QString ensureThemeName(const QString& name) const;
    bool exists(const QString& qrcPath) const;

private:
    QString m_theme;
};

#endif