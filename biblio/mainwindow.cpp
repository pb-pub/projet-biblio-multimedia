#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "gamewindow.h"
#include "settingswindow.h"
#include <QDir>
#include <QDebug>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Charger la police personnalisée
    QStringList possibleFontPaths = {
        QCoreApplication::applicationDirPath() + "/../../../biblio/assets/fonts/NinjaStrike.otf",
        "./assets/fonts/NinjaStrike.otf",
        "../assets/fonts/NinjaStrike.otf",
        "../../assets/fonts/NinjaStrike.otf",
        "../../../assets/fonts/NinjaStrike.otf"
    };

    QString fontPath;
    bool foundFontPath = false;
    for (const auto &path : possibleFontPaths) {
        if (QFile::exists(path)) {
            fontPath = path;
            foundFontPath = true;
            qDebug() << "Found font path: " << fontPath;
            break;
        }
    }

    if (foundFontPath) {
        int fontId = QFontDatabase::addApplicationFont(fontPath);
        if (fontId != -1) {
            QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
            if (!fontFamilies.isEmpty()) {
                QFont ninjaFont(fontFamilies.first(), 24); // Ajustez la taille au besoin
                ui->pushButton->setFont(ninjaFont);
                ui->pushButton_2->setFont(ninjaFont);
                // Vous pouvez appliquer la police à d'autres widgets ici
            } else {
                qWarning() << "Could not find font family in" << fontPath;
            }
        } else {
            qWarning() << "Failed to load font from" << fontPath;
        }
    } else {
        qWarning() << "NinjaStrike.otf font file not found. Tried paths:";
        for (const auto &path : possibleFontPaths) {
            qWarning() << " - " << path;
        }
    }


    // Charger l'image de fond
    QStringList possibleImagePaths = {
        QCoreApplication::applicationDirPath() + "/../../../biblio/assets/textures/main_menu_background.jpg",
        "./assets/textures/main_menu_background.jpg",
        "../assets/textures/main_menu_background.jpg",
        "../../assets/textures/main_menu_background.jpg",
        "../../../assets/textures/main_menu_background.jpg"
    };
    
    QString imagePath;
    bool foundImagePath = false;
    for (const auto &path : possibleImagePaths) {
        if (QFile::exists(path)) {
            imagePath = path;
            foundImagePath = true;
            qDebug() << "Found background image path: " << imagePath;
            break;
        }
    }

    if (foundImagePath) {
        if (!m_backgroundImage.load(imagePath)) {
            qWarning() << "Failed to load background image from" << imagePath;
        }
    } else {
        qWarning() << "Background image main_menu_background.jpg not found. Tried paths:";
        for (const auto &path : possibleImagePaths) {
            qWarning() << " - " << path;
        }
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    QMainWindow::paintEvent(event); // Important: call base class implementation
    if (!m_backgroundImage.isNull()) {
        QPainter painter(this);
        // Scale the image to fit the window size, maintaining aspect ratio
        QPixmap scaledPixmap = m_backgroundImage.scaled(this->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        // Center the image
        int x = (this->width() - scaledPixmap.width()) / 2;
        int y = (this->height() - scaledPixmap.height()) / 2;
        painter.drawPixmap(x, y, scaledPixmap);
    }
}

void MainWindow::on_pushButton_2_clicked()
{
    this->close(); 
    GameWindow* gameWindow = new GameWindow();
    gameWindow->show();
}


void MainWindow::on_pushButton_clicked()
{
    this->close();
    SettingsWindow* settingsWindow = new SettingsWindow();
    settingsWindow->show();
}


void MainWindow::on_pushButton_3_clicked()
{
    // Chercher le fichier PDF dans les dossiers assets
    QStringList possiblePdfPaths = {
        QCoreApplication::applicationDirPath() + "/../../../biblio/assets/Rapport_BDM.pdf",
        "./assets/Rapport_BDM.pdf",
        "../assets/Rapport_BDM.pdf",
        "../../assets/Rapport_BDM.pdf",
        "../../../assets/Rapport_BDM.pdf"
    };
    
    QString pdfPath;
    bool foundPdfPath = false;
    for (const auto &path : possiblePdfPaths) {
        if (QFile::exists(path)) {
            pdfPath = path;
            foundPdfPath = true;
            qDebug() << "Found PDF path: " << pdfPath;
            break;
        }
    }

    if (foundPdfPath) {
        QUrl pdfUrl = QUrl::fromLocalFile(QFileInfo(pdfPath).absoluteFilePath());
        if (!QDesktopServices::openUrl(pdfUrl)) {
            QMessageBox::warning(this, "Erreur", "Impossible d'ouvrir le fichier PDF.\nVérifiez qu'un lecteur PDF est installé.");
        }
    } else {
        QMessageBox::information(this, "Fichier non trouvé", "Le fichier PDF n'a pas été trouvé dans le dossier assets.");
        qDebug() << "PDF file not found. Tried paths:";
        for (const auto &path : possiblePdfPaths) {
            qDebug() << " - " << path;
        }
    }
}

