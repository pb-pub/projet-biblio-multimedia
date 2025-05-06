#include "gamewindow.h"
#include "ui_gamewindow.h"
#include "mainwindow.h"
#include "gameoverdialog.h" // Added include for GameOverDialog


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
            int finalScore = score; // Capture score before closing
            this->close(); // Close the current game window

            GameOverDialog gameOverDialog(finalScore, nullptr); // Pass the score
            gameOverDialog.exec(); // Show the dialog modally
            // Execution will resume here after the dialog is closed.
            // If "New Game" was clicked, a new GameWindow is already shown.
            // If "Exit" was clicked, the application is quitting.
        }
    });

    // Assurer que le label est visible au-dessus du GameWidget
    if (ui->label) {
        // Configurer le label pour qu'il soit au premier plan
        ui->label->raise();
        
        // Appliquer un style pour améliorer la visibilité
        ui->label->setStyleSheet(
            "QLabel { "
            "   color: white; "
            "   background-color: rgba(0, 0, 0, 150); " // Fond semi-transparent
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
        
        
        // Repositionner le label sur le dessus si nécessaire
        QTimer::singleShot(100, [this]() {
            ui->label->raise();
            updateLabelDisplay();
        });
    } 
}

GameWindow::~GameWindow()
{
    delete ui;
    delete gameWidget; // Clean up the game widget
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





