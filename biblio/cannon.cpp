#include "cannon.h"
#include <iostream>
#include <sys/socket.h>
#include "fruit.h"

Cannon::Cannon()
{
    position = QVector3D(0, 1, 30);
    angleX = 0;
    angleY = 0;
    angleZ = 0;
    quadric = gluNewQuadric();
    hasTexture = false;
}

Cannon::~Cannon()
{
    gluDeleteQuadric(quadric);
}

void Cannon::drawCannon()
{
    // Set material properties for the cannon
    GLfloat ambient[] = {0.2f, 0.2f, 0.2f, 1.0f};
    GLfloat diffuse[] = {0.8f, 0.8f, 0.8f, 1.0f};
    GLfloat specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat shininess = 50.0f;

    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, &shininess);

    

    // Draw the cannon base (cubic) with texture
    glPushMatrix();
    glTranslatef(position.x(), position.y() - 1.3f, position.z());
    glScalef(1.5f, 1.f, 1.5f);
    
    // Apply cannon texture if available
    if (hasTexture) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, cannonTexture);
    }
    
    glBegin(GL_QUADS);

    // Face avant (z = +1)
    glNormal3f(0.0, 0.0, 1.0);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0, -1.0, 1.0);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0, -1.0, 1.0);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(0.8, 1.0, 0.8);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.8, 1.0, 0.8);

    // Face arrière (z = -1)
    glNormal3f(0.0, 0.0, -1.0);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0, -1.0, -1.0);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.8, 1.0, -0.8);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(0.8, 1.0, -0.8);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0, -1.0, -1.0);

    // Face gauche (x = -1)
    glNormal3f(-1.0, 0.0, 0.0);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0, -1.0, -1.0);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0, -1.0, 1.0);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.8, 1.0, 0.8);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.8, 1.0, -0.8);

    // Face droite (x = +1)
    glNormal3f(1.0, 0.0, 0.0);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0, -1.0, -1.0);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0, -1.0, 1.0);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(0.8, 1.0, 0.8);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(0.8, 1.0, -0.8);

    // Face supérieure (y = +1)
    glNormal3f(0.0, 1.0, 0.0);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.8, 1.0, -0.8);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.8, 1.0, 0.8);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(0.8, 1.0, 0.8);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(0.8, 1.0, -0.8);

    // Face inférieure (y = -1)
    glNormal3f(0.0, -1.0, 0.0);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0, -1.0, -1.0);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0, -1.0, -1.0);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0, -1.0, 1.0);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0, -1.0, 1.0);

    glEnd();
    
    // If texture was enabled for the base, keep it enabled for barrel
    if (!hasTexture) {
        glDisable(GL_TEXTURE_2D);
    }
    
    glPopMatrix();

    
    // Apply cannon texture if available
    if (hasTexture) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, cannonTexture);
        
        // Enable texture coordinates generation
        gluQuadricTexture(quadric, GL_TRUE);
    }

    // Draw the spherical cannon barrel attach
    glPushMatrix();
    glTranslatef(position.x(), position.y() - 0.1f, position.z());
    gluSphere(quadric, 0.6, 16, 16);
    glPopMatrix();

    // Draw the cannon barrel with texture
    glPushMatrix();
    glTranslatef(position.x(), position.y(), position.z());

    // Rotate the cannon based on the angle
    glRotatef(angleX, 1.0f, 0.0f, 0.0f);
    glRotatef(angleY, 0.0f, 1.0f, 0.0f);
    glRotatef(angleZ, 0.0f, 0.0f, 1.0f);


    // Draw the cannon body (cylinder)
    gluCylinder(quadric, 0.2, 0.2, 3, 16, 16);

    // Draw the cannon barrel (cone)
    glTranslatef(0.0f, 0.0f, 3.f);
    gluCylinder(quadric, 0.2, 0.4, 3, 16, 16);

    // Disable texture if we enabled it
    if (hasTexture) {
        gluQuadricTexture(quadric, GL_FALSE);
        glDisable(GL_TEXTURE_2D);
    }

    glPopMatrix();
}

void Cannon::setDirection(QVector3D direction)
{

    angleY = atan2(direction.x(), direction.z()) * 180.0f / M_PI;

    // Calculate the X rotation (vertical angle)
    float horizontalDistance = sqrt(direction.x() * direction.x() +
                                    direction.z() * direction.z());
    angleX = atan2(direction.y(), horizontalDistance) * 180.0f / M_PI;

    angleZ = 0;
}

void Cannon::onFruitCreated(QVector3D direction)
{
    setDirection(direction);
}
