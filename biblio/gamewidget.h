/**
 * @file gamewidget.h
 * @brief Déclaration de la classe GameWidget.
 * @author Boutet Paul, El Gote Ismaïl
 */

#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include <QWidget>
#include <QOpenGLWidget>
#include "fruit.h"
#include <qlabel.h>
#include <vector>
#include <QColor>
#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include <QTimer>
#include <QSoundEffect>
#include "camerahandler.h"
#include "cannon.h"
#include <QKeyEvent>
#include <QTime>
#include <QVector3D>
#include <opencv2/opencv.hpp>
#include "katana.h"

typedef struct GLUquadric GLUquadric;

namespace Ui
{
class GameWidget;
}

/**
 * @class GameWidget
 * @brief Gère la logique principale du jeu, l'affichage 3D et l'interaction avec la caméra.
 *
 * Cette classe est responsable de l'initialisation de l'environnement OpenGL,
 * du rendu des objets du jeu (fruits, canon), de la gestion des entrées utilisateur
 * (via la caméra), et de la mise à jour de l'état du jeu.
 */
class GameWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Constructeur de GameWidget.
     * @param parent Widget parent, nullptr par défaut.
     */
    explicit GameWidget(QWidget *parent = nullptr);

    /**
     * @brief Destructeur de GameWidget.
     * Libère les ressources allouées.
     */
    ~GameWidget();

    /**
     * @brief Met à jour l'affichage des fruits.
     * Force l'actualisation de l'affichage des fruits à l'écran.
     */
    void updateFruitDisplay();

signals:
    /**
     * @brief Signal émis lorsqu'un fruit est touché.
     * Connecté pour augmenter le score du joueur.
     */
    void scoreIncreased();

    /**
     * @brief Signal émis lorsqu'une bombe est touchée.
     * Connecté pour diminuer la vie du joueur.
     */
    void lifeDecrease();

public slots:
    /**
     * @brief Initialise l'environnement OpenGL.
     * Appelé une fois avant le premier appel à paintGL() ou resizeGL().
     * Configure l'état initial d'OpenGL (couleur de fond, profondeur, etc.).
     */
    void initializeGL();

    /**
     * @brief Initialise les textures utilisées dans le jeu.
     * Charge les images pour les fruits, bombes, etc., et les prépare pour OpenGL.
     */
    void initializeTextures();

    /**
     * @brief Démarre un compte à rebours avant le début du jeu pour donner le
     * temps au joueur de se préparer et à OpenGL de s'initialiser.
     * @param seconds Durée du compte à rebours en secondes.
     */
    void startCountdown(int seconds);

    /**
     * @brief Gère le redimensionnement de la fenêtre OpenGL.
     * @param width Nouvelle largeur de la fenêtre.
     * @param height Nouvelle hauteur de la fenêtre.
     * Met à jour la projection et le viewport OpenGL.
     */
    void resizeGL(int width, int height);

    /**
     * @brief Gère le rendu de la scène OpenGL.
     * Appelé chaque fois que le widget a besoin d'être redessiné.
     * Dessine tous les éléments du jeu (fruits, canon, fond, etc.).
     */
    void paintGL();

    /**
     * @brief Met à jour la frame de la caméra et traite les interactions.
     * Ce slot est probablement connecté à un QTimer pour récupérer périodiquement
     * une nouvelle image de la caméra et la traiter.
     */
    void updateFrame();

private:
    Ui::GameWidget *ui;
    std::vector<Fruit *> m_fruit; ///< Conteneur pour tous les objets Fruit actifs dans le jeu.
    GLuint *textures; ///< Tableau d'identifiants de texture OpenGL.
    QFont m_font; ///< Police de caractères utilisée pour afficher du texte (ex: score, messages).
    QSoundEffect *m_sliceSound; ///< Effet sonore joué lorsqu'un fruit est coupé.
    QSoundEffect *m_shootSound; ///< Effet sonore joué lors d'un tir.
    QLabel *label; ///< QLabel utilisé pour afficher le score et les vies.
    GLUquadric *cylinder; ///< Objet quadrique GLU.
    CameraHandler *cameraHandler; ///< Gestionnaire pour l'interaction avec la webcam.
    QTimer *cameraTimer; ///< Timer pour déclencher la mise à jour périodique de la frame de la caméra.
    cv::Mat currentFrame; ///< Image actuelle capturée par la caméra (en couleur).
    cv::Mat grayFrame; ///< Image actuelle capturée par la caméra (convertie en niveaux de gris).
    bool cameraInitialized; ///< Indicateur de l'état d'initialisation de la caméra.
    QVector3D projectedPoint; ///< Coordonnées 3D d'un point projeté (potentiellement depuis l'espace caméra vers l'espace jeu).
    bool hasProjectedPoint; ///< Indicateur de la disponibilité d'un point projeté.
    Cannon cannon; ///< Objet représentant le canon du joueur.
    bool displayCamera; ///< Indicateur pour afficher ou non le flux de la caméra à l'écran.
    GLuint m_cameraTextureId; ///< Identifiant de texture OpenGL pour le flux vidéo de la caméra.
    Katana* m_katana = nullptr; ///< Objet représentant le katana du joueur.

    /**
     * @brief Gère les événements de pression de touche.
     * @param event Événement de clavier contenant les informations sur la touche pressée.
     * Utilisé pour les contrôles au clavier (débogage, actions alternatives).
     */
    void keyPressEvent(QKeyEvent *event) override;

    /**
     * @brief Crée une texture OpenGL à partir d'une couleur unie.
     * @param color Couleur de la texture à créer.
     * @return QImage représentant la texture colorée.
     * Utilisé comme fallback si le chargement d'une texture image échoue.
     */
    QImage createColorTexture(const QColor &color);

    /**
     * @brief Crée et initialise un nouvel objet Fruit.
     * @return Pointeur vers le Fruit nouvellement créé.
     * Configure la position initiale, la vitesse, le type (fruit/bombe) du fruit.
     */
    Fruit *createFruit();

    /**
     * @brief Initialise la caméra.
     * Configure la capture vidéo et prépare la caméra pour son utilisation.
     */
    void initializeCamera();

    /**
     * @brief Vérifie si un point donné touche un fruit.
     * @param point Coordonnées du point d'interaction.
     * @param fruit Pointeur vers l'objet Fruit à tester.
     * @param currentTime Temps actuel, utilisé pour gérer les timings de collision ou d'animation.
     * @return true si le fruit est touché, false sinon.
     * @note L'algorithme peut impliquer de vérifier si `point` est dans le rayon du `fruit`.
     */
    bool isFruitHit(const cv::Point &point, Fruit *fruit, QTime currentTime);

    /**
     * @brief Convertit un point de l'espace caméra 2D en coordonnées de l'espace de jeu 3D
     * en le mappant sur le cylindre entourant le joueur.
     * @param cameraPoint Coordonnées 2D du point dans l'image de la caméra.
     * @param gameX Référence pour stocker la coordonnée X résultante dans l'espace de jeu. (paramètre de sortie)
     * @param gameZ Référence pour stocker la coordonnée Z résultante dans l'espace de jeu. (paramètre de sortie)
     */
    void convertCameraPointToGameSpace(const cv::Point &cameraPoint, float &gameX, float &gameZ);
};

#endif // GAMEWIDGET_H
