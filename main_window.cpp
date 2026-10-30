#include "main_window.h"
#include <QDebug>
#include <QTime>
#include <QDateTime>
#include <QThread>
#include <windows.h>
#include <winuser.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    isRunning = false;
    isSelectingArea = false;
    isTemplateLoaded = false;
    lastMatchValue = 0.0;
    lastCastTime = 0;
    lastCatchTime = 0;

    setupUI();
    setupConnections();
    statusLabel->setText("Status: Ready");
}

MainWindow::~MainWindow()
{
    stopFishing();
}

void MainWindow::setupUI()
{
    setWindowTitle("MC Fishing Bot");
    setGeometry(100, 100, 400, 300);
    setMinimumSize(400, 300);

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    startButton = new QPushButton("Start", this);
    startButton->setMinimumHeight(40);
    buttonLayout->addWidget(startButton);

    stopButton = new QPushButton("Stop", this);
    stopButton->setMinimumHeight(40);
    stopButton->setEnabled(false);
    buttonLayout->addWidget(stopButton);

    selectAreaButton = new QPushButton("Select Area", this);
    selectAreaButton->setMinimumHeight(40);
    buttonLayout->addWidget(selectAreaButton);

    loadTemplateButton = new QPushButton("Load Template", this);
    loadTemplateButton->setMinimumHeight(40);
    buttonLayout->addWidget(loadTemplateButton);

    mainLayout->addLayout(buttonLayout);

    statusLabel = new QLabel("Status: Ready", this);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setMinimumHeight(30);
    statusLabel->setStyleSheet("background-color: #f0f0f0; border-radius: 4px; padding: 5px;");
    mainLayout->addWidget(statusLabel);

    QHBoxLayout *matchLayout = new QHBoxLayout();
    matchLayout->addWidget(new QLabel("Match:", this));

    matchValueLabel = new QLabel("0.00", this);
    matchValueLabel->setAlignment(Qt::AlignCenter);
    matchValueLabel->setMinimumWidth(60);
    matchLayout->addWidget(matchValueLabel);

    matchProgressBar = new QProgressBar(this);
    matchProgressBar->setRange(0, 100);
    matchProgressBar->setValue(0);
    matchProgressBar->setMinimumHeight(20);
    matchLayout->addWidget(matchProgressBar);

    mainLayout->addLayout(matchLayout);

    timeLabel = new QLabel("Last: --:--:-- | Wait: 0s", this);
    timeLabel->setAlignment(Qt::AlignCenter);
    timeLabel->setMinimumHeight(30);
    timeLabel->setStyleSheet("background-color: #f0f0f0; border-radius: 4px; padding: 5px;");
    mainLayout->addWidget(timeLabel);

    selectionOverlay = new QWidget(nullptr, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    selectionOverlay->setAttribute(Qt::WA_TranslucentBackground);
    selectionOverlay->installEventFilter(this);
    selectionOverlay->hide();

    captureTimer = new QTimer(this);
    captureTimer->setInterval(200);
    captureTimer->setTimerType(Qt::PreciseTimer);

    statusUpdateTimer = new QTimer(this);
    statusUpdateTimer->setInterval(1000);
}

void MainWindow::setupConnections()
{
    connect(startButton, &QPushButton::clicked, this, &MainWindow::startFishing);
    connect(stopButton, &QPushButton::clicked, this, &MainWindow::stopFishing);
    connect(selectAreaButton, &QPushButton::clicked, this, &MainWindow::selectArea);
    connect(loadTemplateButton, &QPushButton::clicked, this, &MainWindow::loadTemplate);
    connect(captureTimer, &QTimer::timeout, this, &MainWindow::processFrame);
    connect(statusUpdateTimer, &QTimer::timeout, this, &MainWindow::updateTimeDisplay);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == selectionOverlay && isSelectingArea)
    {
        if (event->type() == QEvent::MouseButtonPress)
        {
            QMouseEvent *e = static_cast<QMouseEvent*>(event);
            selectionStart = e->globalPosition().toPoint();
            selectionEnd = selectionStart;
            selectionOverlay->update();
            return true;
        }
        else if (event->type() == QEvent::MouseMove)
        {
            QMouseEvent *e = static_cast<QMouseEvent*>(event);
            selectionEnd = e->globalPosition().toPoint();
            selectionOverlay->update();
            return true;
        }
        else if (event->type() == QEvent::MouseButtonRelease)
        {
            QMouseEvent *e = static_cast<QMouseEvent*>(event);
            selectionEnd = e->globalPosition().toPoint();

            selectedArea = QRect(selectionStart, selectionEnd).normalized();

            QApplication::restoreOverrideCursor();
            selectionOverlay->hide();
            this->show();
            isSelectingArea = false;
            statusLabel->setText("Status: Area selected");
            return true;
        }
        else if (event->type() == QEvent::Paint)
        {
            QPainter p(selectionOverlay);
            p.fillRect(selectionOverlay->rect(), QColor(0,0,0,140));
            QRect r = QRect(selectionStart, selectionEnd).normalized();
            p.setPen(QPen(Qt::red, 3));
            p.drawRect(r);
            return true;
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::startFishing()
{
    if (!isTemplateLoaded) {
        QMessageBox::warning(this, "Warning", "Load template first!");
        return;
    }
    if (selectedArea.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Select area first!");
        return;
    }

    isRunning = true;
    startButton->setEnabled(false);
    stopButton->setEnabled(true);
    selectAreaButton->setEnabled(false);
    loadTemplateButton->setEnabled(false);

    lastCastTime = QDateTime::currentMSecsSinceEpoch();
    statusLabel->setText("Status: Running");

    captureTimer->start();
    statusUpdateTimer->start();
    simulateRightClick();
    statusLabel->setText("Status: Fishing...");
}

void MainWindow::stopFishing()
{
    isRunning = false;
    captureTimer->stop();
    statusUpdateTimer->stop();

    startButton->setEnabled(true);
    stopButton->setEnabled(false);
    selectAreaButton->setEnabled(true);
    loadTemplateButton->setEnabled(true);

    statusLabel->setText("Status: Stopped");
    matchValueLabel->setText("0.00");
    matchProgressBar->setValue(0);
}

void MainWindow::selectArea()
{
    if (isRunning) {
        QMessageBox::warning(this, "Warning", "Stop first!");
        return;
    }

    isSelectingArea = true;
    QRect g = QApplication::primaryScreen()->geometry();
    selectionOverlay->setGeometry(g);
    selectionOverlay->show();
    this->hide();
    QApplication::setOverrideCursor(Qt::CrossCursor);
}

void MainWindow::loadTemplate()
{
    QString path = QFileDialog::getOpenFileName(this, "Load Image", "", "Images (*.png *.jpg *.bmp)");
    if (path.isEmpty()) return;

    templateImage = cv::imread(path.toStdString(), cv::IMREAD_COLOR);
    if (templateImage.empty()) {
        QMessageBox::warning(this, "Error", "Failed to load!");
        isTemplateLoaded = false;
    } else {
        isTemplateLoaded = true;
        statusLabel->setText("Status: Template loaded");
    }
}

void MainWindow::processFrame()
{
    if (!isRunning)
        return;

    qint64 now = QDateTime::currentMSecsSinceEpoch();
    if (now - lastCastTime < 4500)
        return;
    if (now - lastCastTime > 60000) {
        simulateRightClick();
        lastCastTime = now;
        statusLabel->setText("Status: Timeout, re-fish");
        return;
    }

    int x = selectedArea.x();
    int y = selectedArea.y();
    int w = selectedArea.width();
    int h = selectedArea.height();

    if (w <= 0 || h <= 0)
        return;

    HDC hScreen = GetDC(NULL);
    HDC hDC = CreateCompatibleDC(hScreen);
    HBITMAP hBmp = CreateCompatibleBitmap(hScreen, w, h);
    HGDIOBJ old = SelectObject(hDC, hBmp);

    BitBlt(hDC, 0, 0, w, h, hScreen, x, y, SRCCOPY);

    BITMAPINFO bmi = {0};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = w;
    bmi.bmiHeader.biHeight = -h;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 24;
    bmi.bmiHeader.biCompression = BI_RGB;

    cv::Mat shot(h, w, CV_8UC3);
    GetDIBits(hDC, hBmp, 0, h, shot.data, &bmi, DIB_RGB_COLORS);

    SelectObject(hDC, old);
    DeleteObject(hBmp);
    DeleteDC(hDC);
    ReleaseDC(NULL, hScreen);

    if (shot.empty())
        return;

    double val = matchTemplate(shot);
    lastMatchValue = val;

    matchValueLabel->setText(QString::number(val, 'f', 2));
    matchProgressBar->setValue(qBound(0, (int)(val * 100), 100));

    if (val > 0.65) {
        simulateRightClick();
        lastCastTime = QDateTime::currentMSecsSinceEpoch();
        statusLabel->setText("Status: Caught! Fishing...");
    }
}

double MainWindow::matchTemplate(const cv::Mat &src)
{
    if (templateImage.empty() || src.empty())
        return 0.0;
    if (templateImage.cols > src.cols || templateImage.rows > src.rows)
        return 0.0;

    cv::Mat res;
    cv::matchTemplate(src, templateImage, res, cv::TM_CCOEFF_NORMED);
    double minVal, maxVal;
    cv::minMaxLoc(res, &minVal, &maxVal);
    return maxVal;
}

void MainWindow::simulateRightClick()
{
    mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0);
    QThread::msleep(20);
    mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);
}

void MainWindow::updateTimeDisplay()
{
    if (!isRunning) return;
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    qint64 elapsed = (now - lastCastTime) / 1000;
    QTime t = QDateTime::fromMSecsSinceEpoch(lastCastTime).time();
    timeLabel->setText(QString("Last: %1 | Wait: %2s").arg(t.toString("HH:mm:ss")).arg(elapsed));
}

void MainWindow::mousePressEvent(QMouseEvent *e)   {}
void MainWindow::mouseMoveEvent(QMouseEvent *e)    {}
void MainWindow::mouseReleaseEvent(QMouseEvent *e) {}