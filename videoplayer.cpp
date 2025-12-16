#include "videoplayer.h"
#include <QRandomGenerator>


VideoPlayer::VideoPlayer(QWidget *parent) : QWidget(parent)
{
    // Initializing media player and audio output
    player = new QMediaPlayer(this);
    audioOutput = new QAudioOutput(this);
    player->setAudioOutput(audioOutput);

    videoWidget = new QVideoWidget(this);
    videoWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    videoWidget->setAspectRatioMode(Qt::KeepAspectRatio);
    player->setVideoOutput(videoWidget);

    // Initialize member variables
    currentIndex = -1;
    isShuffled = false;
    isPlaying = false;

    // Create UI elements
    setupUI();
    setupConnections();


}

VideoPlayer::~VideoPlayer() {
    delete player;
    delete audioOutput;
    delete videoWidget;
}

void VideoPlayer::setupUI() {
    // Control buttons
    btnAdd = new QPushButton("Add Videos", this);
    btnPlay = new QPushButton("Play", this);
    btnNext = new QPushButton("Next", this);
    btnPrev = new QPushButton("Previous", this);
    btnShuffle = new QPushButton("Shuffle: Off", this);
    btnStop = new QPushButton("Stop", this);
    btnRemove = new QPushButton("Remove", this);
    btnClear = new QPushButton("Clear All", this);

    // Set button icons (using standard Qt icons)
    btnPlay->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    btnStop->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
    btnNext->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));
    btnPrev->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));

    // Volume slider
    volumeSlider = new QSlider(Qt::Horizontal, this);
    volumeSlider->setRange(0, 100);
    volumeSlider->setValue(50);
    volumeSlider->setMaximumWidth(150);
    audioOutput->setVolume(0.5);

    QLabel *volumeLabel = new QLabel("Volume:", this);

    // Seek slider
    progressSlider = new QSlider(Qt::Horizontal, this);
    progressSlider->setRange(0, 0);

    // Time labels
    timeLabel = new QLabel("00:00 / 00:00", this);

    // Playlist widget
    playlistWidget = new QListWidget(this);
    playlistWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    playlistWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    // Status label
    statusLabel = new QLabel("Ready", this);
    statusLabel->setStyleSheet("QLabel { color: gray; font-style: italic; }");

    // Button layouts
    QHBoxLayout *controlLayout = new QHBoxLayout();
    controlLayout->addWidget(btnAdd);
    controlLayout->addWidget(btnRemove);
    controlLayout->addWidget(btnClear);
    controlLayout->addStretch();
    controlLayout->addWidget(btnPrev);
    controlLayout->addWidget(btnPlay);
    controlLayout->addWidget(btnStop);
    controlLayout->addWidget(btnNext);
    controlLayout->addStretch();
    controlLayout->addWidget(btnShuffle);
    controlLayout->addWidget(volumeLabel);
    controlLayout->addWidget(volumeSlider);
    controlLayout->setSpacing(10);
    controlLayout->setContentsMargins(5, 5, 5, 5);

    // Progress bar layout
    QHBoxLayout *seekLayout = new QHBoxLayout();
    seekLayout->addWidget(timeLabel);
    seekLayout->addWidget(progressSlider);
    seekLayout->setSpacing(10);

    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(videoWidget, 6);
    mainLayout->addLayout(seekLayout);
    mainLayout->addWidget(playlistWidget, 2);
    mainLayout->addWidget(statusLabel);
    mainLayout->addLayout(controlLayout);
    mainLayout->setSpacing(5);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    setLayout(mainLayout);
}

