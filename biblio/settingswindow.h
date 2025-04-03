#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QWidget>
#include "camerawidget.h"

namespace Ui {
class SettingsWindow;
}

class SettingsWindow : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsWindow(QWidget *parent = nullptr);
    ~SettingsWindow();

private slots:
    void on_pushButton_clicked();

private:
    Ui::SettingsWindow *ui;
    CameraWidget *cameraWidget; // Assuming you have a CameraWidget class
};

#endif // SETTINGSWINDOW_H
