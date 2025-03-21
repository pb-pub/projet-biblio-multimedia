#include "spherewidget.h"

SphereWidget::SphereWidget(QWidget *parent)
    : QOpenGLWidget(parent), quadric(nullptr), currentFruit(ORANGE), bombFlashIntensity(0.0f), bombFlashIncreasing(true)
{
    bombTimer = new QTimer(this);
    connect(bombTimer, &QTimer::timeout, this, &SphereWidget::updateBombFlash);
    bombTimer->setInterval(50);  // 50ms pour une mise à jour fluide
}

SphereWidget::~SphereWidget()
{
    if (quadric) {
        gluDeleteQuadric(quadric);
    }
    if (bombTimer->isActive()) {
        bombTimer->stop();
    }
}

void SphereWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glEnable(GL_DEPTH_TEST);
    
    // Activer l'éclairage
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    
    // Configurer les propriétés de la lumière - lumière blanche pure
    GLfloat light_position[] = { 1.0, 1.0, 2.0, 0.0 }; // Position de la lumière (directionnelle)
    GLfloat light_ambient[] = { 0.3, 0.3, 0.3, 1.0 };  // Composante ambiante blanche
    GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };  // Composante diffuse blanche
    GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 }; // Composante spéculaire blanche
    
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    
    // Configurer les propriétés de matériau pour la sphère
    GLfloat mat_ambient[] = { 0.2, 0.2, 0.2, 1.0 };
    GLfloat mat_diffuse[] = { 0.8, 0.0, 0.0, 1.0 };    // Rouge
    GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat mat_shininess[] = { 50.0 };                // Brillance
    
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
    
    quadric = gluNewQuadric();
    // Configuration du quadrique pour le rendu avec normales (nécessaire pour l'éclairage)
    gluQuadricNormals(quadric, GLU_SMOOTH);
}

void SphereWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)w/h, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

void SphereWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(0, 0, 5, 0, 0, 0, 0, 1, 0);
    
    // Rotation lente de l'objet
    static float angle = 0.0;
    angle += 0.5;
    glRotatef(angle, 0.0, 1.0, 0.0);
    
    // Dessiner le fruit ou la bombe selon le type actuel
    switch (currentFruit) {
        case APPLE:
            drawApple();
            break;
        case ORANGE:
            drawOrange();
            break;
        case BANANA:
            drawBanana();
            break;
        case PEAR:
            drawPear();
            break;
        case BOMB:
            drawBomb();
            break;
    }
    
    glFlush();
    // Demander une mise à jour pour la rotation continue
    update();
}

