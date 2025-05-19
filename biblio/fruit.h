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
    Fruit(GLuint* textures, QTime currentTime);
    ~Fruit();

    bool isBomb();
    void setType(FruitType type);
    void draw(QTime currentTime);
    QVector3D getPosition(QTime currentTime);
    QVector3D getInitialDirection() { return initalSpeed; }

    void cut(const QVector3D& cutOriginPoint, const QVector3D& cutNormalVector, QTime currentTime);
    bool isCut() const;
    
private :
    FruitType currentFruit;

    void drawApple(QTime currentTime);
    void drawOrange(QTime currentTime);
    void drawBanana(QTime currentTime);
    void drawPear(QTime currentTime);
    void drawBomb(QTime currentTime);

    void setTexture(GLuint textureID);
    void setMaterial(const GLfloat* ambient, const GLfloat* diffuse, const GLfloat* specular, const  GLfloat* shininess);
    
    FruitType getRandomFruitType();
    QVector3D getRandomInitSpeed();
    
    QVector3D initalSpeed;
    QVector3D initialPosition;
    GLUquadric* quadric;    
    GLuint* textures;
    QTime startTime;

    bool m_isCut;
    QVector4D m_clipPlaneEquation;
    QVector3D normal; 
    QTime cutTime;

};



#endif // FRUIT_H
