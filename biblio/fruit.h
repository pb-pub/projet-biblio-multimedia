/**
 * @file fruit.h
 * @brief Déclaration de la classe Fruit.
 * @author Boutet Paul, El Gote Ismaïl
 */

#ifndef FRUIT_H
#define FRUIT_H

#include <qopengl.h>
#include <QColor>
#include <QVector3D>
#include <QTime>
#include <QVector4D> // Added for QVector4D
#ifdef __APPLE__
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

/**
 * @class Fruit
 * @brief Représente un fruit ou une bombe dans le jeu.
 *
 * Gère la physique (position, mouvement), l'apparence (type, texture),
 * l'état (coupé ou non) et le dessin de l'objet fruit/bombe.
 */
class Fruit {
public:
    /**
     * @enum FruitType
     * @brief Énumère les différents types de fruits et la bombe.
     */
    enum FruitType {
        APPLE,     
        STRAWBERRY, 
        BANANA,     
        PEAR,       
        BOMB        
    };

    /**
     * @brief Constructeur principal de la classe Fruit.
     * @param type Type de fruit (FruitType).
     * @param textures Pointeur vers un tableau d'identifiants de textures OpenGL.
     * @param currentTime Temps actuel, utilisé pour initialiser le temps de départ du fruit.
     * @param initSpeed Vitesse initiale du fruit.
     * @param initPosition Position initiale du fruit.
     */
    Fruit(FruitType type, GLuint* textures, QTime currentTime, QVector3D initSpeed, QVector3D initPosition);

    /**
     * @brief Constructeur de la classe Fruit avec position et vitesse aléatoires.
     * @param type Type de fruit (FruitType).
     * @param textures Pointeur vers un tableau d'identifiants de textures OpenGL.
     * @param currentTime Temps actuel, utilisé pour initialiser le temps de départ du fruit.
     */
    Fruit(FruitType type, GLuint* textures, QTime currentTime);

    /**
     * @brief Constructeur de la classe Fruit avec type, position et vitesse aléatoires.
     * @param textures Pointeur vers un tableau d'identifiants de textures OpenGL.
     * @param currentTime Temps actuel, utilisé pour initialiser le temps de départ du fruit.
     */
    Fruit(GLuint* textures, QTime currentTime);

    /**
     * @brief Destructeur de la classe Fruit.
     * Libère les ressources allouées (par exemple, l'objet quadrique GLU).
     */
    ~Fruit();

    /**
     * @brief Vérifie si le fruit est une bombe.
     * @return true si le type actuel est BOMB, false sinon.
     */
    bool isBomb();

    /**
     * @brief Définit le type du fruit.
     * @param type Nouveau type de fruit (FruitType).
     */
    void setType(FruitType type);

    /**
     * @brief Dessine le fruit à sa position actuelle.
     * @param currentTime Temps actuel, utilisé pour calculer la position et gérer les animations.
     * @note Appelle la méthode de dessin spécifique au type de fruit.
     */
    void draw(QTime currentTime);

    /**
     * @brief Calcule et retourne la position du fruit à un temps donné.
     * @param currentTime Temps actuel pour lequel calculer la position.
     * @param firstPart Facteur optionnel pour le calcul de la position (utilisé pour les fruits coupés).
     * @return QVector3D représentant la position du fruit.
     * @note La position est calculée en utilisant une trajectoire parabolique basée sur la vitesse initiale et la gravité.
     */
    QVector3D getPosition(QTime currentTime, float firstPart = 1.f);

    /**
     * @brief Retourne la direction (vitesse) initiale du fruit.
     * @return QVector3D représentant la vitesse initiale.
     */
    QVector3D getInitialDirection() { return initalSpeed; }

