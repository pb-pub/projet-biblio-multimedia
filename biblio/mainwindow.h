/**
 * @file mainwindow.h
 * @brief Déclaration de la classe MainWindow.
 * @author Boutet Paul, El Gote Ismaïl
 */
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPainter> // Added for paintEvent
#include <QStyleOption> // Added for paintEvent
#include <QStyle> // Added for paintEvent
#include <QFontDatabase> // Added for custom font

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

/**
 * @class MainWindow
 * @brief Fenêtre principale de l'application, servant de menu d'accueil.
 *
 * Cette classe est la première fenêtre affichée à l'utilisateur. Elle permet typiquement
 * de lancer une nouvelle partie, d'accéder aux paramètres, ou de quitter l'application.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Constructeur de MainWindow.
     * @param parent Widget parent, nullptr par défaut.
     * Initialise l'interface utilisateur de la fenêtre principale.
     */
    MainWindow(QWidget *parent = nullptr);

    /**
     * @brief Destructeur de MainWindow.
     * Libère les ressources allouées, notamment l'objet ui.
     */
    ~MainWindow();

protected: // Changed to protected to allow overriding paintEvent
    /**
     * @brief Gère les événements de peinture pour dessiner l'image de fond.
     * @param event Événement de peinture.
     */
    void paintEvent(QPaintEvent *event) override;

private slots:
    /**
     * @brief Slot appelé lors du clic sur le deuxième bouton poussoir (pushButton_2).
     * Permet d'ouvrir une fenètre de jeu.
     */
    void on_pushButton_2_clicked();

    /**
     * @brief Slot appelé lors du clic sur le premier bouton poussoir (pushButton).
     * Permet d'ouvrir une fenètre de paramètres.
     */
    void on_pushButton_clicked();

    /**
     * @brief Slot appelé lors du clic sur le troisième bouton poussoir (pushButton_3).
     * Permet d'ouvrir le rapport.
     */
    void on_pushButton_3_clicked();

private:
    Ui::MainWindow *ui; ///< Pointeur vers l'objet d'interface utilisateur généré par Qt Designer.
    QPixmap m_backgroundImage; ///< Image de fond.
};
#endif // MAINWINDOW_H
