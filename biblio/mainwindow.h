/**
 * @file mainwindow.h
 * @brief Déclaration de la classe MainWindow.
 * @author Boutet Paul, El Gote Ismaïl
 */
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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

private:
    Ui::MainWindow *ui; ///< Pointeur vers l'objet d'interface utilisateur généré par Qt Designer.
};
#endif // MAINWINDOW_H
