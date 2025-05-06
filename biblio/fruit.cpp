#include "fruit.h"
#include <iostream>
#include <QImage>
#include <QDir>

Fruit::Fruit(FruitType type, GLuint *textureids, QTime currentTime, QVector3D initSpeed, QVector3D initPosition) : currentFruit(type), textures(textureids), startTime(currentTime), initalSpeed(initSpeed), initialPosition(initPosition)
{

    quadric = gluNewQuadric();
    gluQuadricDrawStyle(quadric, GLU_FILL);
    gluQuadricNormals(quadric, GLU_SMOOTH);
    gluQuadricTexture(quadric, GL_TRUE);

}


Fruit::Fruit(FruitType type, GLuint *textureids, QTime currentTime) : currentFruit(type), textures(textureids), startTime(currentTime), initalSpeed(QVector3D(1, 7, -20)), initialPosition(QVector3D(0, 1, 30))
{

    quadric = gluNewQuadric();
    gluQuadricDrawStyle(quadric, GLU_FILL);
    gluQuadricNormals(quadric, GLU_SMOOTH);
    gluQuadricTexture(quadric, GL_TRUE);

}

Fruit::Fruit(GLuint* textureids, QTime currentTime) : textures(textureids), startTime(currentTime), initialPosition(QVector3D(0, 1, 30))
{
    std::cout<<"Creating fruit" << std::endl;
    initalSpeed = getRandomInitSpeed();
    currentFruit = getRandomFruitType();

    quadric = gluNewQuadric();
    gluQuadricDrawStyle(quadric, GLU_FILL);
    gluQuadricNormals(quadric, GLU_SMOOTH);
    gluQuadricTexture(quadric, GL_TRUE);

}

Fruit::~Fruit()
{
    gluDeleteQuadric(quadric);
}

Fruit::FruitType Fruit::getRandomFruitType()
{
    int randomValue = rand() % 5; // Random number between 0 and 4
    switch (randomValue)
    {
    case 0:
        return APPLE;
    case 1:
        return ORANGE;
    case 2:
        return BANANA;
    case 3:
        return PEAR;
    case 4:
        return BOMB;
    default:
        return APPLE; // Default case
    }
}

QVector3D Fruit::getRandomInitSpeed()
{
    // Generate a random speed vector
    float x = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 3.0f - 1.5f; // Random value between -2 and 2
    float y = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 1.8f + 4.8f; // Random value between 3 and 7
    float z = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 8.0f - 32.0f; // Random value between -35 and -25
    return QVector3D(x, y, z);
}



void Fruit::setType(FruitType type)
{
    currentFruit = type;
}


void Fruit::draw(QTime currentTime)
{

    switch (currentFruit)
    {
    case APPLE:
        drawApple(currentTime);
        break;
    case ORANGE:
        drawOrange(currentTime);
        break;
    case BANANA:
        drawBanana(currentTime);
        break;
    case PEAR:
        drawPear(currentTime);
        break;
    case BOMB:
        drawBomb(currentTime);
        break;
    }
}

void Fruit::setTexture(GLuint textureID)
{
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glEnable(GL_TEXTURE_2D);
}

void Fruit::setMaterial(const GLfloat *ambient, const GLfloat *diffuse, const GLfloat *specular, const GLfloat *shininess)
{
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
}