// Méthode pour dessiner une pomme avec des quadriques
void SphereWidget::drawApple()
{
    // Corps principal de la pomme (rouge)
    GLfloat apple_ambient[] = { 0.3, 0.05, 0.05, 1.0 };  // Composante ambiante rouge
    GLfloat apple_diffuse[] = { 0.8, 0.1, 0.1, 1.0 };    // Rouge pomme pour diffuse
    GLfloat apple_specular[] = { 0.6, 0.6, 0.6, 1.0 };   // Reflet brillant 
    GLfloat apple_shininess[] = { 60.0 };                // Brillance
    
    glMaterialfv(GL_FRONT, GL_AMBIENT, apple_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, apple_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, apple_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, apple_shininess);
    
    // Corps de la pomme (légèrement aplati vers le bas)
    glPushMatrix();
    glScalef(1.0, 0.95, 1.0);
    gluSphere(quadric, 1.0, 32, 32);
    glPopMatrix();
    
    // Petit creux en haut de la pomme
    glPushMatrix();
    glTranslatef(0.0, 1.0, 0.0);
    glRotatef(90.0, 1.0, 0.0, 0.0);
    glScalef(0.3, 0.3, 0.2);
    
    // Couleur marron foncé pour le creux
    GLfloat dark_red_ambient[] = { 0.15, 0.02, 0.02, 1.0 };
    GLfloat dark_red_diffuse[] = { 0.4, 0.05, 0.05, 1.0 };
    GLfloat dark_red_specular[] = { 0.1, 0.1, 0.1, 1.0 };
    
    glMaterialfv(GL_FRONT, GL_AMBIENT, dark_red_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, dark_red_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, dark_red_specular);
    gluDisk(quadric, 0.0, 1.0, 20, 1);
    glPopMatrix();
    
    // Queue de la pomme (marron)
    GLfloat stem_ambient[] = { 0.15, 0.1, 0.0, 1.0 };
    GLfloat stem_diffuse[] = { 0.4, 0.2, 0.0, 1.0 };
    GLfloat stem_specular[] = { 0.2, 0.2, 0.2, 1.0 };
    GLfloat stem_shininess[] = { 10.0 };
    
    glMaterialfv(GL_FRONT, GL_AMBIENT, stem_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, stem_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, stem_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, stem_shininess);
    
    glPushMatrix();
    glTranslatef(0.0, 1.05, 0.0);
    glRotatef(90.0, 1.0, 0.0, 0.0);
    glRotatef(15.0, 0.0, 1.0, 0.0); // Inclinaison légère de la queue
    gluCylinder(quadric, 0.08, 0.05, 0.4, 8, 3);
    glPopMatrix();
    
    // Feuille verte - couleurs optimisées pour un vert plus sombre
    GLfloat leaf_ambient[] = { 0.02, 0.08, 0.01, 1.0 };   // Composante ambiante vert foncé
    GLfloat leaf_diffuse[] = { 0.05, 0.18, 0.02, 1.0 };   // Vert foncé pour diffuse
    GLfloat leaf_specular[] = { 0.03, 0.05, 0.02, 1.0 };  // Reflet très léger
    GLfloat leaf_shininess[] = { 5.0 };                   // Brillance réduite
    
    glMaterialfv(GL_FRONT, GL_AMBIENT, leaf_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, leaf_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, leaf_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, leaf_shininess);
    
    // Feuille verte en forme de larme
    glPushMatrix();
    glTranslatef(0.15, 1.15, -0.05);
    glRotatef(30.0, 0.0, 1.0, 0.0);
    glRotatef(45.0, 1.0, 0.0, 0.0);
    glRotatef(10.0, 0.0, 0.0, 1.0);
    glScalef(0.25, 0.05, 0.4);
    gluSphere(quadric, 1.0, 16, 12);
    glPopMatrix();
    
    // Nervure centrale de la feuille
    glPushMatrix();
    glTranslatef(0.15, 1.165, -0.05);
    glRotatef(30.0, 0.0, 1.0, 0.0);
    glRotatef(45.0, 1.0, 0.0, 0.0);
    glRotatef(10.0, 0.0, 0.0, 1.0);
    
    // Couleur plus foncée pour la nervure
    GLfloat leaf_nerve_ambient[] = { 0.01, 0.05, 0.01, 1.0 };
    GLfloat leaf_nerve_diffuse[] = { 0.03, 0.12, 0.01, 1.0 };
    
    glMaterialfv(GL_FRONT, GL_AMBIENT, leaf_nerve_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, leaf_nerve_diffuse);
    glScalef(0.01, 0.01, 0.4);
    gluCylinder(quadric, 1.0, 0.5, 1.0, 8, 2);
    glPopMatrix();
}

void SphereWidget::drawOrange()
{
    // Corps principal de l'orange (orange vif)
    GLfloat orange_ambient[] = { 0.3, 0.15, 0.0, 1.0 };
    GLfloat orange_diffuse[] = { 0.9, 0.45, 0.0, 1.0 };
    GLfloat orange_specular[] = { 0.7, 0.7, 0.7, 1.0 };
    GLfloat orange_shininess[] = { 40.0 };
    
    glMaterialfv(GL_FRONT, GL_AMBIENT, orange_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, orange_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, orange_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, orange_shininess);
    
    // Forme de l'orange
    gluSphere(quadric, 1.0, 32, 32);
    
    // Texture de l'orange (petits creux)
    GLfloat dark_orange[] = { 0.7, 0.35, 0.0, 1.0 };
    glMaterialfv(GL_FRONT, GL_DIFFUSE, dark_orange);
    
    // Créer quelques petits creux pour la texture de l'orange
    for (int i = 0; i < 40; i++) {
        float theta = (float)rand() / RAND_MAX * 2 * M_PI;
        float phi = (float)rand() / RAND_MAX * M_PI;
        
        float x = sin(phi) * cos(theta);
        float y = sin(phi) * sin(theta);
        float z = cos(phi);
        
        glPushMatrix();
        glTranslatef(x * 0.95, y * 0.95, z * 0.95);
        glScalef(0.05, 0.05, 0.03);
        gluSphere(quadric, 1.0, 4, 4);
        glPopMatrix();
    }
    
    // Tige en haut (vert foncé)
    GLfloat stem_ambient[] = { 0.05, 0.15, 0.05, 1.0 };
    GLfloat stem_diffuse[] = { 0.1, 0.4, 0.1, 1.0 };
    
    glMaterialfv(GL_FRONT, GL_AMBIENT, stem_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, stem_diffuse);
    
    glPushMatrix();
    glTranslatef(0.0, 1.0, 0.0);
    glRotatef(90.0, 1.0, 0.0, 0.0);
    gluCylinder(quadric, 0.05, 0.05, 0.2, 8, 2);
    glPopMatrix();
}

