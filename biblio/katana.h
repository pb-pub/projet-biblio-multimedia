#ifndef KATANA_H
#define KATANA_H

#ifdef __APPLE__
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

#include <QVector3D>

class Katana {
public:
    Katana();
    ~Katana();
    void draw(const QVector3D& position);

private:
    void drawBlade();
    void drawHandle();
    void drawChain();  // Ajout de la déclaration de drawChain
    GLUquadric* quadric;
};

#endif // KATANA_H
