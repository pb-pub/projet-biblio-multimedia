/**
 * @file katana.h
 * @brief Déclaration de la classe Katana.
 * @author Boutet Paul, El Gote Ismaïl // Assuming same authors, adjust if not
 */

#ifndef KATANA_H
#define KATANA_H

#ifdef __APPLE__
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

#include <QVector3D>
#include <vector>

/**
 * @class Katana
 * @brief Représente un katana dans le jeu, utilisé pour interagir avec les objets.
 *
 * Cette classe gère le dessin du katana en 3D.
 */
class Katana {
public:

    
    std::vector<GLuint> textures; ///< Tableau de textures pour le katana.

    /**
     * @brief Constructeur de Katana.
     * Initialise les ressources nécessaires pour le katana, comme l'objet quadrique.
     */
    Katana();

    /**
     * @brief Destructeur de Katana.
     * Libère les ressources allouées, notamment l'objet quadrique.
     */
    ~Katana();

    /**
     * @brief Dessine le katana à la position spécifiée.
     * @param position Position 3D où le katana doit être dessiné.
     */
    void draw(const QVector3D& position);

    /**
     * @brief Définit les textures du katana.
     * @param textures Tableau de textures à appliquer au katana.
     */
    void setTextures(GLuint blade, GLuint handle, GLuint chain) { textures[0] = blade; textures[1] = handle; textures[2] = chain; } 

private:
    /**
     * @brief Dessine la lame du katana.
     */
    void drawBlade();

    /**
     * @brief Dessine le manche du katana.
     */
    void drawHandle();

    /**
     * @brief Dessine la chaîne ou un élément décoratif du katana.
     */
    void drawChain();
    GLUquadric* quadric; ///< Objet quadrique GLU utilisé pour dessiner les parties cylindriques du katana.
};

#endif // KATANA_H