void Fruit::drawApple(QTime currentTime)
{
    // Positionnement de la pomme
    glPushMatrix();
    QVector3D position = getPosition(currentTime);
    glTranslatef(position.x(), position.y(), position.z());
    
    // Add rotation based on time
    float rotationAngle = startTime.msecsTo(currentTime) / 20.0f; // Adjust divisor for rotation speed
    glRotatef(rotationAngle, 0.0f, 1.0f, 0.3f); // Rotate around a slightly tilted axis
    
    // Set color to white before texturing to avoid tinting the texture
    glColor3f(1.0f, 1.0f, 1.0f); 
    // Desin du corps de la pomme (avec la texture)
    setTexture(textures[0]);
    glPushMatrix(); // Apple body rotation
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f); // Rotation for texture alignment
    gluSphere(quadric, 0.3, 32, 32);
    glPopMatrix(); // End apple body rotation
    glDisable(GL_TEXTURE_2D);

    // Dessin de la tige (stem)
    // Set material for the stem (e.g., brown)
    GLfloat stemAmbient[] = {0.4f, 0.2f, 0.0f, 1.0f};
    GLfloat stemDiffuse[] = {0.5f, 0.25f, 0.0f, 1.0f};
    GLfloat stemSpecular[] = {0.1f, 0.05f, 0.0f, 1.0f};
    GLfloat stemShininess[] = {10.0f};
    setMaterial(stemAmbient, stemDiffuse, stemSpecular, stemShininess);
    glColor3fv(stemDiffuse); // Set color for stem, works with GL_COLOR_MATERIAL

    glPushMatrix();
    glTranslatef(0.0f, 0.3f, 0.0f); // Position the stem on top of the apple
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f); // Orient the stem upwards
    gluCylinder(quadric, 0.02, 0.015, 0.15, 16, 16); // Stem
    glPopMatrix();

    // Dessin du calice (bottom part)
    // Set material for the calyx (e.g., dark green/brown)
    GLfloat calyxAmbient[] = {0.1f, 0.1f, 0.0f, 1.0f};
    GLfloat calyxDiffuse[] = {0.2f, 0.2f, 0.0f, 1.0f};
    GLfloat calyxSpecular[] = {0.05f, 0.05f, 0.0f, 1.0f};
    GLfloat calyxShininess[] = {5.0f};
    setMaterial(calyxAmbient, calyxDiffuse, calyxSpecular, calyxShininess);
    glColor3fv(calyxDiffuse); // Set color for calyx

    glPushMatrix();
    glTranslatef(0.0f, -0.28f, 0.0f); // Position the calyx at the bottom
    gluSphere(quadric, 0.05, 16, 16); // Calyx as a small sphere
    glPopMatrix();

    // Reset color to white so it doesn't affect other objects
    glColor3f(1.0f, 1.0f, 1.0f);

    glPopMatrix(); // Main apple matrix
}

void Fruit::drawOrange(QTime currentTime)
{
    // Positionnement de l'orange
    glPushMatrix(); // Main orange transform
    QVector3D position = getPosition(currentTime);
    glTranslatef(position.x(), position.y(), position.z());
    
    // Add rotation based on time
    float rotationAngle = startTime.msecsTo(currentTime) / 18.0f;
    glRotatef(rotationAngle, 0.2f, 1.0f, 0.0f);
    
    // Set color to white before texturing the main body
    glColor3f(1.0f, 1.0f, 1.0f);
    // Dessin de l'orange (avec la texture)
    setTexture(textures[1]);
    glPushMatrix(); // Orange body rotation
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f); // Rotation pour aligner la texture sur les pôles
    gluSphere(quadric, 0.3, 32, 32); // Main orange body
    glPopMatrix(); // End orange body rotation
    glDisable(GL_TEXTURE_2D);

    // Dessin du "nombril" (calice/navel) de l'orange
    GLfloat navelAmbient[] = {0.3f, 0.15f, 0.0f, 1.0f}; // Darker orange/brown
    GLfloat navelDiffuse[] = {0.4f, 0.2f, 0.05f, 1.0f};
    GLfloat navelSpecular[] = {0.1f, 0.05f, 0.0f, 1.0f};
    GLfloat navelShininess[] = {5.0f};
    setMaterial(navelAmbient, navelDiffuse, navelSpecular, navelShininess);
    glColor3fv(navelDiffuse);

    glPushMatrix();
    glTranslatef(0.0f, -0.29f, 0.0f); // Position at the bottom of the orange
    gluSphere(quadric, 0.04, 16, 16); // Small sphere for the navel
    glPopMatrix();

    // Dessin d'une petite indication de tige/feuille verte
    GLfloat stemLeafAmbient[] = {0.0f, 0.2f, 0.0f, 1.0f}; // Greenish
    GLfloat stemLeafDiffuse[] = {0.0f, 0.3f, 0.0f, 1.0f};
    GLfloat stemLeafSpecular[] = {0.0f, 0.1f, 0.0f, 1.0f};
    GLfloat stemLeafShininess[] = {10.0f};
    setMaterial(stemLeafAmbient, stemLeafDiffuse, stemLeafSpecular, stemLeafShininess);
    glColor3fv(stemLeafDiffuse);

    glPushMatrix();
    glTranslatef(0.0f, 0.3f, 0.0f); // Position at the top
    // Optional: make it a very short, wide cylinder or a tiny sphere
    // gluCylinder(quadric, 0.03, 0.02, 0.05, 12, 1);
    gluSphere(quadric, 0.03, 12, 12); // Tiny sphere for stem/leaf spot
    glPopMatrix();

    // Reset color to white so it doesn't affect other objects
    glColor3f(1.0f, 1.0f, 1.0f);

    glPopMatrix(); // End main orange transform
}

