#include "gameoverdialog.h"
#include "gamewindow.h" // Required to start a new game

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QApplication> // Required for qApp->quit()
#include <QFont>

GameOverDialog::GameOverDialog(int score, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Game Over");

    gameOverLabel = new QLabel("GAME OVER", this);
    QFont gameOverFont = gameOverLabel->font();
    gameOverFont.setPointSize(24);
    gameOverFont.setBold(true);
    gameOverLabel->setFont(gameOverFont);
    gameOverLabel->setAlignment(Qt::AlignCenter);

    scoreLabel = new QLabel(QString("Final Score: %1").arg(score), this);
    QFont scoreFont = scoreLabel->font();
    scoreFont.setPointSize(16);
    scoreLabel->setFont(scoreFont);
    scoreLabel->setAlignment(Qt::AlignCenter);

    newGameButton = new QPushButton("New Game", this);
    exitButton = new QPushButton("Exit", this);

    connect(newGameButton, &QPushButton::clicked, this, &GameOverDialog::onNewGameClicked);
    connect(exitButton, &QPushButton::clicked, this, &GameOverDialog::onExitClicked);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(gameOverLabel);
    layout->addWidget(scoreLabel);
    layout->addWidget(newGameButton);
    layout->addWidget(exitButton);
    setLayout(layout);

    // Set a fixed size for the dialog or make it adjust to content
    setFixedSize(300, 250); // Adjusted size for better spacing
}

GameOverDialog::~GameOverDialog()
{
}

void GameOverDialog::onNewGameClicked()
{
    accept(); 
    GameWindow *gameWindow = new GameWindow();
    gameWindow->show();
}

void GameOverDialog::onExitClicked()
{
    QApplication::quit();

}
