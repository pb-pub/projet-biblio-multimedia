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

Fruit::~Fruit()
{
    gluDeleteQuadric(quadric);
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
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f); // Rotation pour aligner la texture sur les pôles

    // Desin du corps de la pomme (avec la texture)
    setTexture(textures[0]);
    gluSphere(quadric, 1.0, 32, 32);

    glDisable(GL_TEXTURE_2D);
}

void Fruit::drawOrange(QTime currentTime)
{
    // Positionnement de l'orange
    glPushMatrix();
    QVector3D position = getPosition(currentTime);
    glTranslatef(position.x(), position.y(), position.z());
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f); // Rotation pour aligner la texture sur les pôles

    // Dessin de l'orange (avec la texture)
    setTexture(textures[1]);
    gluSphere(quadric, 0.3, 32, 32);

    glDisable(GL_TEXTURE_2D);
}

void Fruit::drawBanana(QTime currentTime)
{
    // Positionnement de la banane
    glPushMatrix();
    QVector3D position = getPosition(currentTime);
    glTranslatef(position.x(), position.y(), position.z());
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f); // Rotation pour aligner la texture sur les pôles

    // Dessin de la banane (avec la texture)
    setTexture(textures[2]);
    gluSphere(quadric, 1.0, 32, 32);

    glDisable(GL_TEXTURE_2D);
}

void Fruit::drawPear(QTime currentTime)
{
    // Positionnement
    glPushMatrix();
    QVector3D position = getPosition(currentTime);
    glTranslatef(position.x(), position.y(), position.z());
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f); // Rotation pour aligner la texture sur les pôles
    // Dessin de la poire (avec la texture)
    setTexture(textures[3]);
    gluSphere(quadric, 1.0, 32, 32);
    glDisable(GL_TEXTURE_2D);
}

void Fruit::drawBomb(QTime currentTime)
{
    // Positionnement de la bombe
    glPushMatrix();
    QVector3D position = getPosition(currentTime);
    glTranslatef(position.x(), position.y(), position.z());
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f); // Rotation pour aligner la texture sur les pôles

    // Dessin de la bombe
    gluSphere(quadric, 1.0, 32, 32);
}

QVector3D Fruit::getPosition(QTime currentTime)
{
    // Calculate the position of the fruit based on its trajectory
    float t = startTime.msecsTo(currentTime) / 1000.0f;
    float deltaT = startTime.msecsTo(currentTime) / 1000.0f;
    float x = initalSpeed.x() * deltaT + initialPosition.x();
    float y = initalSpeed.y() * deltaT + initialPosition.y() - (0.5f * 9.81f * deltaT * deltaT);
    float z = initalSpeed.z() * deltaT + initialPosition.z();

    std::cout << "Position: " << x << ", " << y << ", " << z << std::endl;

    return QVector3D(x, y, z);
}
