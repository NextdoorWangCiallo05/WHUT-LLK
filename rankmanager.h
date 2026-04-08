#pragma once
#ifndef RANKMANAGER_H
#define RANKMANAGER_H

#include <QString>
#include <QDateTime>
#include <QList>

struct RankRecord
{
    QString nickname;
    int leftSec = 0;
    QDateTime dateTime;
};

class RankManager
{
public:
    static RankManager& instance();

    void load();
    void save();

    void addRecord(const QString& nickname, int leftSec);
    QList<RankRecord> topRecords(int limit = 10) const;
    void clear();

private:
    RankManager() = default;

    QString storagePath() const;
    void sortAndTrim();

private:
    QList<RankRecord> m_records;
};

#endif