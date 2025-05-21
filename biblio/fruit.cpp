#include "fruit.h"
#include <iostream>
#include <QImage>
#include <QDir>

Fruit::Fruit(FruitType type, GLuint *textureids, QTime currentTime, QVector3D initSpeed, QVector3D initPosition) : currentFruit(type), textures(textureids), startTime(currentTime), initalSpeed(initSpeed), initialPosition(initPosition), m_isCut(false)
{

    quadric = gluNewQuadric();
    gluQuadricDrawStyle(quadric, GLU_FILL);
    gluQuadricNormals(quadric, GLU_SMOOTH);
    gluQuadricTexture(quadric, GL_TRUE);
}

Fruit::Fruit(FruitType type, GLuint *textureids, QTime currentTime) : currentFruit(type), textures(textureids), startTime(currentTime), initalSpeed(QVector3D(1, 7, -20)), initialPosition(QVector3D(0, 1, 30)), m_isCut(false)
{

    quadric = gluNewQuadric();
    gluQuadricDrawStyle(quadric, GLU_FILL);
    gluQuadricNormals(quadric, GLU_SMOOTH);
    gluQuadricTexture(quadric, GL_TRUE);
}

Fruit::Fruit(GLuint *textureids, QTime currentTime) : textures(textureids), startTime(currentTime), initialPosition(QVector3D(0, 1, 30)), m_isCut(false)
{
    std::cout << "Creating fruit" << std::endl;
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
        return STRAWBERRY;
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
    float x = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.4f - 1.2f;  // Random value between -2 and 2
    float y = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 1.4f + 5.f;   // Random value between 3 and 7
    float z = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 8.0f - 32.0f; // Random value between -35 and -25
    return QVector3D(x, y, z);
}

void Fruit::setType(FruitType type)
{
    currentFruit = type;
}

