#include "gamewindow.h"
#include "ui_gamewindow.h"
#include "mainwindow.h"


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



