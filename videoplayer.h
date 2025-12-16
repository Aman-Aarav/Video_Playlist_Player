#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QWidget>
#include <QHBoxLayout>
#include <QListWidget>
#include <QFileDialog>
#include <QSlider>
#include <QLabel>
#include <QUrl>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QAudioOutput>
#include <QPushButton>
#include <QVBoxLayout>


class VideoPlayer : public QWidget {
    Q_OBJECT

public:
    explicit VideoPlayer(QWidget *parent = nullptr);
    ~VideoPlayer();

private slots:
    // button actions slots
    void addFiles();
    void togglePlayPause();
    void stop();
    void next();
    void previous();
    void toggleShuffle();
    void removeSelected();
    void clearPlaylist();
    void handleItemDoubleClick();

    // Player events handlers
    void onPlaybackStateChanged(QMediaPlayer::PlaybackState state);
    void onDurationChanged(qint64 duration);
    void onPositionChanged(qint64 position);
    void onMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void onSeekSliderMoved(int position);
    void onVolumeChanged(int value);
    void onError(QMediaPlayer::Error error, const QString &errorString);

private:
    // UI setup methods
    void setupUI();
    void setupConnections();

    // utility methods
    void updateTimeLabel();
    QString formatTime(qint64 milliseconds);
    void highlightCurrentItem();
    void updateStatus(const QString &message);
    // all UI elements
    QPushButton *btnAdd;
    QPushButton *btnPlay;
    QPushButton *btnStop;
    QPushButton *btnNext;
    QPushButton *btnPrev;
    QPushButton *btnShuffle;
    QPushButton *btnRemove;
    QPushButton *btnClear;
    QListWidget *playlistWidget;
    QSlider *progressSlider;
    QSlider *volumeSlider;
    QLabel *timeLabel;
    QLabel *statusLabel;

    QList<QUrl> mediaPlayList;
    int currentIndex;
    bool isShuffled;
    bool isPlaying;
    // Media elements
    QMediaPlayer *player;
    QAudioOutput *audioOutput;
    QVideoWidget *videoWidget;
};

#endif // VIDEOPLAYER_H
