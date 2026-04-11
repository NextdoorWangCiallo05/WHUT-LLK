#include "classicrankmanager.h"

#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>
#include <algorithm>

// 获取单例实例
ClassicRankManager& ClassicRankManager::instance()
{
    static ClassicRankManager ins;
    return ins;
}

// 获取排行榜数据的存储路径
QString ClassicRankManager::storagePath() const
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    return dir + "/classic_rank.json";
}

// 从存储文件加载排行榜数据
void ClassicRankManager::load()
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

        ClassicRankRecord rec;
        rec.nickname = o.value("nickname").toString("Anonymous");
        rec.usedSec = o.value("usedSec").toInt(0);
        rec.dateTime = QDateTime::fromString(o.value("dateTime").toString(), Qt::ISODate);

        if (!rec.dateTime.isValid())
            rec.dateTime = QDateTime::currentDateTime();

        m_records.push_back(rec);
    }

    sortAndTrim();
}

// 将排行榜数据保存到存储文件
void ClassicRankManager::save()
{
    sortAndTrim();

    QJsonArray arr;
    for (const auto& rec : m_records) {
        QJsonObject o;
        o["nickname"] = rec.nickname;
        o["usedSec"] = rec.usedSec;
        o["dateTime"] = rec.dateTime.toString(Qt::ISODate);
        arr.push_back(o);
    }

    QFile f(storagePath());
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return;

    f.write(QJsonDocument(arr).toJson(QJsonDocument::Indented));
    f.close();
}

// 添加新的排行榜记录
void ClassicRankManager::addRecord(const QString& nickname, int usedSec)
{
    ClassicRankRecord rec;
    QString name = nickname.trimmed();
    rec.nickname = name.isEmpty() ? QStringLiteral("Anonymous") : name;
    rec.usedSec = qMax(0, usedSec);
    rec.dateTime = QDateTime::currentDateTime();

    m_records.push_back(rec);
    sortAndTrim();
    save();
}

// 获取排行榜前N条记录，默认返回前10条
QList<ClassicRankRecord> ClassicRankManager::topRecords(int limit) const
{
    QList<ClassicRankRecord> out = m_records;
    if (limit > 0 && out.size() > limit)
        out = out.mid(0, limit);
    return out;
}

// 清空排行榜数据
void ClassicRankManager::clear()
{
    m_records.clear();
    save();
}

// 对排行榜记录进行排序和裁剪，确保只保留前10条记录
void ClassicRankManager::sortAndTrim()
{
    std::sort(m_records.begin(), m_records.end(), [](const ClassicRankRecord& a, const ClassicRankRecord& b) {
        if (a.usedSec != b.usedSec)
            return a.usedSec < b.usedSec;  // 用时越少越靠前
        return a.dateTime < b.dateTime;    // 同分按更早时间
        });

    while (m_records.size() > 10)
        m_records.removeLast();
}

// 将排行榜数据导出为CSV文件，返回是否成功
bool ClassicRankManager::exportToCsv(const QString& filePath) const
{
    QFile f(filePath);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
        return false;

    QTextStream out(&f);
    out.setEncoding(QStringConverter::Utf8);
    out << "nickname,usedSec,dateTime\n";
    for (const auto& r : m_records) {
        QString nick = r.nickname;
        nick.replace("\"", "\"\"");
        out << "\"" << nick << "\","
            << r.usedSec << ","
            << r.dateTime.toString(Qt::ISODate) << "\n";
    }
    return true;
}