#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    videoPlayer = new VideoPlayer(this);
    setCentralWidget(videoPlayer);
    setWindowTitle("Video Playlist Player");

    QFile styleFile(":/style");
    if (styleFile.open(QFile::ReadOnly)) {
        QString style = QLatin1String(styleFile.readAll());
        this->setStyleSheet(style);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