void Fruit::drawBanana(QTime currentTime)
{
    // Positionnement de la banane
    glPushMatrix();
    QVector3D position = getPosition(currentTime);
    glTranslatef(position.x(), position.y(), position.z());

    // Add rotation based on time
    float rotationAngle = startTime.msecsTo(currentTime) / 15.0f;
    glRotatef(rotationAngle, 0.5f, 1.0f, 0.5f);
    
    // Position the banana properly
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);

    // Set color to white before texturing
    glColor3f(1.0f, 1.0f, 1.0f);
    // Dessin de la banane (forme courbée)
    setTexture(textures[2]);
    
    // Parameters for the banana shape
    const int numSegments = 8;
    const float bananaLength = 0.8f;
    const float maxRadius = 0.07f;
    const float curvature = 0.2f;
    
    // Draw the banana as connected curved segments using quadrics
    for (int i = 0; i < numSegments - 1; i++) {
        float t1 = (float)i / (numSegments - 1);
        float t2 = (float)(i + 1) / (numSegments - 1);
        
        // Calculate positions along a curved path
        float x1 = curvature * sin(t1 * M_PI);
        float z1 = t1 * bananaLength;
        float x2 = curvature * sin(t2 * M_PI);
        float z2 = t2 * bananaLength;
        
        // Calculate radii (thicker in middle, thinner at ends)
        float r1 = maxRadius * sin(t1 * M_PI * 0.8f + 0.1f * M_PI);
        float r2 = maxRadius * sin(t2 * M_PI * 0.8f + 0.1f * M_PI);
        
        // Position and orient the cylinder segment
        glPushMatrix();
        glTranslatef(x1, 0.0f, z1);
        
        // Calculate direction and length
        float dx = x2 - x1;
        float dz = z2 - z1;
        float length = sqrt(dx*dx + dz*dz);
        
        // Rotate to align with the curve
        float angle = atan2(dx, dz) * 180.0f / M_PI;
        glRotatef(angle, 0.0f, 1.0f, 0.0f);
        
        // Draw the segment
        gluCylinder(quadric, r1, r2, length, 16, 1);
        
        // Draw end caps for smoother appearance
        if (i == 0) {
            gluDisk(quadric, 0.0, r1, 16, 1);
        }
        if (i == numSegments - 2) {
            glTranslatef(0.0f, 0.0f, length);
            gluDisk(quadric, 0.0, r2, 16, 1);
        }
        
        glPopMatrix();
    }

    glDisable(GL_TEXTURE_2D);
    // Reset color to white after drawing banana parts if any non-textured parts were colored
    glColor3f(1.0f, 1.0f, 1.0f);
    glPopMatrix();
}

void Fruit::drawPear(QTime currentTime)
{
    // Positionnement
    glPushMatrix(); // Main pear transform
    QVector3D position = getPosition(currentTime);
    glTranslatef(position.x(), position.y(), position.z());
    
    // Add rotation based on time for the whole pear
    float rotationAngle = startTime.msecsTo(currentTime) / 22.0f;
    glRotatef(rotationAngle, 0.3f, 1.0f, 0.2f); // Rotate the whole pear

    // Set color to white for textured parts to avoid tinting
    glColor3f(1.0f, 1.0f, 1.0f);
    
    // Enable and set texture for pear body parts
    setTexture(textures[3]);

    // Bottom Part (Main Body of the Pear)
    glPushMatrix();
    glTranslatef(0.0f, -0.05f, 0.0f);   // Position the lower, wider part slightly down
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f); // Align texture poles correctly
    gluSphere(quadric, 0.22f, 32, 32);  // Main bottom part, radius 0.22
    glPopMatrix();

    // Top Part (Neck of the Pear)
    glPushMatrix();
    glTranslatef(0.0f, 0.18f, 0.0f);   // Position the upper, narrower part
    glScalef(0.75f, 1.3f, 0.75f);      // Scale to make it narrower and taller, creating the pear neck shape
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f); // Align texture poles correctly
    gluSphere(quadric, 0.15f, 32, 32);  // Neck part, radius 0.15 before scaling
    glPopMatrix();
    
    glDisable(GL_TEXTURE_2D); // Disable texturing after drawing textured parts

    // Dessin de la tige (Stem)
    GLfloat stemAmbient[] = {0.4f, 0.2f, 0.0f, 1.0f}; // Brownish color for the stem
    GLfloat stemDiffuse[] = {0.5f, 0.25f, 0.0f, 1.0f};
    GLfloat stemSpecular[] = {0.1f, 0.05f, 0.0f, 1.0f};
    GLfloat stemShininess[] = {10.0f};
    setMaterial(stemAmbient, stemDiffuse, stemSpecular, stemShininess);
    glColor3fv(stemDiffuse); // Apply diffuse color for the stem

    glPushMatrix();
    // Position stem at the top of the neck. Neck top Y is approx: 0.18 (translate) + 0.15*1.3 (scaled radius) = 0.375
    glTranslatef(0.0f, 0.37f, 0.0f); 
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f); // Orient stem upwards
    gluCylinder(quadric, 0.02f, 0.015f, 0.12f, 16, 16); // Stem dimensions
    glPopMatrix();

    // Dessin du calice (Bottom part of the pear)
    GLfloat calyxAmbient[] = {0.1f, 0.1f, 0.0f, 1.0f}; // Dark color for the calyx
    GLfloat calyxDiffuse[] = {0.2f, 0.2f, 0.0f, 1.0f};
    GLfloat calyxSpecular[] = {0.05f, 0.05f, 0.0f, 1.0f};
    GLfloat calyxShininess[] = {5.0f};
    setMaterial(calyxAmbient, calyxDiffuse, calyxSpecular, calyxShininess);
    glColor3fv(calyxDiffuse); // Apply diffuse color for the calyx

    glPushMatrix();
    // Position calyx at the bottom. Base bottom Y is approx: -0.05 (translate) - 0.22 (radius) = -0.27
    glTranslatef(0.0f, -0.27f, 0.0f); 
    gluSphere(quadric, 0.05f, 16, 16); // Calyx as a small sphere
    glPopMatrix();

    // Reset color to white to avoid affecting subsequent drawings
    glColor3f(1.0f, 1.0f, 1.0f);

    glPopMatrix(); // End main pear transform
}