void VideoPlayer::setupConnections() {
    // Button connections
    connect(btnAdd, &QPushButton::clicked, this, &VideoPlayer::addFiles);
    connect(btnPlay, &QPushButton::clicked, this, &VideoPlayer::togglePlayPause);
    connect(btnStop, &QPushButton::clicked, this, &VideoPlayer::stop);
    connect(btnNext, &QPushButton::clicked, this, &VideoPlayer::next);
    connect(btnPrev, &QPushButton::clicked, this, &VideoPlayer::previous);
    connect(btnShuffle, &QPushButton::clicked, this, &VideoPlayer::toggleShuffle);
    connect(btnRemove, &QPushButton::clicked, this, &VideoPlayer::removeSelected);
    connect(btnClear, &QPushButton::clicked, this, &VideoPlayer::clearPlaylist);

    // Playlist connections
    connect(playlistWidget, &QListWidget::itemDoubleClicked, this, &VideoPlayer::handleItemDoubleClick);

    // Player connections
    connect(player, &QMediaPlayer::playbackStateChanged, this, &VideoPlayer::onPlaybackStateChanged);
    connect(player, &QMediaPlayer::durationChanged, this, &VideoPlayer::onDurationChanged);
    connect(player, &QMediaPlayer::positionChanged, this, &VideoPlayer::onPositionChanged);
    connect(player, &QMediaPlayer::mediaStatusChanged, this, &VideoPlayer::onMediaStatusChanged);
    connect(player, &QMediaPlayer::errorOccurred, this, &VideoPlayer::onError);

    // Slider connections
    connect(progressSlider, &QSlider::sliderMoved, this, &VideoPlayer::onSeekSliderMoved);
    connect(volumeSlider, &QSlider::valueChanged, this, &VideoPlayer::onVolumeChanged);
}

void VideoPlayer::addFiles() {
    QStringList files = QFileDialog::getOpenFileNames(
        this,
        "Select Videos",
        "",
        "Videos (*.mp4 *.avi *.mkv *.mov *.wmv *.flv *.webm)"
        );

    if (!files.isEmpty()) {
        for (const QString &file : files) {
            QFileInfo fileInfo(file);
            mediaPlayList.append(QUrl::fromLocalFile(file));
            playlistWidget->addItem(fileInfo.fileName());
        }

        if (currentIndex == -1 && !mediaPlayList.isEmpty()) {
            currentIndex = 0;
            highlightCurrentItem();
        }

        updateStatus(QString("Added %1 video(s)").arg(files.size()));
    }
}

void VideoPlayer::togglePlayPause() {
    if (mediaPlayList.isEmpty()) {
        updateStatus("No videos in playlist");
        return;
    }

    if (currentIndex < 0 || currentIndex >= mediaPlayList.size()) {
        currentIndex = 0;
    }

    if (player->playbackState() == QMediaPlayer::PlayingState) {
        player->pause();
    } else {
        if (player->source().isEmpty() || player->playbackState() == QMediaPlayer::StoppedState) {
            player->setSource(mediaPlayList[currentIndex]);
        }
        player->play();
    }
}

void VideoPlayer::stop() {
    player->stop();
    progressSlider->setValue(0);
    updateStatus("Stopped");
}

void VideoPlayer::next() {
    if (mediaPlayList.isEmpty()) return;

    if (isShuffled) {
        currentIndex = QRandomGenerator::global()->bounded(mediaPlayList.size());
    } else {
        currentIndex = (currentIndex + 1) % mediaPlayList.size();
    }

    highlightCurrentItem();
    player->setSource(mediaPlayList[currentIndex]);
    player->play();
}

void VideoPlayer::previous() {
    if (mediaPlayList.isEmpty()) return;

    if (isShuffled) {
        currentIndex = QRandomGenerator::global()->bounded(mediaPlayList.size());
    } else {
        currentIndex = (currentIndex - 1 + mediaPlayList.size()) % mediaPlayList.size();
    }

    highlightCurrentItem();
    player->setSource(mediaPlayList[currentIndex]);
    player->play();
}

void VideoPlayer::toggleShuffle() {
    isShuffled = !isShuffled;
    btnShuffle->setText(isShuffled ? "Shuffle: On" : "Shuffle: Off");
    btnShuffle->setStyleSheet(isShuffled ? "background-color: lightgreen;" : "");
    updateStatus(isShuffled ? "Shuffle mode enabled" : "Shuffle mode disabled");
}

