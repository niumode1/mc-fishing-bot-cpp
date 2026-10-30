#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QTimer>
#include <QScreen>
#include <QPixmap>
#include <QRect>
#include <QMouseEvent>
#include <QPainter>
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>

#include <opencv2/opencv.hpp>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    QPushButton *startButton;
    QPushButton *stopButton;
    QPushButton *selectAreaButton;
    QPushButton *loadTemplateButton;
    QLabel *statusLabel;
    QLabel *matchValueLabel;
    QProgressBar *matchProgressBar;
    QLabel *timeLabel;

    bool isRunning;
    bool isSelectingArea;
    bool isTemplateLoaded;
    QRect selectedArea;
    QPoint selectionStart;
    QPoint selectionEnd;
    double lastMatchValue;
    qint64 lastCastTime;
    qint64 lastCatchTime;

    QTimer *captureTimer;
    QTimer *statusUpdateTimer;

    cv::Mat templateImage;
    QWidget *selectionOverlay;

    void setupUI();
    void setupConnections();
    double matchTemplate(const cv::Mat &screenshot);
    void simulateRightClick();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void startFishing();
    void stopFishing();
    void selectArea();
    void loadTemplate();
    void processFrame();
    void updateTimeDisplay();
};

#endif