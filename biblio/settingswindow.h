/**
 * @file settingswindow.h
 * @brief Déclaration de la classe SettingsWindow.
 * @author Boutet Paul, El Gote Ismaïl
 */

#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QWidget>
#include "camerawidget.h"

namespace Ui {
class SettingsWindow;
}

/**
 * @class SettingsWindow
 * @brief Fenêtre de paramètres pour configurer les options de l'application.
 *
 * Cette classe hérite de QWidget et permet à l'utilisateur de configurer
 * certains paramètres, notamment la caméra. Elle peut également afficher
 * un aperçu du flux vidéo de la caméra.
 */
class SettingsWindow : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Constructeur de SettingsWindow.
     * @param parent Widget parent, nullptr par défaut.
     * Initialise l'interface utilisateur de la fenêtre des paramètres et potentiellement le cameraWidget.
     */
    explicit SettingsWindow(QWidget *parent = nullptr);

    /**
     * @brief Destructeur de SettingsWindow.
     * Libère les ressources allouées, notamment l'objet ui et cameraWidget.
     */
    ~SettingsWindow();

private slots:
    /**
     * @brief Slot appelé lors du clic sur le bouton poussoir (pushButton).
     * Permet d'activer / déactiver sur le seuillage.
     */
    void on_pushButton_clicked();

private:
    Ui::SettingsWindow *ui;     ///< Pointeur vers l'objet d'interface utilisateur généré par Qt Designer.
    CameraWidget *cameraWidget; ///< Pointeur vers un widget qui affiche le flux de la caméra ou permet de configurer ses paramètres.
};

#endif // SETTINGSWINDOW_H