void Fruit::drawBomb(QTime currentTime)
{
    // Positionnement de la bombe
    glPushMatrix();
    QVector3D position = getPosition(currentTime);
    glTranslatef(position.x(), position.y(), position.z());
    
    // Add rotation based on time
    float rotationAngle = startTime.msecsTo(currentTime) / 10.0f; 
    glRotatef(rotationAngle, 0.0f, -1.0f, 0.0f);
    
    // Set color to white before texturing the bomb body
    glColor3f(1.0f, 1.0f, 1.0f);
    // Dessin du corps principal de la bombe (sphère)
    setTexture(textures[4]);
    glPushMatrix(); // Bomb body rotation
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f); // Rotation pour aligner la texture sur les pôles
    gluSphere(quadric, 0.3, 32, 32);
    glPopMatrix(); // End bomb body rotation
    glDisable(GL_TEXTURE_2D);


    // Dessin de la mèche (cylindre)
    // Set material for the fuse (e.g., grey/dark yellow)
    GLfloat fuseAmbient[] = {0.2f, 0.2f, 0.1f, 1.0f};
    GLfloat fuseDiffuse[] = {0.4f, 0.4f, 0.2f, 1.0f};
    GLfloat fuseSpecular[] = {0.1f, 0.1f, 0.05f, 1.0f};
    GLfloat fuseShininess[] = {5.0f};
    setMaterial(fuseAmbient, fuseDiffuse, fuseSpecular, fuseShininess);
    glColor3fv(fuseDiffuse); // Set color for fuse

    glPushMatrix();
    glTranslatef(0.0f, 0.3f, 0.0f); // Positionner la mèche au-dessus de la sphère
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f); // Aligner le cylindre verticalement
    gluCylinder(quadric, 0.02, 0.02, 0.2, 16, 16); // Cylindre pour la mèche
    glPopMatrix();

    // Reset color to white so it doesn't affect other objects
    glColor3f(1.0f, 1.0f, 1.0f);
    // Désactiver la texture (already disabled after bomb body)
    // glDisable(GL_TEXTURE_2D); // This was here, but texture is already disabled.
    glPopMatrix();
}

QVector3D Fruit::getPosition(QTime currentTime)
{
    // Calculate the position of the fruit based on its trajectory
    
    float deltaT = startTime.msecsTo(currentTime) / 1000.0f;
    float x = initalSpeed.x() * deltaT + initialPosition.x();
    float y = initalSpeed.y() * deltaT + initialPosition.y() - (0.5f * 9.81f * deltaT * deltaT);
    float z = initalSpeed.z() * deltaT + initialPosition.z();

    //std::cout << "Position: " << x << ", " << y << ", " << z << std::endl;

    return QVector3D(x, y, z);
}

bool Fruit::isBomb()
{
    return currentFruit == BOMB;
}
