#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <QObject>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QFileInfo>

class AudioPlayer : public QObject
{
    Q_OBJECT

public:
    explicit AudioPlayer(QObject *parent = nullptr);
    ~AudioPlayer();

    // 播放控制
    void play(const QString &filePath);
    void pause();
    void resume();
    void stop();
    void setVolume(int volume); // 0-100

    // 状态查询
    bool isPlaying() const;
    bool isPaused() const;
    bool isStopped() const;
    QString currentFilePath() const;
    int duration() const; // 总时长(毫秒)
    int position() const; // 当前位置(毫秒)

    // 进度控制
    void setPosition(int position);

signals:
    void playbackStarted();
    void playbackPaused();
    void playbackStopped();
    void playbackFinished();
    void positionChanged(int position);
    void durationChanged(int duration);
    void errorOccurred(const QString &errorMessage);

private slots:
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void onPlaybackStateChanged(QMediaPlayer::PlaybackState state);
    void onPositionChanged(qint64 position);
    void onDurationChanged(qint64 duration);
    void onErrorOccurred(QMediaPlayer::Error error, const QString &errorString);

private:
    QMediaPlayer *m_mediaPlayer;
    QAudioOutput *m_audioOutput;
    QString m_currentFilePath;
};

#endif // AUDIOPLAYER_H