void VideoPlayer::removeSelected() {
    int row = playlistWidget->currentRow();
    if (row >= 0 && row < mediaPlayList.size()) {
        mediaPlayList.removeAt(row);
        delete playlistWidget->takeItem(row);

        if (row == currentIndex) {
            player->stop();
            if (mediaPlayList.isEmpty()) {
                currentIndex = -1;
            } else if (currentIndex >= mediaPlayList.size()) {
                currentIndex = mediaPlayList.size() - 1;
            }
        } else if (row < currentIndex) {
            currentIndex--;
        }

        updateStatus("Video removed from playlist");
    }
}

void VideoPlayer::clearPlaylist() {
    player->stop();
    mediaPlayList.clear();
    playlistWidget->clear();
    currentIndex = -1;
    updateStatus("Playlist cleared");
}

void VideoPlayer::handleItemDoubleClick() {
    currentIndex = playlistWidget->currentRow();
    if (currentIndex >= 0 && currentIndex < mediaPlayList.size()) {
        player->setSource(mediaPlayList[currentIndex]);
        player->play();
    }
}

void VideoPlayer::onPlaybackStateChanged(QMediaPlayer::PlaybackState state) {
    switch (state) {
    case QMediaPlayer::PlayingState:
        btnPlay->setText("Pause");
        btnPlay->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
        isPlaying = true;
        updateStatus("Playing");
        break;
    case QMediaPlayer::PausedState:
        btnPlay->setText("Play");
        btnPlay->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        isPlaying = false;
        updateStatus("Paused");
        break;
    case QMediaPlayer::StoppedState:
        btnPlay->setText("Play");
        btnPlay->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        isPlaying = false;
        updateStatus("Stopped");
        break;
    }
}

void VideoPlayer::onDurationChanged(qint64 duration) {
    progressSlider->setRange(0, duration);
    updateTimeLabel();
}

void VideoPlayer::onPositionChanged(qint64 position) {
    if (!progressSlider->isSliderDown()) {
        progressSlider->setValue(position);
    }
    updateTimeLabel();
}

void VideoPlayer::onMediaStatusChanged(QMediaPlayer::MediaStatus status) {
    if (status == QMediaPlayer::EndOfMedia) {
        next(); // Auto-play next video
    }
}

void VideoPlayer::onSeekSliderMoved(int position) {
    player->setPosition(position);
}

void VideoPlayer::onVolumeChanged(int value) {
    audioOutput->setVolume(value / 100.0);
}

void VideoPlayer::onError(QMediaPlayer::Error error, const QString &errorString) {
    updateStatus(QString("Error: %1").arg(errorString));
}

void VideoPlayer::updateTimeLabel() {
    qint64 position = player->position();
    qint64 duration = player->duration();

    QString posStr = formatTime(position);
    QString durStr = formatTime(duration);

    timeLabel->setText(QString("%1 / %2").arg(posStr, durStr));
}

QString VideoPlayer::formatTime(qint64 milliseconds) {
    int seconds = (milliseconds / 1000) % 60;
    int minutes = (milliseconds / 60000) % 60;
    int hours = (milliseconds / 3600000);

    if (hours > 0) {
        return QString("%1:%2:%3")
            .arg(hours)
            .arg(minutes, 2, 10, QChar('0'))
            .arg(seconds, 2, 10, QChar('0'));
    } else {
        return QString("%1:%2")
            .arg(minutes, 2, 10, QChar('0'))
            .arg(seconds, 2, 10, QChar('0'));
    }
}

void VideoPlayer::highlightCurrentItem() {
    if (currentIndex >= 0 && currentIndex < playlistWidget->count()) {
        playlistWidget->setCurrentRow(currentIndex);
    }
}

void VideoPlayer::updateStatus(const QString &message) {
    statusLabel->setText(message);
}
