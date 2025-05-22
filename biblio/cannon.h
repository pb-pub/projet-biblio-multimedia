/**
 * @file cannon.h
 * @brief Déclaration de la classe Cannon.
 * @author Boutet Paul, El Gote Ismaïl 
 */

#include <qvectornd.h> 
#include <QVector3D>  
#include <qopengl.h>  

#ifdef __APPLE__
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

/**
 * @class Cannon
 * @brief Représente le canon utilisé par le joueur pour lancer des fruits.
 *
 * Cette classe gère la position, l'orientation et le dessin du canon.
 * Elle peut également réagir à la création de fruits pour s'orienter.
 */
class Cannon {
private :
    QVector3D position;     ///< Position 3D du canon dans la scène.
    float angleX;           ///< Angle de rotation du canon autour de l'axe X.
    float angleY;           ///< Angle de rotation du canon autour de l'axe Y.
    float angleZ;           ///< Angle de rotation du canon autour de l'axe Z.
    GLUquadric* quadric;    ///< Objet quadrique GLU utilisé pour dessiner les parties du canon.
    GLuint cannonTexture;   ///< Identifiant de la texture OpenGL pour le canon.
    bool hasTexture;        ///< Indicateur booléen : true si une texture est assignée au canon, false sinon.


public:
    /**
     * @brief Constructeur de la classe Cannon.
     * Initialise les angles, la position par défaut et l'objet quadrique.
     */
    Cannon();

    /**
     * @brief Destructeur de la classe Cannon.
     * Libère les ressources allouées (par exemple, l'objet quadrique GLU).
     */
    ~Cannon();

    /**
     * @brief Définit la position du canon.
     * @param position Nouvelle position 3D du canon.
     */
    void setPosition(QVector3D position) { this->position = position; }

    /**
     * @brief Oriente le canon pour qu'il pointe vers une direction donnée.
     * @param direction Vecteur 3D indiquant la direction cible.
     * @note Calcule les angles de rotation (angleX, angleY) nécessaires.
     */
    void setDirection(QVector3D direction);

    /**
     * @brief Assigne une texture au canon.
     * @param textureId Identifiant de la texture OpenGL à utiliser.
     */
    void setTexture(GLuint textureId) { cannonTexture = textureId; hasTexture = true; }

    /**
     * @brief Dessine le canon dans la scène OpenGL.
     * Utilise la position et les angles actuels pour transformer et dessiner le modèle du canon.
     */
    void drawCannon();

    /**
     * @brief Méthode appelée lors de la création d'un fruit.
     * Permet au canon de s'orienter vers la direction de lancement du fruit.
     * @param direction Direction initiale du fruit lancé.
     */
    void onFruitCreated(QVector3D direction);

};