void SphereWidget::drawBanana()
{
    // Couleur jaune de la banane
    GLfloat banana_ambient[] = { 0.3, 0.3, 0.05, 1.0 };
    GLfloat banana_diffuse[] = { 0.9, 0.9, 0.2, 1.0 };
    GLfloat banana_specular[] = { 0.7, 0.7, 0.4, 1.0 };
    GLfloat banana_shininess[] = { 50.0 };
    
    glMaterialfv(GL_FRONT, GL_AMBIENT, banana_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, banana_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, banana_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, banana_shininess);
    
    // Corps principal incurvé de la banane
    glPushMatrix();
    glRotatef(90.0, 0.0, 1.0, 0.0);
    
    // Créer la forme courbée de la banane avec des quadriques
    const int segments = 12;
    const float radius = 0.3f;
    const float length = 2.0f;
    const float curvature = 0.5f;
    
    // Dessiner des sphères en série pour former la banane
    for (int i = 0; i < segments; i++) {
        float t = i / (float)(segments - 1);
        float x = t * length - length/2;
        float y = curvature * sin(t * M_PI);
        
        glPushMatrix();
        glTranslatef(x, y, 0.0);
        glScalef(1.0, 1.0, 0.8); // Légèrement aplati sur l'axe Z
        gluSphere(quadric, radius * (1.0 - 0.5 * fabs(2*t - 1.0)), 16, 16); // Plus fin aux extrémités
        glPopMatrix();
    }
    
    // Extrémités noires de la banane
    GLfloat black_ambient[] = { 0.05, 0.05, 0.05, 1.0 };
    GLfloat black_diffuse[] = { 0.1, 0.1, 0.1, 1.0 };
    
    glMaterialfv(GL_FRONT, GL_AMBIENT, black_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, black_diffuse);
    
    // Extrémité de la tige
    glPushMatrix();
    glTranslatef(-length/2, curvature * sin(0), 0.0);
    glScalef(0.1, 0.1, 0.1);
    gluSphere(quadric, 1.0, 8, 8);
    glPopMatrix();
    
    glPopMatrix();
}

void SphereWidget::drawPear()
{
    // Couleur verte de la poire
    GLfloat pear_ambient[] = { 0.1, 0.15, 0.05, 1.0 };
    GLfloat pear_diffuse[] = { 0.4, 0.6, 0.1, 1.0 };
    GLfloat pear_specular[] = { 0.6, 0.6, 0.3, 1.0 };
    GLfloat pear_shininess[] = { 40.0 };
    
    glMaterialfv(GL_FRONT, GL_AMBIENT, pear_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, pear_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, pear_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, pear_shininess);
    
    // Forme de la poire (partie basse plus large)
    glPushMatrix();
    glRotatef(180.0, 1.0, 0.0, 0.0); // Inverser pour que la partie large soit en bas
    
    // Partie supérieure de la poire (plus fine)
    glPushMatrix();
    glTranslatef(0.0, 0.0, 0.6);
    glScalef(0.5, 0.5, 0.9);
    gluSphere(quadric, 0.8, 32, 32);
    glPopMatrix();
    
    // Partie inférieure de la poire (plus large)
    glPushMatrix();
    glTranslatef(0.0, 0.0, -0.2);
    glScalef(0.9, 0.9, 0.8);
    gluSphere(quadric, 1.0, 32, 32);
    glPopMatrix();
    
    glPopMatrix();
    
    // Queue de la poire (marron)
    GLfloat stem_ambient[] = { 0.15, 0.1, 0.0, 1.0 };
    GLfloat stem_diffuse[] = { 0.3, 0.2, 0.0, 1.0 };
    
    glMaterialfv(GL_FRONT, GL_AMBIENT, stem_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, stem_diffuse);
    
    glPushMatrix();
    glTranslatef(0.0, 1.0, 0.0);
    glRotatef(90.0, 1.0, 0.0, 0.0);
    glRotatef(10.0, 0.0, 1.0, 0.0); // Légère inclinaison
    gluCylinder(quadric, 0.06, 0.04, 0.4, 8, 2);
    glPopMatrix();
    
    // Feuille sur la queue
    GLfloat leaf_ambient[] = { 0.05, 0.1, 0.02, 1.0 };
    GLfloat leaf_diffuse[] = { 0.1, 0.3, 0.05, 1.0 };
    
    glMaterialfv(GL_FRONT, GL_AMBIENT, leaf_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, leaf_diffuse);
    
    glPushMatrix();
    glTranslatef(0.1, 1.2, 0.0);
    glRotatef(30.0, 0.0, 0.0, 1.0);
    glRotatef(60.0, 1.0, 0.0, 0.0);
    glScalef(0.2, 0.05, 0.3);
    gluSphere(quadric, 1.0, 16, 12);
    glPopMatrix();
}