    /**
     * @brief Marque le fruit comme coupé et définit le plan de coupe.
     * @param cutOriginPoint Point d'origine sur le plan de coupe.
     * @param cutNormalVector Vecteur normal au plan de coupe.
     * @param currentTime Temps actuel auquel le fruit est coupé.
     */
    void cut(const QVector3D& cutOriginPoint, const QVector3D& cutNormalVector, QTime currentTime);

    /**
     * @brief Vérifie si le fruit a été coupé.
     * @return true si le fruit est coupé, false sinon.
     */
    bool isCut() const;
    
private :
    FruitType currentFruit; ///< Type actuel du fruit (pomme, bombe, etc.).

    /**
     * @brief Dessine une pomme.
     * @param currentTime Temps actuel pour le calcul de la position.
     * @param firstPart Facteur pour dessiner une partie du fruit (utilisé si coupé).
     */
    void drawApple(QTime currentTime, float firstPart = 1.f);

    /**
     * @brief Dessine une fraise.
     * @param currentTime Temps actuel pour le calcul de la position.
     * @param firstPart Facteur pour dessiner une partie du fruit (utilisé si coupé).
     */
    void drawStrawberry(QTime currentTime, float firstPart = 1.f);

    /**
     * @brief Dessine une banane.
     * @param currentTime Temps actuel pour le calcul de la position.
     * @param firstPart Facteur pour dessiner une partie du fruit (utilisé si coupé).
     */
    void drawBanana(QTime currentTime, float firstPart = 1.f);

    /**
     * @brief Dessine une poire.
     * @param currentTime Temps actuel pour le calcul de la position.
     * @param firstPart Facteur pour dessiner une partie du fruit (utilisé si coupé).
     */
    void drawPear(QTime currentTime, float firstPart = 1.f);

    /**
     * @brief Dessine une bombe.
     * @param currentTime Temps actuel pour le calcul de la position.
     * @param firstPart Facteur pour dessiner une partie de la bombe (utilisé si coupée).
     */
    void drawBomb(QTime currentTime, float firstPart = 1.f);

    /**
     * @brief Applique une texture à l'objet.
     * @param textureID Identifiant de la texture OpenGL à appliquer.
     */
    void setTexture(GLuint textureID);

    /**
     * @brief Définit les propriétés matérielles pour l'éclairage OpenGL.
     * @param ambient Composante ambiante du matériau.
     * @param diffuse Composante diffuse du matériau.
     * @param specular Composante spéculaire du matériau.
     * @param shininess Exposant de brillance spéculaire.
     */
    void setMaterial(const GLfloat* ambient, const GLfloat* diffuse, const GLfloat* specular, const  GLfloat* shininess);
    
    /**
     * @brief Sélectionne un type de fruit aléatoire (excluant la bombe).
     * @return FruitType aléatoire.
     */
    FruitType getRandomFruitType();

    /**
     * @brief Génère une vitesse initiale aléatoire pour le fruit.
     * @return QVector3D représentant la vitesse initiale aléatoire.
     */
    QVector3D getRandomInitSpeed();
    
    QVector3D initalSpeed;      ///< Vitesse initiale du fruit lors de son lancement.
    QVector3D initialPosition;  ///< Position initiale du fruit lors de son lancement.
    GLUquadric* quadric;        ///< Objet quadrique GLU utilisé pour dessiner des formes (sphères, cylindres).
    GLuint* textures;           ///< Pointeur vers le tableau global de textures OpenGL.
    QTime startTime;            ///< Temps auquel le fruit a été créé ou lancé.

    bool m_isCut;               ///< Indicateur booléen : true si le fruit a été coupé, false sinon.
    QVector4D m_clipPlaneEquation; ///< Équation du plan de coupe (Ax + By + Cz + D = 0) sous forme de QVector4D (A, B, C, D).
    QVector3D normal;           ///< Vecteur normal au plan de coupe (redondant avec m_clipPlaneEquation.toVector3D() ?).
    QTime cutTime;              ///< Temps auquel le fruit a été coupé.

};



#endif // FRUIT_H
