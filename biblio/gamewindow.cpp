#include "gamewindow.h"
#include "ui_gamewindow.h"
#include "mainwindow.h"
#include "gameoverdialog.h" 


#include <QVBoxLayout>
#include <qlabel.h>
#include <string>

GameWindow::GameWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::GameWindow)
{
    ui->setupUi(this);

    if (!ui->game->layout()) {
        ui->game->setLayout(new QVBoxLayout());
    }
    gameWidget = new GameWidget(this);
    ui->game->layout()->addWidget(gameWidget);
    
    // Connect the scoreIncreased signal to a slot that updates the score
    connect(gameWidget, &GameWidget::scoreIncreased, this, [this]() {
        // Increment score and update display
        score++;
        updateLabelDisplay();
    });
    
    // Connect the lifeDecrease signal to a slot that decreases lives
    connect(gameWidget, &GameWidget::lifeDecrease, this, [this]() {
        // Decrement lives and update display
        if (lives > 0) {
            lives--;
        }
        updateLabelDisplay();
        
        // Check if game over
        if (lives <= 0) {
            // Handle game over
            int finalScore = score; 
            gameWidget->close(); 
            this->close(); 

            GameOverDialog gameOverDialog(finalScore, nullptr);
            gameOverDialog.exec(); 
        }
    });

    if (ui->label) {
        ui->label->raise();
        
        ui->label->setStyleSheet(
            "QLabel { "
            "   color: white; "
            "   background-color: rgba(0, 0, 0, 150); " 
            "   border-radius: 10px; "
            "   padding: 5px; "
            "   font-size: 14pt; "
            "   font-weight: bold; "
            "}"
        );
        
        // S'assurer que le label est au premier plan
        ui->label->setParent(this);
        ui->label->setAttribute(Qt::WA_TransparentForMouseEvents);
        ui->label->show();
        QTimer::singleShot(100, [this]() {
            ui->label->raise();
            updateLabelDisplay();
        });
    } 
}

GameWindow::~GameWindow()
{
    delete ui;
    delete gameWidget;
}

void GameWindow::updateLabelDisplay()
{
    QString lives = "";
    for (int i = 0; i < this->lives; i++) {
        lives += "✘ ";
    }
 
    if (ui->label) {
        ui->label->setText(QString("<html><head/><body><p><span style=\" font-weight:700; font-style:italic; text-decoration: underline;\">Score : </span></p><p>%1</p><p><span style=\" color:#aa0000;\">"+ lives +"</span></p></body></html>").arg(score));
    }
}