void SphereWidget::drawBomb()
{
    // Couleur noire de la bombe avec effet de flash
    GLfloat flash = bombFlashIntensity;
    GLfloat bomb_ambient[] = { 0.1f + flash * 0.3f, 0.1f, 0.1f, 1.0f };
    GLfloat bomb_diffuse[] = { 0.2f + flash * 0.8f, 0.2f, 0.2f, 1.0f };
    GLfloat bomb_specular[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat bomb_shininess[] = { 80.0f };
    
    glMaterialfv(GL_FRONT, GL_AMBIENT, bomb_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, bomb_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, bomb_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, bomb_shininess);
    
    // Corps sphérique de la bombe
    gluSphere(quadric, 1.0, 32, 32);
    
    // Mèche (cylindre courbé)
    GLfloat fuse_ambient[] = { 0.15, 0.1, 0.05, 1.0 };
    GLfloat fuse_diffuse[] = { 0.5, 0.25, 0.1, 1.0 };
    
    glMaterialfv(GL_FRONT, GL_AMBIENT, fuse_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, fuse_diffuse);
    
    // Base de la mèche
    glPushMatrix();
    glTranslatef(0.0, 0.8, 0.0);
    glRotatef(90.0, 1.0, 0.0, 0.0);
    gluCylinder(quadric, 0.1, 0.1, 0.2, 10, 2);
    glPopMatrix();
    
    // Partie courbée de la mèche
    glPushMatrix();
    glTranslatef(0.0, 1.0, 0.0);
    
    const int segments = 8;
    const float radius = 0.08f;
    const float length = 0.8f;
    
    for (int i = 0; i < segments; i++) {
        float t = i / (float)(segments - 1);
        float x = 0.4 * sin(t * M_PI);
        float y = t * length;
        
        glPushMatrix();
        glTranslatef(x, y, 0.0);
        if (i < segments - 1) {
            float nextT = (i + 1) / (float)(segments - 1);
            float nextX = 0.4 * sin(nextT * M_PI);
            float nextY = nextT * length;
            
            float dx = nextX - x;
            float dy = nextY - y;
            float angle = atan2(dx, dy) * 180.0 / M_PI;
            
            glRotatef(angle, 0.0, 0.0, 1.0);
            gluCylinder(quadric, radius, radius * 0.8, sqrt(dx*dx + dy*dy), 8, 2);
        }
        glPopMatrix();
    }
    
    // Bout de la mèche incandescent
    GLfloat tip_ambient[] = { 0.8f * flash, 0.3f * flash, 0.0f, 1.0f };
    GLfloat tip_diffuse[] = { 1.0f, 0.6f * (1.0f - flash), 0.0f, 1.0f };
    GLfloat tip_emission[] = { 0.6f * flash, 0.3f * flash, 0.0f, 1.0f }; // Émission pour l'effet lumineux
    
    glMaterialfv(GL_FRONT, GL_AMBIENT, tip_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, tip_diffuse);
    glMaterialfv(GL_FRONT, GL_EMISSION, tip_emission);
    
    glTranslatef(0.4 * sin(M_PI), length, 0.0);
    glScalef(0.15, 0.15, 0.15);
    gluSphere(quadric, 1.0, 12, 12);
    
    // Réinitialiser l'émission
    GLfloat no_emission[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glMaterialfv(GL_FRONT, GL_EMISSION, no_emission);
    
    glPopMatrix();
}

void SphereWidget::setFruitType(FruitType type)
{
    currentFruit = type;
    
    // Arrêter le timer de la bombe s'il est activé
    if (bombTimer->isActive() && type != BOMB) {
        bombTimer->stop();
    }
    
    // Démarrer le timer si c'est une bombe
    if (type == BOMB && !bombTimer->isActive()) {
        bombFlashIntensity = 0.0f;
        bombFlashIncreasing = true;
        bombTimer->start();
    }
    
    update();
}

void SphereWidget::updateBombFlash()
{
    // Mettre à jour l'intensité du flash
    if (bombFlashIncreasing) {
        bombFlashIntensity += 0.05f;
        if (bombFlashIntensity >= 1.0f) {
            bombFlashIntensity = 1.0f;
            bombFlashIncreasing = false;
        }
    } else {
        bombFlashIntensity -= 0.05f;
        if (bombFlashIntensity <= 0.0f) {
            bombFlashIntensity = 0.0f;
            bombFlashIncreasing = true;
        }
    }
    update();
}
