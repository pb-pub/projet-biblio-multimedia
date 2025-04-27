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
    float x = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 4.0f - 2.0f; // Random value between -2 and 2
    float y = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 4.0f + 3.0f; // Random value between 3 and 7
    float z = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 10.0f - 35.0f; // Random value between -35 and -25
    return QVector3D(x, y, z);
}



void Fruit::setType(FruitType type)
{
    currentFruit = type;
}

// void Fruit::draw(const float time) {
//     // Position the fruit

//     std::cout << "Drawing fruit type: " << currentFruit << std::endl;

//     // initializeTextures();
//     glPushMatrix();

//     GLfloat ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
//     GLfloat diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
//     GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
//     GLfloat shininess = 0.0f;
//     setMaterial(ambient, diffuse, specular, &shininess);

//     std::cout << "Texture id : " << textures[1] << std::endl;
//     setTexture(textures[1]);

//     // Correction de l'orientation de la texture
//     glRotatef(90.0f, 1.0f, 0.0f, 0.0f);  // Rotation pour aligner la texture sur les pôles
//     gluSphere(quadric, 1, 32, 32);

//     glDisable(GL_TEXTURE_2D);
//     glPopMatrix();
// }

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
    
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f); // Rotation for texture alignment

    // Desin du corps de la pomme (avec la texture)
    setTexture(textures[0]);
    gluSphere(quadric, 0.3, 32, 32);

    glDisable(GL_TEXTURE_2D);
    glPopMatrix(); // Added missing glPopMatrix()
}

void Fruit::drawOrange(QTime currentTime)
{
    // Positionnement de l'orange
    glPushMatrix();
    QVector3D position = getPosition(currentTime);
    glTranslatef(position.x(), position.y(), position.z());
    
    // Add rotation based on time
    float rotationAngle = startTime.msecsTo(currentTime) / 18.0f;
    glRotatef(rotationAngle, 0.2f, 1.0f, 0.0f);
    
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f); // Rotation pour aligner la texture sur les pôles

    // Dessin de l'orange (avec la texture)
    setTexture(textures[1]);
    gluSphere(quadric, 0.3, 32, 32);

    glDisable(GL_TEXTURE_2D);
    glPopMatrix(); // Added missing glPopMatrix()
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
    glPopMatrix();
}

void Fruit::drawPear(QTime currentTime)
{
    // Positionnement
    glPushMatrix();
    QVector3D position = getPosition(currentTime);
    glTranslatef(position.x(), position.y(), position.z());
    
    // Add rotation based on time
    float rotationAngle = startTime.msecsTo(currentTime) / 22.0f;
    glRotatef(rotationAngle, 0.3f, 1.0f, 0.2f);
    
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f); // Rotation pour aligner la texture sur les pôles
    // Dessin de la poire (avec la texture)
    setTexture(textures[3]);
    gluSphere(quadric, 0.3, 32, 32);
    glDisable(GL_TEXTURE_2D);
    glPopMatrix(); // Added missing glPopMatrix()
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
    
    // Dessin du corps principal de la bombe (sphère)
    setTexture(textures[4]);
    gluSphere(quadric, 0.3, 32, 32);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f); // Rotation pour aligner la texture sur les pôles

    // Dessin de la mèche (cylindre)
    glPushMatrix();
    glTranslatef(0.0f, 0.35f, 0.0f); // Positionner la mèche au-dessus de la sphère
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f); // Aligner le cylindre verticalement
    gluCylinder(quadric, 0.05, 0.05, 0.2, 16, 16); // Cylindre pour la mèche
    glPopMatrix();

    // Désactiver la texture
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}

QVector3D Fruit::getPosition(QTime currentTime)
{
    // Calculate the position of the fruit based on its trajectory
    float t = startTime.msecsTo(currentTime) / 1000.0f;
    float deltaT = startTime.msecsTo(currentTime) / 1000.0f;
    float x = initalSpeed.x() * deltaT + initialPosition.x();
    float y = initalSpeed.y() * deltaT + initialPosition.y() - (0.5f * 9.81f * deltaT * deltaT);
    float z = initalSpeed.z() * deltaT + initialPosition.z();

    //std::cout << "Position: " << x << ", " << y << ", " << z << std::endl;

    return QVector3D(x, y, z);
}
