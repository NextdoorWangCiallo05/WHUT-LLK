#include "rankmanager.h"

#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <algorithm>

RankManager& RankManager::instance()
{
    static RankManager ins;
    return ins;
}

QString RankManager::storagePath() const
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    return dir + "/timed_rank.json";
}

void RankManager::load()
{
    m_records.clear();

    QFile f(storagePath());
    if (!f.open(QIODevice::ReadOnly))
        return;

    const QByteArray data = f.readAll();
    f.close();

    const QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isArray())
        return;

    const QJsonArray arr = doc.array();
    for (const auto& v : arr) {
        if (!v.isObject()) continue;
        QJsonObject o = v.toObject();

        RankRecord rec;
        rec.nickname = o.value("nickname").toString("Anonymous");
        rec.leftSec = o.value("leftSec").toInt(0);
        rec.dateTime = QDateTime::fromString(o.value("dateTime").toString(), Qt::ISODate);

        if (!rec.dateTime.isValid())
            rec.dateTime = QDateTime::currentDateTime();

        m_records.push_back(rec);
    }

    sortAndTrim();
}

void RankManager::save()
{
    sortAndTrim();

    QJsonArray arr;
    for (const auto& rec : m_records) {
        QJsonObject o;
        o["nickname"] = rec.nickname;
        o["leftSec"] = rec.leftSec;
        o["dateTime"] = rec.dateTime.toString(Qt::ISODate);
        arr.push_back(o);
    }

    QFile f(storagePath());
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return;

    f.write(QJsonDocument(arr).toJson(QJsonDocument::Indented));
    f.close();
}

void RankManager::addRecord(const QString& nickname, int leftSec)
{
    RankRecord rec;
    QString name = nickname.trimmed();
    rec.nickname = name.isEmpty() ? QStringLiteral("Anonymous") : name;
    rec.leftSec = leftSec;
    rec.dateTime = QDateTime::currentDateTime();

    m_records.push_back(rec);
    sortAndTrim();
    save();
}

QList<RankRecord> RankManager::topRecords(int limit) const
{
    QList<RankRecord> out = m_records;
    if (limit > 0 && out.size() > limit)
        out = out.mid(0, limit);
    return out;
}

void RankManager::clear()
{
    m_records.clear();
    save();
}

void RankManager::sortAndTrim()
{
    std::sort(m_records.begin(), m_records.end(), [](const RankRecord& a, const RankRecord& b) {
        if (a.leftSec != b.leftSec)
            return a.leftSec > b.leftSec; // 剩余时间越多越靠前
        return a.dateTime < b.dateTime;   // 时间早的靠前
        });

    while (m_records.size() > 10)
        m_records.removeLast();
}