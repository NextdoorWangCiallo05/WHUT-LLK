#include "audiomanager.h"
#include <QSettings>
#include <QSoundEffect>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QFileInfo>
#include <QDebug>
#include <QUrl>

AudioManager::AudioManager(QObject* parent)
    : QObject(parent),
    m_player(nullptr),
    m_output(nullptr),
    m_volume(50),
    m_muted(false),
    m_sfxMuted(false)
{
    QSettings s("YourCompany", "LLK_Refresh");
    m_volume = s.value("audio/volume", 50).toInt();
    m_muted = s.value("audio/muted", false).toBool();
    m_sfxMuted = s.value("audio/sfxMuted", false).toBool();
}

AudioManager& AudioManager::instance()
{
    static AudioManager ins;
    return ins;
}

void AudioManager::init()
{
    if (m_player) return;

    m_output = new QAudioOutput(this);
    m_output->setVolume(m_volume / 100.0);
    m_output->setMuted(m_muted);

    m_player = new QMediaPlayer(this);
    m_player->setAudioOutput(m_output);
}

void AudioManager::playBgm(const QString& res)
{
    init();

    QUrl url;
    if (res.startsWith(":/")) {
        // qrc 资源必须用 qrc scheme
        url = QUrl("qrc" + res);   // 例如 qrc:/audio/bgm.mp3
    }
    else {
        url = QUrl::fromLocalFile(res);
    }

    qDebug() << "[Audio] source =" << url;
    m_player->setSource(url);
    m_player->setLoops(QMediaPlayer::Infinite);
    m_player->play();
}

void AudioManager::stopBgm()
{
    if (m_player) m_player->stop();
}

void AudioManager::setVolume(int v)
{
    if (v < 0) v = 0;
    if (v > 100) v = 100;
    m_volume = v;
    if (m_output) m_output->setVolume(m_volume / 100.0);
    saveSettings();              // 新增
    emit audioStateChanged();
}

int AudioManager::volume() const
{
    return m_volume;
}

void AudioManager::setMuted(bool m)
{
    m_muted = m;
    if (m_output) m_output->setMuted(m_muted);
    saveSettings();              // 新增
    emit audioStateChanged();
}

bool AudioManager::isMuted() const
{
    return m_muted;
}

void AudioManager::saveSettings()
{
    QSettings s("YourCompany", "LLK_Refresh");
    s.setValue("audio/volume", m_volume);
    s.setValue("audio/muted", m_muted);
    s.setValue("audio/sfxMuted", m_sfxMuted);
}

void AudioManager::playClickSfx()
{
    if (m_sfxMuted) return;

    auto* s = new QSoundEffect(this);
    s->setSource(QUrl("qrc:/audio/click.wav"));
    s->setVolume(m_volume / 100.0); // 跟随音量，独立于BGM静音
    connect(s, &QSoundEffect::playingChanged, s, [s]() {
        if (!s->isPlaying()) s->deleteLater();
        });
    s->play();
}

void AudioManager::playClearSfx()
{
    if (m_sfxMuted) return;

    auto* s = new QSoundEffect(this);
    s->setSource(QUrl("qrc:/audio/clear.wav"));
    s->setVolume(m_volume / 100.0);
    connect(s, &QSoundEffect::playingChanged, s, [s]() {
        if (!s->isPlaying()) s->deleteLater();
        });
    s->play();
}

void AudioManager::setSfxMuted(bool m)
{
    if (m_sfxMuted == m) return;
    m_sfxMuted = m;
    saveSettings();
    emit audioStateChanged();
}

bool AudioManager::isSfxMuted() const
{
    return m_sfxMuted;
}