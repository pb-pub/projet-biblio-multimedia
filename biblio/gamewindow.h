/**
 * @file gamewindow.h
 * @brief Déclaration de la classe GameWindow.
 * @author Boutet Paul, El Gote Ismaïl 
 */

#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include <QMainWindow>
#include "gamewidget.h" 
namespace Ui {
class GameWindow;
}

/**
 * @class GameWindow
 * @brief Fenêtre principale de l'application de jeu.
 *
 * Cette classe hérite de QMainWindow et sert de conteneur principal pour les widgets du jeu,
 * tels que GameWidget. Elle gère également l'affichage du score et des vies.
 */
class GameWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Constructeur de GameWindow.
     * @param parent Widget parent, nullptr par défaut.
     * Initialise l'interface utilisateur et configure les connexions de signaux/slots.
     */
    explicit GameWindow(QWidget *parent = nullptr);

    /**
     * @brief Destructeur de GameWindow.
     * Libère les ressources allouées, notamment l'objet ui.
     */
    ~GameWindow();


private:
    int lives = 3;          ///< Nombre de vies restantes pour le joueur. Initialisé à 3.
    int score = 0;          ///< Score actuel du joueur. Initialisé à 0.
    GameWidget* gameWidget; ///< Pointeur vers le widget principal du jeu (où se déroule l'action 3D).
    Ui::GameWindow *ui; ///< Pointeur vers l'objet d'interface utilisateur généré par Qt Designer.

    /**
     * @brief Met à jour l'affichage des étiquettes (score et vies) dans l'interface utilisateur.
     * Cette méthode est appelée lorsque le score ou le nombre de vies change (par des signaux).
     */
    void updateLabelDisplay();
};

#endif // GAMEWINDOW_H
