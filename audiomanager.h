#pragma once
#pragma once
#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <QObject>

class QMediaPlayer;
class QAudioOutput;

class AudioManager : public QObject
{
    Q_OBJECT
public:
    static AudioManager& instance();

    void init();                       // 初始化播放器
    void playBgm(const QString& res);  // 例如 ":/audio/bgm.mp3"
    void stopBgm();

    void setVolume(int v);             // 0~100
    int  volume() const;

    void setMuted(bool m);
    bool isMuted() const;

signals:
    void audioStateChanged();

private:
    explicit AudioManager(QObject* parent = nullptr);
    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;
    void saveSettings();

private:
    QMediaPlayer* m_player;
    QAudioOutput* m_output;
    int m_volume;
    bool m_muted;
};

#endif