void Fruit::draw(QTime currentTime)
{
    if (m_isCut)
    {
        GLdouble planeEq[4] = {
            static_cast<GLdouble>(m_clipPlaneEquation.x()),
            static_cast<GLdouble>(m_clipPlaneEquation.y()),
            static_cast<GLdouble>(m_clipPlaneEquation.z()),
            static_cast<GLdouble>(m_clipPlaneEquation.w())};
        glClipPlane(GL_CLIP_PLANE0, planeEq);
        glEnable(GL_CLIP_PLANE0);
    }

    switch (currentFruit)
    {
    case APPLE:
        drawApple(currentTime);
        break;
    case STRAWBERRY:
        drawStrawberry(currentTime);
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

    if (m_isCut)
    {
        GLdouble invertedPlaneEq[4];
        invertedPlaneEq[0] = -static_cast<GLdouble>(m_clipPlaneEquation.x());
        invertedPlaneEq[1] = -static_cast<GLdouble>(m_clipPlaneEquation.y());
        invertedPlaneEq[2] = -static_cast<GLdouble>(m_clipPlaneEquation.z());
        invertedPlaneEq[3] = -static_cast<GLdouble>(m_clipPlaneEquation.w());

        glClipPlane(GL_CLIP_PLANE0, invertedPlaneEq);

        switch (currentFruit)
        {
        case APPLE:
            drawApple(currentTime, -1.f);
            break;
        case STRAWBERRY:
            drawStrawberry(currentTime, -1.f);
            break;
        case BANANA:
            drawBanana(currentTime, -1.f);
            break;
        case PEAR:
            drawPear(currentTime, -1.f);
            break;
        case BOMB:
            drawBomb(currentTime, -1.f);
            break;
        }
        glDisable(GL_CLIP_PLANE0);
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

void Fruit::drawApple(QTime currentTime, float firstPart)
{
    // Positionnement de la pomme
    glPushMatrix();
    QVector3D position = getPosition(currentTime, firstPart);
    glTranslatef(position.x(), position.y(), position.z());

    // Add rotation based on time
    float rotationAngle = startTime.msecsTo(currentTime) / 20.0f; // Adjust divisor for rotation speed
    glRotatef(rotationAngle, 0.0f, 1.0f, 0.3f);                   // Rotate around a slightly tilted axis

    // Set color to white before texturing to avoid tinting the texture
    glColor3f(1.0f, 1.0f, 1.0f);
    // Desin du corps de la pomme (avec la texture)
    setTexture(textures[0]);
    glPushMatrix();                     // Apple body rotation
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f); // Rotation for texture alignment
    gluSphere(quadric, 0.3, 32, 32);
    glPopMatrix(); // End apple body rotation
    glDisable(GL_TEXTURE_2D);

    // Dessin de la tige (stem)
    GLfloat stemAmbient[] = {0.4f, 0.2f, 0.0f, 1.0f};
    GLfloat stemDiffuse[] = {0.5f, 0.25f, 0.0f, 1.0f};
    GLfloat stemSpecular[] = {0.1f, 0.05f, 0.0f, 1.0f};
    GLfloat stemShininess[] = {10.0f};
    setMaterial(stemAmbient, stemDiffuse, stemSpecular, stemShininess);
    glColor3fv(stemDiffuse); // Set color for stem, works with GL_COLOR_MATERIAL

    glPushMatrix();
    glTranslatef(0.0f, 0.3f, 0.0f);                  // Position the stem on top of the apple
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);             // Orient the stem upwards
    gluCylinder(quadric, 0.02, 0.015, 0.15, 16, 16); // Stem
    glPopMatrix();

    // Dessin du calice (bottom part)
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

void Fruit::drawStrawberry(QTime currentTime, float firstPart)
{
    glPushMatrix();
    QVector3D position = getPosition(currentTime, firstPart);
    glTranslatef(position.x(), position.y(), position.z());

    float rotationAngle = startTime.msecsTo(currentTime) / 18.0f;
    glRotatef(rotationAngle, 0.2f, 1.0f, 0.0f);

    glColor3f(1.0f, 1.0f, 1.0f);
    setTexture(textures[1]);

    // Strawberry Body
    const int numSegments = 12; // Number of segments around the strawberry
    const int numSlices = 4;
    float profile[][2] = {
        {-0.25f, 0.0f}, // Bottom tip
        {-0.1f, 0.25f}, // Middle widest part
        {0.15f, 0.2f},  // Tapering towards top
        {0.2f, 0.0f}    // Top tip
    };

    glBegin(GL_TRIANGLES);
    // Iterate through slices
    for (int j = 0; j < numSlices - 1; ++j)
    { 
        float y1 = profile[j][0];
        float r1 = profile[j][1];
        float y2 = profile[j + 1][0];
        float r2 = profile[j + 1][1];

        // Texture coordinates V (vertical)
        float v1_tex = static_cast<float>(j) / (numSlices - 1);
        float v2_tex = static_cast<float>(j + 1) / (numSlices - 1);

        // Iterate around the circumference
        for (int i = 0; i < numSegments; ++i)
        {
            float angle1 = (2.0f * M_PI * i) / numSegments;
            float angle2 = (2.0f * M_PI * (i + 1)) / numSegments;

            // Texture coordinates U (horizontal)
            float u1_tex = static_cast<float>(i) / numSegments;
            float u2_tex = static_cast<float>(i + 1) / numSegments;

            // Vertices for the current quad
            QVector3D p1(r1 * cos(angle1), y1, r1 * sin(angle1));
            QVector3D p2(r1 * cos(angle2), y1, r1 * sin(angle2));
            QVector3D p3(r2 * cos(angle2), y2, r2 * sin(angle2));
            QVector3D p4(r2 * cos(angle1), y2, r2 * sin(angle1));

            // Triangle 1
            glTexCoord2f(u1_tex, v1_tex);
            glVertex3f(p1.x(), p1.y(), p1.z());
            glTexCoord2f(u2_tex, v1_tex);
            glVertex3f(p2.x(), p2.y(), p2.z());
            glTexCoord2f(u2_tex, v2_tex);
            glVertex3f(p3.x(), p3.y(), p3.z());

            // Triangle 2
            glTexCoord2f(u1_tex, v1_tex);
            glVertex3f(p1.x(), p1.y(), p1.z());
            glTexCoord2f(u2_tex, v2_tex);
            glVertex3f(p3.x(), p3.y(), p3.z());
            glTexCoord2f(u1_tex, v2_tex);
            glVertex3f(p4.x(), p4.y(), p4.z());
        }
    }
    glEnd();

    glDisable(GL_TEXTURE_2D); // Disable texturing after drawing strawberry body

    // Reset color to white
    glColor3f(1.0f, 1.0f, 1.0f);

    glPopMatrix(); // End main strawberry transform
}

void Fruit::drawBanana(QTime currentTime, float firstPart)
{
    // Positionnement de la banane
    glPushMatrix();
    QVector3D position = getPosition(currentTime, firstPart);
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
    for (int i = 0; i < numSegments - 1; i++)
    {
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
        float length = sqrt(dx * dx + dz * dz);

        // Rotate to align with the curve
        float angle = atan2(dx, dz) * 180.0f / M_PI;
        glRotatef(angle, 0.0f, 1.0f, 0.0f);

        // Draw the segment
        gluCylinder(quadric, r1, r2, length, 16, 1);

        // Draw end caps for smoother appearance
        if (i == 0)
        {
            gluDisk(quadric, 0.0, r1, 16, 1);
        }
        if (i == numSegments - 2)
        {
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

void Fruit::drawPear(QTime currentTime, float firstPart)
{
    // Positionnement
    glPushMatrix(); // Main pear transform
    QVector3D position = getPosition(currentTime, firstPart);
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
    glTranslatef(0.0f, 0.18f, 0.0f);    // Position the upper, narrower part
    glScalef(0.75f, 1.3f, 0.75f);       // Scale to make it narrower and taller, creating the pear neck shape
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
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);                // Orient stem upwards
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

void Fruit::drawBomb(QTime currentTime, float firstPart)
{
    // Positionnement de la bombe
    glPushMatrix();
    QVector3D position = getPosition(currentTime, firstPart);
    glTranslatef(position.x(), position.y(), position.z());

    float rotationAngle = startTime.msecsTo(currentTime) / 10.0f;
    glRotatef(rotationAngle, 0.0f, -1.0f, 0.0f);
    glColor3f(1.0f, 1.0f, 1.0f);

    // Dessin du corps principal de la bombe
    setTexture(textures[4]);
    glPushMatrix();
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    gluSphere(quadric, 0.3, 32, 32);
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);

    // Dessin de la mèche
    GLfloat fuseAmbient[] = {0.2f, 0.2f, 0.1f, 1.0f};
    GLfloat fuseDiffuse[] = {0.4f, 0.4f, 0.2f, 1.0f};
    GLfloat fuseSpecular[] = {0.1f, 0.1f, 0.05f, 1.0f};
    GLfloat fuseShininess[] = {5.0f};
    setMaterial(fuseAmbient, fuseDiffuse, fuseSpecular, fuseShininess);
    glColor3fv(fuseDiffuse);

    glPushMatrix();
    glTranslatef(0.0f, 0.3f, 0.0f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    gluCylinder(quadric, 0.02, 0.02, 0.2, 16, 16);
    glPopMatrix();

    // Reset color
    glColor3f(1.0f, 1.0f, 1.0f);
    glPopMatrix();
}

QVector3D Fruit::getPosition(QTime currentTime, float firstPart)
{
    // Calculate the position of the fruit based on its trajectory
    float slowdownFactor = 1.5f;
    float deltaT = startTime.msecsTo(currentTime) / 1000.0f;
    deltaT /= slowdownFactor; // Slow down the fruit's fall speed

    float x = initalSpeed.x() * deltaT + initialPosition.x();
    float y = initalSpeed.y() * deltaT + initialPosition.y() - (0.5f * 9.81f * deltaT * deltaT);
    float z = initalSpeed.z() * deltaT + initialPosition.z();

    QVector3D position = QVector3D(x, y, z);

    if (m_isCut)
    {
        float deltaTcut = cutTime.msecsTo(currentTime) / 1000.f + 0.1f;
        deltaTcut /= slowdownFactor;
        x += normal.x() * deltaTcut * firstPart;
        y += normal.y() * deltaTcut * firstPart;
        z += normal.z() * deltaTcut * firstPart;
    }

    // std::cout << "Position: " << x << ", " << y << ", " << z << std::endl;
    return QVector3D(x, y, z);
}

bool Fruit::isBomb()
{
    return currentFruit == BOMB;
}

void Fruit::cut(const QVector3D &cutOriginPoint, const QVector3D &cutNormalVector, const QTime currentTime)
{
    if (m_isCut)
        return;
    m_isCut = true;
    cutTime = currentTime;
    normal = cutNormalVector.normalized();
    // Plane equation: Ax + By + Cz + D = 0f
    // D = - (A*Px + B*Py + C*Pz) = -dot(normal, pointOnPlane)
    float d = -QVector3D::dotProduct(normal, cutOriginPoint);
    m_clipPlaneEquation = QVector4D(normal.x(), normal.y(), normal.z(), d);
    std::cout << "Fruit cut. Plane: " << normal.x() << "x + " << normal.y() << "y + " << normal.z() << "z + " << d << " = 0" << std::endl;
}

bool Fruit::isCut() const
{
    return m_isCut;
}
