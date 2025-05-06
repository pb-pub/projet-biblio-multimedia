#ifndef GAMEOVERDIALOG_H
#define GAMEOVERDIALOG_H

#include <QDialog>

class QLabel;
class QPushButton;

class GameOverDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GameOverDialog(int score, QWidget *parent = nullptr);
    ~GameOverDialog();

private slots:
    void onNewGameClicked();
    void onExitClicked();

private:
    QLabel *gameOverLabel;
    QLabel *scoreLabel;
    QPushButton *newGameButton;
    QPushButton *exitButton;
};

#endif // GAMEOVERDIALOG_H
