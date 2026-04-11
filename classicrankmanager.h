#pragma once
#ifndef CLASSICRANKMANAGER_H
#define CLASSICRANKMANAGER_H

#include <QString>
#include <QDateTime>
#include <QList>

struct ClassicRankRecord
{
    QString nickname;
    int usedSec = 0;      // 用时（秒），越少越好
    QDateTime dateTime;
};

class ClassicRankManager
{
public:
    static ClassicRankManager& instance();

    void load();
    void save();

    void addRecord(const QString& nickname, int usedSec);
    QList<ClassicRankRecord> topRecords(int limit = 10) const;
    void clear();

    bool exportToCsv(const QString& filePath) const;

private:
    ClassicRankManager() = default;

    QString storagePath() const;
    void sortAndTrim();

private:
    QList<ClassicRankRecord> m_records;
};

#endif