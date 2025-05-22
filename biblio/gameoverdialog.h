/**
 * @file gameoverdialog.h
 * @brief Déclaration de la classe GameOverDialog.
 * @author Boutet Paul, El Gote Ismaïl 
 */

#ifndef GAMEOVERDIALOG_H
#define GAMEOVERDIALOG_H

#include <QDialog>

class QLabel;
class QPushButton;

/**
 * @class GameOverDialog
 * @brief Boîte de dialogue affichée lorsque la partie est terminée.
 *
 * Cette classe affiche le score final du joueur et propose des options pour
 * commencer une nouvelle partie ou quitter l'application.
 */
class GameOverDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief Constructeur de GameOverDialog.
     * @param score Score final du joueur à afficher.
     * @param parent Widget parent, nullptr par défaut.
     * Initialise l'interface utilisateur de la boîte de dialogue avec le score.
     */
    explicit GameOverDialog(int score, QWidget *parent = nullptr);

    /**
     * @brief Destructeur de GameOverDialog.
     * Libère les ressources allouées.
     */
    ~GameOverDialog();

private slots:
    /**
     * @brief Slot appelé lorsque le bouton "Nouvelle Partie" est cliqué.
     * Gère la logique pour redémarrer une partie en relançant une fenêtre de jeu.
     */
    void onNewGameClicked();

    /**
     * @brief Slot appelé lorsque le bouton "Quitter" est cliqué.
     * Ferme l'application ou la boîte de dialogue.
     * Rejette la boîte de dialogue.
     */
    void onExitClicked();

private:
    QLabel *gameOverLabel;  ///< Étiquette affichant le message "Game Over".
    QLabel *scoreLabel;     ///< Étiquette affichant le score final du joueur.
    QPushButton *newGameButton; ///< Bouton pour démarrer une nouvelle partie.
    QPushButton *exitButton;    ///< Bouton pour quitter le jeu/l'application.
};

#endif // GAMEOVERDIALOG_H
