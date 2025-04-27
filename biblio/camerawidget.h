#ifndef CAMERAWIDGET_H
#define CAMERAWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QImage>
#include <QPixmap>
#include "camerahandler.h"

namespace Ui {
class CameraWidget;
}

class CameraWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CameraWidget(QWidget *parent = nullptr);
    ~CameraWidget();

private slots:
    void updateFrame();
    void on_thresholdingButton_clicked();

private:
    Ui::CameraWidget *ui;
    CameraHandler cameraHandler;
    QTimer *timer;
    
    void showPlaceholderMessage(const QString &title, const QString &message);
    bool thresholdingEnabled = false; // Flag to enable/disable thresholding
};

#endif // CAMERAWIDGET_H
