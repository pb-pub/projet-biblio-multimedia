#include "katana.h"
#include <QOpenGLContext>

Katana::Katana() {
    quadric = gluNewQuadric();
    textures.resize(3); // Initialise le vecteur pour contenir 3 éléments
}

Katana::~Katana() {
    if (quadric) {
        gluDeleteQuadric(quadric);
    }
}

void Katana::draw(const QVector3D& position) {
    glPushMatrix();
    
    // Positionner le katana
    glTranslatef(position.x(), position.y(), position.z());
    
    // Rotation pour une meilleure orientation
    glRotatef(-45.0f, 0.0f, 1.0f, 0.0f);
    glRotatef(30.0f, 1.0f, 0.0f, 0.0f);
    
    // Échelle globale réduite (changé de 0.8f à 0.6f)
    glScalef(0.6f, 0.6f, 0.6f);
    
    // Dessiner la lame
    drawBlade();

    // Dessiner le tsuba (garde-bout)
    drawHandle();

    // Dessiner la chaîne
    drawChain();

    glPopMatrix();
}


void Katana::drawChain(void) {
    GLfloat chain_ambient[] = {0.3f, 0.3f, 0.3f, 1.0f};  // Couleur gris métallique
    GLfloat chain_specular[] = {0.8f, 0.8f, 0.8f, 1.0f}; // Reflets métalliques
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, chain_ambient);
    glMaterialfv(GL_FRONT, GL_SPECULAR, chain_specular);
    glMaterialf(GL_FRONT, GL_SHININESS, 50.0f);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textures[2]); 

    // Position de départ de la chaîne (au bout du manche)
    float startX = 0.0f;
    float startY = -0.6f;
    float startZ = 0.0f;

    // Paramètres de la chaîne
    const int chainLinks = 15;        // Nombre de maillons
    const float linkLength = 0.08f;   // Longueur de chaque maillon
    const float chainAmplitude = 0.1f; // Amplitude de l'oscillation

    glPushMatrix();
    for(int i = 0; i < chainLinks; i++) {
        float t = (float)i / chainLinks;
        // Création d'un mouvement ondulant
        float offsetX = chainAmplitude * sin(t * M_PI * 2.0f);
        float offsetY = -linkLength * i;

        // Dessiner le maillon
        glPushMatrix();
        glTranslatef(startX + offsetX, startY + offsetY, startZ);
        gluSphere(quadric, 0.02f, 8, 8);  // Petit maillon sphérique

        // Connecter les maillons
        if(i < chainLinks - 1) {
            glBegin(GL_LINES);
            glVertex3f(0.0f, 0.0f, 0.0f);
            glVertex3f(chainAmplitude * sin((t + 1.0f/chainLinks) * M_PI * 2.0f), 
                      -linkLength, 0.0f);
            glEnd();
        }
        glPopMatrix();
    }
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}



void Katana::drawBlade() {
    
    // 1. Dessiner la lame (droite et argentée)
    GLfloat blade_ambient[] = {0.8f, 0.8f, 0.8f, 1.0f};
    GLfloat blade_specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, blade_ambient);
    glMaterialfv(GL_FRONT, GL_SPECULAR, blade_specular);
    glMaterialf(GL_FRONT, GL_SHININESS, 100.0f);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textures[0]); // Texture pour la lame
    
    // Lame droite
    glBegin(GL_QUADS);
    // Face avant
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-0.05f, 0.0f, 0.01f);     // Base gauche
    glVertex3f(0.05f, 0.0f, 0.01f);      // Base droite
    glVertex3f(0.05f, 2.0f, 0.01f);      // Haut droit
    glVertex3f(-0.05f, 2.0f, 0.01f);     // Haut gauche
    
    // Face arrière
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(-0.05f, 0.0f, -0.01f);
    glVertex3f(0.05f, 0.0f, -0.01f);
    glVertex3f(0.05f, 2.0f, -0.01f);
    glVertex3f(-0.05f, 2.0f, -0.01f);
    glEnd();

    // Ajouter la pointe triangulaire rectangle
    glBegin(GL_TRIANGLES);
    // Face avant de la pointe
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-0.05f, 2.0f, 0.01f);    // Base gauche
    glVertex3f(0.05f, 2.0f, 0.01f);     // Base droite
    glVertex3f(-0.05f, 2.2f, 0.0f);     // Pointe (alignée avec le côté gauche)

    // Face arrière de la pointe
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(-0.05f, 2.0f, -0.01f);   // Base gauche
    glVertex3f(0.05f, 2.0f, -0.01f);    // Base droite
    glVertex3f(-0.05f, 2.2f, 0.0f);     // Pointe (alignée avec le côté gauche)
    glEnd();

    // Côtés de la pointe
    glBegin(GL_TRIANGLES);
    // Côté gauche (maintenant vertical)
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(-0.05f, 2.0f, -0.01f);
    glVertex3f(-0.05f, 2.0f, 0.01f);
    glVertex3f(-0.05f, 2.2f, 0.0f);     // Pointe (alignée avec le côté gauche)

    // Côté droit
    glNormal3f(1.0f, 0.5f, 0.0f);
    glVertex3f(0.05f, 2.0f, -0.01f);
    glVertex3f(0.05f, 2.0f, 0.01f);
    glVertex3f(-0.05f, 2.2f, 0.0f);     // Pointe (alignée avec le côté gauche)
    glEnd();

    glDisable(GL_TEXTURE_2D);

}

void Katana::drawHandle() {
    // 2. Dessiner le tsuba (garde-bout)
    GLfloat tsuba_ambient[] = {0.4f, 0.4f, 0.4f, 1.0f}; // Couleur métallique foncée
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, tsuba_ambient);
        
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textures[1]); // Texture pour le tsuba

    glTranslatef(0.0f, 0.0f, 0.0f);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    gluDisk(quadric, 0.0f, 0.2f, 32, 1);    // Face avant
    glTranslatef(0.0f, 0.0f, 0.02f);
    gluDisk(quadric, 0.0f, 0.2f, 32, 1);    // Face arrière
    gluCylinder(quadric, 0.2f, 0.2f, 0.02f, 32, 1); // Bord

    glDisable(GL_TEXTURE_2D);
    glPopMatrix();


    // 3. Dessiner le manche (cylindrique)
    GLfloat handle_ambient[] = {0.5f, 0.3f, 0.1f, 1.0f}; // Couleur bois
    GLfloat handle_specular[] = {0.8f, 0.5f, 0.2f, 1.0f}; // Reflets bois
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, handle_ambient);
    glMaterialfv(GL_FRONT, GL_SPECULAR, handle_specular);
    glMaterialf(GL_FRONT, GL_SHININESS, 50.0f);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textures[1]); // Texture pour le manche
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.0f); // Positionner le manche
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f); // Rotation pour aligner le cylindre
    gluCylinder(quadric, 0.05f, 0.05f, 0.6f, 32, 1); // Dessiner le manche
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
    
}
