#include <qvectornd.h>
#include <GL/glu.h>


class Cannon {
private :
    QVector3D position;
    float angleX;
    float angleY;
    float angleZ;
    GLUquadric* quadric;
    GLuint cannonTexture; // Add texture ID
    bool hasTexture;      // Flag to check if texture is set


public:
    Cannon();
    ~Cannon();

    void setPosition(QVector3D position) { this->position = position; }
    void setDirection(QVector3D direction);
    void setTexture(GLuint textureId) { cannonTexture = textureId; hasTexture = true; }
    void drawCannon();
    void onFruitCreated(QVector3D direction);

};