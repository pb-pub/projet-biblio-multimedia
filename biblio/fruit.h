#ifndef FRUIT_H
#define FRUIT_H

#include <GL/glu.h>
#include <qopengl.h>
#include <QColor>
#include <QVector3D>
#include <QTime>

class Fruit {
public:
    enum FruitType {
        APPLE,
        ORANGE,
        BANANA,
        PEAR,
        BOMB
    };

    Fruit(FruitType type, GLuint* textures, QTime currentTime, QVector3D initSpeed, QVector3D initPosition);
    Fruit(FruitType type, GLuint* textures, QTime currentTime);
    ~Fruit();

    void setType(FruitType type);
    void draw(QTime currentTime);

private :
    FruitType currentFruit;

    void drawApple(QTime currentTime);
    void drawOrange(QTime currentTime);
    void drawBanana(QTime currentTime);
    void drawPear(QTime currentTime);
    void drawBomb(QTime currentTime);

    void setTexture(GLuint textureID);
    void setMaterial(const GLfloat* ambient, const GLfloat* diffuse, const GLfloat* specular, const GLfloat* shininess);

    QVector3D getPosition(QTime currentTime);
    
    QVector3D initalSpeed;
    QVector3D initialPosition;
    GLUquadric* quadric;    
    GLuint* textures;
    QTime startTime;

};



#endif // FRUIT_H