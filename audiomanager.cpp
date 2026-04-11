#include "audiomanager.h"
#include <QSettings>
#include <QSoundEffect>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QDebug>
#include <QUrl>

// 这个文件实现了 AudioManager 类，负责管理游戏的背景音乐和音效，包括播放、停止、调整音量和静音等功能。
// 它使用了 Qt 的多媒体模块来处理音频，并通过 QSettings 来保存用户的音频设置。
// 设计上，AudioManager 是一个单例类，确保全局只有一个实例来管理音频资源。它预创建了两个 QSoundEffect 对象来复用，避免频繁创建销毁带来的性能问题。
// 通过信号 audioStateChanged，AudioManager 可以通知界面更新音频相关的状态显示，例如音量滑块和静音按钮的状态。

AudioManager::AudioManager(QObject* parent)
    : QObject(parent),
    m_player(nullptr),
    m_output(nullptr),
    m_clickSfx(nullptr),
    m_clearSfx(nullptr),
    m_volume(50),
    m_muted(false),
    m_sfxMuted(false)
{
    QSettings s("YourCompany", "LLK_Refresh");
    m_volume = s.value("audio/volume", 50).toInt();
    m_muted = s.value("audio/muted", false).toBool();
    m_sfxMuted = s.value("audio/sfxMuted", false).toBool();
}

// 获取单例实例
AudioManager& AudioManager::instance()
{
    static AudioManager ins;
    return ins;
}

// 初始化播放器和音效对象
void AudioManager::init()
{
    if (m_player) return;

    m_output = new QAudioOutput(this);
    m_output->setVolume(m_volume / 100.0);
    m_output->setMuted(m_muted);

    m_player = new QMediaPlayer(this);
    m_player->setAudioOutput(m_output);

    // 预创建并复用音效对象
    m_clickSfx = new QSoundEffect(this);
    m_clickSfx->setSource(QUrl("qrc:/audio/click.wav"));
    m_clickSfx->setVolume(m_volume / 100.0);

    m_clearSfx = new QSoundEffect(this);
    m_clearSfx->setSource(QUrl("qrc:/audio/clear.wav"));
    m_clearSfx->setVolume(m_volume / 100.0);
}

// 播放背景音乐，支持资源路径和本地文件路径
void AudioManager::playBgm(const QString& res)
{
    init();

    QUrl url;
    if (res.startsWith(":/")) {
        url = QUrl("qrc" + res); 
    }
    else {
        url = QUrl::fromLocalFile(res);
    }

    qDebug() << "[Audio] source =" << url;
    m_player->setSource(url);
    m_player->setLoops(QMediaPlayer::Infinite);
    m_player->play();
}

// 停止背景音乐
void AudioManager::stopBgm()
{
    if (m_player) m_player->stop();
}

// 设置音量，范围0~100，并应用到播放器和音效对象
void AudioManager::setVolume(int v)
{
    if (v < 0) v = 0;
    if (v > 100) v = 100;
    m_volume = v;

    if (m_output)   m_output->setVolume(m_volume / 100.0);
    if (m_clickSfx) m_clickSfx->setVolume(m_volume / 100.0);
    if (m_clearSfx) m_clearSfx->setVolume(m_volume / 100.0);

    saveSettings();
    emit audioStateChanged();
}

// 获取当前音量
int AudioManager::volume() const
{
    return m_volume;
}

// 设置静音状态，并应用到播放器
void AudioManager::setMuted(bool m)
{
    m_muted = m;
    if (m_output) m_output->setMuted(m_muted);
    saveSettings();
    emit audioStateChanged();
}

// 获取当前静音状态
bool AudioManager::isMuted() const
{
    return m_muted;
}

// 设置音效静音状态
void AudioManager::saveSettings()
{
    QSettings s("YourCompany", "LLK_Refresh");
    s.setValue("audio/volume", m_volume);
    s.setValue("audio/muted", m_muted);
    s.setValue("audio/sfxMuted", m_sfxMuted);
}

// 获取当前音效静音状态
void AudioManager::playClickSfx()
{
    if (m_sfxMuted) return;
    init();
    if (!m_clickSfx) return;

    if (m_clickSfx->isPlaying()) m_clickSfx->stop();
    m_clickSfx->play();
}

// 播放消除音效，允许高频触发
void AudioManager::playClearSfx()
{
    if (m_sfxMuted) return;
    init();
    if (!m_clearSfx) return;

    if (m_clearSfx->isPlaying()) m_clearSfx->stop();
    m_clearSfx->play();
}

// 设置音效静音状态
void AudioManager::setSfxMuted(bool m)
{
    if (m_sfxMuted == m) return;
    m_sfxMuted = m;
    saveSettings();
    emit audioStateChanged();
}

// 获取当前音效静音状态
bool AudioManager::isSfxMuted() const
{
    return m_sfxMuted;
}