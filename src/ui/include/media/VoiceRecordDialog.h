#ifndef VOICERECORDDIALOG_H
#define VOICERECORDDIALOG_H

#include <QDialog>
#include <QTimer>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>
#include <QFile>
#include <QDebug>
#include <QMediaRecorder>
#include <QAudioDevice>
#include <QMediaFormat>
#include <QMediaCaptureSession>
#include <QAudioInput>
#include <QMediaPlayer>
#include <QAudioOutput>

class VoiceRecordDialog : public QDialog
{
    Q_OBJECT

public:
    explicit VoiceRecordDialog(QWidget *parent = nullptr);
    ~VoiceRecordDialog();

    QString getRecordedFilePath() const { return m_recordedWavPath; }
    int getAudioDuration() const { return m_audioDuration; }

    // 静态方法：转换WAV到MP3
    static QString convertToMp3(const QString &wavPath, QString *errorMsg = nullptr);

private slots:
    void startRecording();
    void stopRecording();
    void cancelRecording();
    void updateTimer();
    void onRecordingStateChanged(QMediaRecorder::RecorderState state);
    void onRecordingError(QMediaRecorder::Error error, const QString &errorString);

private:
    void setupUI();
    void setupAudioRecorder();
    bool ensureDirectoryExists();
    QString generateFileName(const QString &extension);
    bool calculateAudioDuration();
    void updateUIForRecording();
    void updateUIForReady();

private:
    // UI组件
    QLabel *m_statusLabel;
    QLabel *m_timerLabel;
    QPushButton *m_cancelButton;
    QPushButton *m_confirmButton;
    QTimer *m_recordTimer;

    // 录音相关
    QMediaCaptureSession *m_captureSession;
    QMediaRecorder *m_mediaRecorder;
    QAudioInput *m_audioInput;
    QMediaPlayer *m_mediaPlayer;
    QAudioOutput *m_audioOutput;
    QString m_audioBaseDir;
    QString m_wavDir;
    QString m_mp3Dir;
    QString m_recordedWavPath;

    // 状态变量
    int m_recordSeconds;
    int m_audioDuration;
    bool m_isRecording;
};

#endif // VOICERECORDDIALOG_H
