#include "AudioPlayer.h"
#include <QDebug>

AudioPlayer::AudioPlayer(QObject *parent)
    : QObject(parent)
    , m_currentFilePath("")
{
    // 创建媒体播放器和音频输出
    m_mediaPlayer = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);

    // 设置音频输出
    m_mediaPlayer->setAudioOutput(m_audioOutput);

    // 设置初始音量
    m_audioOutput->setVolume(0.5); // 50% 音量

    // 连接信号槽
    connect(m_mediaPlayer, &QMediaPlayer::mediaStatusChanged,
            this, &AudioPlayer::onMediaStatusChanged);
    connect(m_mediaPlayer, &QMediaPlayer::playbackStateChanged,
            this, &AudioPlayer::onPlaybackStateChanged);
    connect(m_mediaPlayer, &QMediaPlayer::positionChanged,
            this, &AudioPlayer::onPositionChanged);
    connect(m_mediaPlayer, &QMediaPlayer::durationChanged,
            this, &AudioPlayer::onDurationChanged);
    connect(m_mediaPlayer, &QMediaPlayer::errorOccurred,
            this, &AudioPlayer::onErrorOccurred);
}

AudioPlayer::~AudioPlayer()
{
    stop();
}

void AudioPlayer::play(const QString &filePath)
{
    if (filePath.isEmpty()) {
        emit errorOccurred("文件路径为空");
        return;
    }

    if (!QFile::exists(filePath)) {
        emit errorOccurred("文件不存在: " + filePath);
        return;
    }
    if(m_currentFilePath == filePath) {
        stop();
        return;
    }

    // 如果正在播放
    if (m_mediaPlayer->playbackState() != QMediaPlayer::StoppedState) {
        stop();
    }

    // 设置新的媒体源
    if (m_currentFilePath != filePath) {
        m_mediaPlayer->setSource(QUrl::fromLocalFile(filePath));
        m_currentFilePath = filePath;
    }

    // 开始播放
    m_mediaPlayer->play();
}

void AudioPlayer::pause()
{
    if (m_mediaPlayer->playbackState() == QMediaPlayer::PlayingState) {
        m_mediaPlayer->pause();
    }
}

void AudioPlayer::resume()
{
    if (m_mediaPlayer->playbackState() == QMediaPlayer::PausedState) {
        m_mediaPlayer->play();
    }
}

void AudioPlayer::stop()
{
    m_mediaPlayer->stop();
    m_currentFilePath.clear();
}

void AudioPlayer::setVolume(int volume)
{
    // 将0-100的整数音量转换为0.0-1.0的浮点数
    float normalizedVolume = qBound(0, volume, 100) / 100.0f;
    m_audioOutput->setVolume(normalizedVolume);
}

bool AudioPlayer::isPlaying() const
{
    return m_mediaPlayer->playbackState() == QMediaPlayer::PlayingState;
}

bool AudioPlayer::isPaused() const
{
    return m_mediaPlayer->playbackState() == QMediaPlayer::PausedState;
}

bool AudioPlayer::isStopped() const
{
    return m_mediaPlayer->playbackState() == QMediaPlayer::StoppedState;
}

QString AudioPlayer::currentFilePath() const
{
    return m_currentFilePath;
}

int AudioPlayer::duration() const
{
    return m_mediaPlayer->duration();
}

int AudioPlayer::position() const
{
    return m_mediaPlayer->position();
}

void AudioPlayer::setPosition(int position)
{
    m_mediaPlayer->setPosition(position);
}

void AudioPlayer::onMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    switch (status) {
    case QMediaPlayer::LoadedMedia:
        qDebug() << "媒体加载完成:" << m_currentFilePath;
        break;
    case QMediaPlayer::EndOfMedia:
        emit playbackFinished();
        qDebug() << "播放完成:" << m_currentFilePath;
        m_currentFilePath = QString();
        break;
    case QMediaPlayer::InvalidMedia:
        emit errorOccurred("无效的媒体文件: " + m_currentFilePath);
        break;
    case QMediaPlayer::NoMedia:
        // 正常状态，无需处理
        break;
    default:
        break;
    }
}

void AudioPlayer::onPlaybackStateChanged(QMediaPlayer::PlaybackState state)
{
    switch (state) {
    case QMediaPlayer::PlayingState:
        emit playbackStarted();
        break;
    case QMediaPlayer::PausedState:
        emit playbackPaused();
        break;
    case QMediaPlayer::StoppedState:
        emit playbackStopped();
        break;
    }
}

void AudioPlayer::onPositionChanged(qint64 position)
{
    emit positionChanged(position);
}

void AudioPlayer::onDurationChanged(qint64 duration)
{
    emit durationChanged(duration);
}

void AudioPlayer::onErrorOccurred(QMediaPlayer::Error error, const QString &errorString)
{
    QString errorMsg;
    switch (error) {
    case QMediaPlayer::NoError:
        return;
    case QMediaPlayer::ResourceError:
        errorMsg = "资源错误: " + errorString;
        break;
    case QMediaPlayer::FormatError:
        errorMsg = "格式错误: " + errorString;
        break;
    case QMediaPlayer::NetworkError:
        errorMsg = "网络错误: " + errorString;
        break;
    case QMediaPlayer::AccessDeniedError:
        errorMsg = "访问被拒绝: " + errorString;
        break;
    default:
        errorMsg = "未知错误: " + errorString;
        break;
    }

    emit errorOccurred(errorMsg);
    qWarning() << "音频播放错误:" << errorMsg;
}
