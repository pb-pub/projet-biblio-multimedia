#ifdef __APPLE__
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

#include "gamewidget.h"
#include "ui_gamewidget.h"
#include <QFontDatabase>
#include <QTimer>
#include <iostream>
#include <QTime>
#include <QDir>
#include <QCoreApplication>
#include <QPainter>
#include <QDebug>
#include <QOpenGLWidget>
#include <opencv2/imgproc.hpp>
#include <QKeyEvent>
#include <QSoundEffect>
#include <QUrl>

// Constants
const float MAX_DIMENSION = 33.0f;

GameWidget::GameWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::GameWidget), m_fruit(std::vector<Fruit *>()), m_cameraTextureId(0) // Initialize camera texture ID
      ,
      m_sliceSound(new QSoundEffect(this)) // Initialize sound effect
      ,
      m_shootSound(new QSoundEffect(this)) // Initialize bomb sound
{
    ui->setupUi(this);
    displayCamera = true;            // Enable camera display for demonstration
    setFocusPolicy(Qt::StrongFocus); // Ensure the widget can receive key press events

    // Set up a timer for animation updates
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &GameWidget::updateFruitDisplay);
    timer->start(16); // ~60 FPS (16ms entre les frames)

    if (ui->openGLWidget)
    {
        class CustomGLWidget : public QOpenGLWidget
        {
        public:
            CustomGLWidget(GameWidget *parent) : QOpenGLWidget(parent), m_parent(parent) {}

        protected:
            void initializeGL() override { m_parent->initializeGL(); }
            void resizeGL(int w, int h) override { m_parent->resizeGL(w, h); }
            void paintGL() override { m_parent->paintGL(); }

        private:
            GameWidget *m_parent;
        };

        // Replace the existing widget with our custom one
        QOpenGLWidget *oldWidget = ui->openGLWidget;
        QLayout *layout = oldWidget->parentWidget()->layout();

        CustomGLWidget *customWidget = new CustomGLWidget(this);
        if (layout)
        {
            layout->replaceWidget(oldWidget, customWidget);
        }

        ui->openGLWidget = customWidget;
        delete oldWidget;
    }

    label = new QLabel("Fruit Ninja", this);

    int fontId = QFontDatabase::addApplicationFont("./../../../biblio/assets/NinjaStrike.otf");
    QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
    if (!fontFamilies.isEmpty())
    {
        QFont m_font(fontFamilies.first(), 150);
        label->setFont(m_font);
    }

    // add a label on the center of the window
    label->setAlignment(Qt::AlignCenter);

    // set the label in red
    label->setStyleSheet("QLabel { color : red }");
    // center the label
    label->setGeometry(0, 0, parent->width(), parent->height());
    label->setParent(this);
    label->setAttribute(Qt::WA_TransparentForMouseEvents); // Make label transparent to mouse events

    startCountdown(3); // Start countdown from 3 seconds

    // Initialize camera
    initializeCamera();

    // Initialize sounds
    QString soundBasePath = QCoreApplication::applicationDirPath() + "/../../../biblio/assets/sounds/";
    m_sliceSound->setSource(QUrl::fromLocalFile(soundBasePath + "fruit_slice.wav"));
    m_sliceSound->setVolume(1.f);

    m_shootSound->setSource(QUrl::fromLocalFile(soundBasePath + "cannon-shot.wav"));
    m_shootSound->setVolume(1.f);
}

GameWidget::~GameWidget()
{
    // Stop camera timer before destruction
    if (cameraTimer)
    {
        cameraTimer->stop();
        delete cameraTimer;
    }

    delete ui;
    delete label;
    delete[] textures; // Note: This deletes the array, not GL textures. Consider glDeleteTextures for 'textures' array.
    for (auto fruit : m_fruit)
    {
        delete fruit;
    }
    m_fruit.clear();
    if (cylinder)
    {
        gluDeleteQuadric(cylinder);
    }

    if (m_cameraTextureId != 0)
    {
        glDeleteTextures(1, &m_cameraTextureId);
    }

    delete cameraHandler;
}

void GameWidget::updateFruitDisplay()
{
    // Update time for animation

    if (ui->openGLWidget)
    {
        // Force an update to the OpenGL widget, which should trigger paintGL
        ui->openGLWidget->update();
    }
}

void GameWidget::initializeGL()
{
    // Activation de l'éclairage
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);

    // Configuration de la source lumineuse
    GLfloat light_position[] = {0.0f, 5.0f, 5.0f, 1.0f};
    GLfloat light_ambient[] = {0.2f, 0.2f, 0.2f, 1.0f};
    GLfloat light_diffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat light_specular[] = {1.0f, 1.0f, 1.0f, 1.0f};

    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

    // Clear color - Change to blue for the sky
    glClearColor(0.53f, 0.81f, 0.98f, 1.0f); // Sky blue color

    initializeTextures();

    // Initialize camera texture
    if (m_cameraTextureId == 0)
    {
        glGenTextures(1, &m_cameraTextureId);
        glBindTexture(GL_TEXTURE_2D, m_cameraTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind
    }
}

void GameWidget::initializeTextures()
{
    // Increase array size to include cannon texture
    textures = new GLuint[7]; // Changed from 6 to 7 to include cannon texture
    glGenTextures(7, textures);

    // Get the application directory and build absolute paths
    QDir appDir(QCoreApplication::applicationDirPath());
    qDebug() << "Application directory: " << appDir.absolutePath();

    // Try multiple possible texture locations
    QStringList possibleBasePaths = {
        appDir.absolutePath() + "/../../../biblio/assets/textures/",
        // Relative to current directory
        "./assets/textures/",
        // Try going up directories
        "../assets/textures/",
        "../../assets/textures/",
        "../../../assets/textures/",
        // Absolute path 
        "/Users/ismail/projet-biblio-multimedia/biblio/assets/textures/"};

    QString base_path;
    bool foundPath = false;

    // Find the first valid path that contains at least one texture
    for (const auto &path : possibleBasePaths)
    {
        QDir dir(path);
        if (dir.exists("apple.jpg") || dir.exists("strawberry.jpg"))
        {
            base_path = path;
            foundPath = true;
            qDebug() << "Found texture path: " << base_path;
            break;
        }
    }

    if (!foundPath)
    {
        qCritical() << "Cannot find texture directory. Tried paths:";
        for (const auto &path : possibleBasePaths)
        {
            qCritical() << " - " << path;
        }
    }

    // Load images
    QImage appleImage(base_path + "apple.jpg");
    QImage strawberryImage(base_path + "strawberry.jpg");
    QImage bananaImage(base_path + "banana.jpg");
    QImage pearImage(base_path + "pear.jpg");
    QImage bombImage(base_path + "bomb.jpg");
    QImage floorImage(base_path + "floor.jpg");
    QImage cannonImage(base_path + "cannon.jpg"); // New cannon texture

    // Try loading backup textures if original textures failed
    if (appleImage.isNull())
    {
        qWarning() << "Failed to load apple.jpg, trying backup red.jpg";
        appleImage = QImage(base_path + "red.jpg");
    }
    if (strawberryImage.isNull())
    {
        qWarning() << "Failed to load orange.jpg, trying backup orange_alt.jpg";
        strawberryImage = QImage(base_path + "orange_alt.jpg");
    }

    appleImage = appleImage.convertToFormat(QImage::Format_RGBA8888);
    strawberryImage = strawberryImage.convertToFormat(QImage::Format_RGBA8888);
    bananaImage = bananaImage.convertToFormat(QImage::Format_RGBA8888);
    pearImage = pearImage.convertToFormat(QImage::Format_RGBA8888);
    bombImage = bombImage.convertToFormat(QImage::Format_RGBA8888);
    floorImage = floorImage.convertToFormat(QImage::Format_RGBA8888);   // Convert floor image
    cannonImage = cannonImage.convertToFormat(QImage::Format_RGBA8888); // Convert cannon image

    // check if images are loaded correctly
    if (appleImage.isNull() || strawberryImage.isNull() || bananaImage.isNull() ||
        pearImage.isNull() || bombImage.isNull() || floorImage.isNull() ||
        cannonImage.isNull())
    {
        qCritical() << "Error loading texture images";

        // Create fallback colored textures
        appleImage = createColorTexture(QColor(255, 0, 0));      // Red for apple
        strawberryImage = createColorTexture(QColor(255, 165, 0));   // Orange
        bananaImage = createColorTexture(QColor(255, 255, 0));   // Yellow for banana
        pearImage = createColorTexture(QColor(0, 255, 0));       // Green for pear
        bombImage = createColorTexture(QColor(50, 50, 50));      // Dark gray for bomb
        floorImage = createColorTexture(QColor(0, 0, 255));      // Blue for floor
        cannonImage = createColorTexture(QColor(100, 100, 100)); // Gray for cannon
    }

    // Apple Texture
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, appleImage.width(), appleImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, appleImage.bits());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Strawberry Texture
    glBindTexture(GL_TEXTURE_2D, textures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, strawberryImage.width(), strawberryImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, strawberryImage.bits());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Banana Texture
    glBindTexture(GL_TEXTURE_2D, textures[2]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bananaImage.width(), bananaImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bananaImage.bits());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Pear Texture
    glBindTexture(GL_TEXTURE_2D, textures[3]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pearImage.width(), pearImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, pearImage.bits());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Bomb Texture
    glBindTexture(GL_TEXTURE_2D, textures[4]); // Use index 4 for bomb (was using 4 even though array was sized 4)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bombImage.width(), bombImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bombImage.bits());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Floor Texture
    glBindTexture(GL_TEXTURE_2D, textures[5]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, floorImage.width(), floorImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, floorImage.bits());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Cannon Texture
    glBindTexture(GL_TEXTURE_2D, textures[6]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, cannonImage.width(), cannonImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, cannonImage.bits());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFlush(); // Ensure texture uploads are finished
    qDebug() << "Texture IDs: " << textures[0] << " " << textures[1] << " " << textures[2] << " "
             << textures[3] << " " << textures[4] << " " << textures[5] << " " << textures[6];

    // Set the cannon texture
    cannon.setTexture(textures[6]);
}

// Add helper method to create fallback textures
QImage GameWidget::createColorTexture(const QColor &color)
{
    QImage img(256, 256, QImage::Format_RGBA8888);
    img.fill(color);
    return img;
}

// Add new createFruit function
Fruit *GameWidget::createFruit()
{
    Fruit *newFruit = new Fruit(textures, QTime::currentTime());
    m_fruit.push_back(newFruit);
    if (m_shootSound->isLoaded())
    {
        m_shootSound->play();
    }

    // Notify the cannon about the new fruit's direction
    QVector3D fruitDirection = newFruit->getInitialDirection();
    cannon.onFruitCreated(fruitDirection);

    return newFruit;
}

void GameWidget::startCountdown(int seconds)
{
    // Start a countdown and change the label texts
    QTimer *countdownTimer = new QTimer(this);
    connect(countdownTimer, &QTimer::timeout, [this, countdownTimer, seconds]() mutable
            {
        if (seconds > 0) {
            label->setText(QString::number(seconds));
            seconds--;
        } else {
            countdownTimer->stop();
            delete countdownTimer;
            delete label; // Delete the label after countdown

            // Use the new createFruit function instead of direct creation
            createFruit();
        } });
    countdownTimer->start(1000); // Update every second
}

void GameWidget::resizeGL(int width, int height)
{
    // Definition du viewport (zone d'affichage)
    glViewport(0, 0, width, height);

    // Definition de la matrice de projection with perspective
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Use perspective projection instead of orthographic
    GLfloat aspect = width > 0 ? (GLfloat)width / (GLfloat)height : 1.0f;
    gluPerspective(45.0f, aspect, 0.1f, 100.0f);

    // Return to modelview matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void GameWidget::paintGL()
{
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set up camera
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Set camera position with better positioning for perspective view
    gluLookAt(0.0f, 1.8f, -1.f,  // Eye position
                                 // gluLookAt(2.0f, 1.8f, 20.f,        // testing eye position
              0.0f, 1.0f, 25.0f, // Look at position (center)
              0.0f, 1.0f, 0.0f); // Up vector

    GLfloat light_position[] = {5.0f, 5.0f, 5.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    // Draw the cannon
    cannon.drawCannon();

    // Draw a cylinder around the player (in 0,y,0)
    glPushMatrix();
    glTranslatef(0.0f, 4.0f, 0.0f);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f); // Rotate to align with the Z-axis
    cylinder = gluNewQuadric();
    gluQuadricDrawStyle(cylinder, GLU_LINE);
    gluCylinder(cylinder, 1.0f, 1.0f, 4.0f, 32, 32);
    glPopMatrix();

    // Draw the ground with a grid pattern and texture
    glPushMatrix();

    // Set material properties for the floor - adjust for texture
    GLfloat floor_ambient[] = {0.7f, 0.7f, 0.7f, 1.0f}; 
    GLfloat floor_diffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};  
    GLfloat floor_specular[] = {0.2f, 0.2f, 0.2f, 1.0f};
    glMaterialfv(GL_FRONT, GL_AMBIENT, floor_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, floor_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, floor_specular);
    glMaterialf(GL_FRONT, GL_SHININESS, 10.0f);

    // Use the floor texture
    glBindTexture(GL_TEXTURE_2D, textures[5]);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glEnable(GL_TEXTURE_2D);

    // Draw the main ground plane with texture
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f); 

    const float textureRepetition = 20.0f; 

    // Add texture coordinates to the ground quad 
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-MAX_DIMENSION, 0.0f, -MAX_DIMENSION);
    glTexCoord2f(textureRepetition, 0.0f);
    glVertex3f(MAX_DIMENSION, 0.0f, -MAX_DIMENSION);
    glTexCoord2f(textureRepetition, textureRepetition);
    glVertex3f(MAX_DIMENSION, 0.0f, MAX_DIMENSION);
    glTexCoord2f(0.0f, textureRepetition);
    glVertex3f(-MAX_DIMENSION, 0.0f, MAX_DIMENSION);
    glEnd();

    // Disable texturing before drawing the grid
    glDisable(GL_TEXTURE_2D);

    // Add a grid pattern on top of the ground
    GLfloat grid_diffuse[] = {0.1f, 0.4f, 0.1f, 1.0f}; // Darker green for grid lines
    glMaterialfv(GL_FRONT, GL_DIFFUSE, grid_diffuse);

    // Draw grid lines
    const float gridSize = 1.0f;
    const float gridY = 0.01f; // Slightly above the ground to prevent z-fighting

    glBegin(GL_LINES);
    // Draw lines along the Z axis
    for (float x = -MAX_DIMENSION; x <= MAX_DIMENSION; x += gridSize)
    {
        glVertex3f(x, gridY, -MAX_DIMENSION);
        glVertex3f(x, gridY, MAX_DIMENSION);
    }

    // Draw lines along the X axis
    for (float z = -MAX_DIMENSION; z <= MAX_DIMENSION; z += gridSize)
    {
        glVertex3f(-MAX_DIMENSION, gridY, z);
        glVertex3f(MAX_DIMENSION, gridY, z);
    }
    glEnd();

    glPopMatrix();

    // Draw the fruit after re-enabling lighting
    for (auto fruit : m_fruit)
    {

        // if the fruit y coordinate is less than 0, remove it
        if (fruit->getPosition(QTime::currentTime()).y() < 0)
        {
            if (!fruit->isCut() && !fruit->isBomb())
            {
                emit lifeDecrease();
            }
            delete fruit;
            m_fruit.erase(std::remove(m_fruit.begin(), m_fruit.end(), fruit), m_fruit.end());

            // Use the new createFruit function instead of direct creation
            createFruit();
            continue;
        }
        else
        {
            fruit->draw(QTime::currentTime());
        }
    }

    // Draw a ninja blade at the projected point if it exists
    if (hasProjectedPoint)
    {
        glPushMatrix();

        // Set initial material properties
        GLfloat initial_ambient[] = {0.3f, 0.3f, 0.3f, 1.0f};
        GLfloat initial_diffuse[] = {0.8f, 0.8f, 0.8f, 1.0f};
        GLfloat initial_specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
        glMaterialfv(GL_FRONT, GL_AMBIENT, initial_ambient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, initial_diffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, initial_specular);
        glMaterialf(GL_FRONT, GL_SHININESS, 100.0f);

        // Move to the projected point
        glTranslatef(projectedPoint.x(), projectedPoint.y(), projectedPoint.z());
        
        // Rotation pour positionner le katana à l'endroit
        glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);  // Tourner le katana autour de l'axe Y
        
        // Dessiner un katana japonais authentique
        // Réorienter pour aligner correctement le katana dans l'espace 
        glRotatef(-10.0f, 1.0f, 0.0f, 0.0f);  // Incliner très légèrement le katana vers l'avant
        glRotatef(-5.0f, 0.0f, 0.0f, 1.0f);   // Incliner très légèrement sur le côté
        
        // Poignée (tsuka) - Texture noire et rouge pour le Bankai de Ichigo
        // Activer les textures
        glEnable(GL_TEXTURE_2D);
        
        // Créer une texture pour la poignée rouge et noire
        static GLuint handleTexture = 0;
        if (handleTexture == 0) {
            // Créer la texture si elle n'existe pas
            glGenTextures(1, &handleTexture);
            glBindTexture(GL_TEXTURE_2D, handleTexture);
            
            // Générer une texture pour la poignée de Tensa Zangetsu
            // Le tissu est noir avec des détails de rouge sombre
            const int TEX_SIZE = 128; // Texture plus grande pour plus de détails
            unsigned char texData[TEX_SIZE * TEX_SIZE * 4];
            for (int y = 0; y < TEX_SIZE; y++) {
                for (int x = 0; x < TEX_SIZE; x++) {
                    int idx = (y * TEX_SIZE + x) * 4;
                    
                    // Par défaut: noir profond (caractéristique du Tensa Zangetsu)
                    int r = 10;
                    int g = 6;
                    int b = 6;
                    
                    // Motif de tissu tressé - simulation du tressage de la poignée
                    int patternX = x % 16;
                    int patternY = y % 16;
                    bool isInCross = (patternX > 6 && patternX < 10) || (patternY > 6 && patternY < 10);
                    
                    if (isInCross) {
                        r = 25; // Rouge plus sombre
                        g = 3;
                        b = 3;
                    }
                    
                    // Ajouter un peu de variation pour le réalisme
                    int noise = ((x*7 + y*9) % 13);
                    if (noise > 10) {
                        r += 5;
                    }
                    
                    // Simuler la texture du tissu serré
                    if ((x + y/2) % 4 == 0) {
                        r -= 2;
                        g -= 1;
                        b -= 1;
                    }
                    
                    texData[idx] = r;      // R
                    texData[idx + 1] = g;  // G
                    texData[idx + 2] = b;  // B
                    texData[idx + 3] = 255; // A
                }
            }
            
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TEX_SIZE, TEX_SIZE, 0, GL_RGBA, 
                         GL_UNSIGNED_BYTE, texData);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            
            // Génération des mipmaps pour améliorer la qualité visuelle
            glGenerateMipmap(GL_TEXTURE_2D);
        } else {
            glBindTexture(GL_TEXTURE_2D, handleTexture);
        }
        
        // Configurer les matériaux pour améliorer le rendu de la texture
        GLfloat handle_ambient[] = {0.4f, 0.1f, 0.1f, 1.0f};  // Base rouge foncé avec ambient suffisant
        GLfloat handle_diffuse[] = {0.7f, 0.15f, 0.15f, 1.0f}; // Rouge plus intense pour Tensa Zangetsu
        GLfloat handle_specular[] = {0.3f, 0.3f, 0.3f, 1.0f}; // Léger éclat métallique
        glMaterialfv(GL_FRONT, GL_AMBIENT, handle_ambient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, handle_diffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, handle_specular);
        glMaterialf(GL_FRONT, GL_SHININESS, 15.0f);  // Semi-brillant pour le tissu tressé
        
        // Mode de texture modulé avec le matériau
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        
        GLUquadric *handle = gluNewQuadric();
        gluQuadricDrawStyle(handle, GLU_FILL);
        gluQuadricNormals(handle, GLU_SMOOTH);
        gluQuadricTexture(handle, GL_TRUE);  // Activer les coordonnées de texture
        
        // Augmenter les segments pour un meilleur mappage de texture
        // Paramètres: base, top, height, slices, stacks (plus de slices/stacks = plus de détails)
        gluCylinder(handle, 0.025f, 0.023f, 0.28f, 16, 6);  // Poignée plus détaillée
        
        // Désactiver les textures après avoir fini avec la poignée
        glDisable(GL_TEXTURE_2D);
        
        // Pommeau (kashira) - bout de la poignée du Bankai
        glPushMatrix();
        GLfloat kashira_diffuse[] = {0.05f, 0.05f, 0.05f, 1.0f};  // Noir
        glMaterialfv(GL_FRONT, GL_DIFFUSE, kashira_diffuse);
        glTranslatef(0.0f, 0.0f, -0.01f);  // Légèrement en retrait
        gluDisk(handle, 0.0f, 0.025f, 12, 2);  // Base du pommeau
        gluCylinder(handle, 0.025f, 0.025f, 0.01f, 12, 1);  // Côté du pommeau
        
        // Anneau pour attacher la chaîne
        GLfloat ring_diffuse[] = {0.2f, 0.2f, 0.2f, 1.0f};   // Gris métal foncé
        GLfloat ring_specular[] = {0.5f, 0.5f, 0.5f, 1.0f};  // Reflet métallique
        glMaterialfv(GL_FRONT, GL_DIFFUSE, ring_diffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, ring_specular);
        glMaterialf(GL_FRONT, GL_SHININESS, 40.0f);
        glTranslatef(0.0f, 0.02f, -0.01f);  // Déplacer sur le côté de la poignée
        
        GLUquadric* ring = gluNewQuadric();
        gluQuadricDrawStyle(ring, GLU_FILL);
        gluQuadricNormals(ring, GLU_SMOOTH);
        gluQuadricOrientation(ring, GLU_OUTSIDE);
        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);  // Tourner pour que l'anneau soit vertical
        gluDisk(ring, 0.005f, 0.008f, 12, 1);  // Anneau extérieur
        glTranslatef(0.0f, 0.0f, 0.003f);      // Épaisseur de l'anneau
        gluDisk(ring, 0.005f, 0.008f, 12, 1);  // Anneau extérieur (autre côté)
        glTranslatef(0.0f, 0.0f, -0.003f);     // Revenir à la position originale
        
        // Dessiner la chaîne qui pend de l'anneau - caractéristique du Tensa Zangetsu
        const int chain_links = 8;
        const float link_radius = 0.004f;
        const float link_thickness = 0.0015f;
        const float chain_length = 0.15f;
        
        for (int i = 0; i < chain_links; i++) {
            float t = (float)i / (chain_links - 1);
            // Position en courbe de la chaîne
            float y_offset = -(t * chain_length);
            float x_offset = 0.01f * sin(t * 3.14159f * 2);  // Légère courbure
            
            glPushMatrix();
            
            // Alternance de l'orientation des maillons pour un effet de chaîne réaliste
            if (i % 2 == 0) {
                glTranslatef(x_offset, y_offset, 0.0f);
                gluDisk(ring, 0.0f, link_radius, 12, 1);
                glTranslatef(0.0f, 0.0f, link_thickness);
                gluDisk(ring, 0.0f, link_radius, 12, 1);
                gluCylinder(ring, link_radius, link_radius, link_thickness, 12, 1);
            } else {
                glTranslatef(x_offset, y_offset, 0.0f);
                glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
                gluDisk(ring, 0.0f, link_radius, 12, 1);
                glTranslatef(0.0f, 0.0f, link_thickness);
                gluDisk(ring, 0.0f, link_radius, 12, 1);
                gluCylinder(ring, link_radius, link_radius, link_thickness, 12, 1);
            }
            
            glPopMatrix();
        }
        
        // Ornement au bout de la chaîne (petit fragment)
        GLfloat charm_diffuse[] = {0.2f, 0.2f, 0.2f, 1.0f};
        glMaterialfv(GL_FRONT, GL_DIFFUSE, charm_diffuse);
        glTranslatef(0.01f, -chain_length, 0.0f);
        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        gluCylinder(ring, 0.006f, 0.002f, 0.012f, 6, 1);  // Petit ornement conique
        
        gluDeleteQuadric(ring);
        glPopMatrix();
        
        // Menuki (décorations sur la poignée)
        glPushMatrix();
        GLfloat menuki_diffuse[] = {0.85f, 0.65f, 0.1f, 1.0f};  // Or satiné
        GLfloat menuki_specular[] = {0.5f, 0.4f, 0.1f, 1.0f};
        glMaterialfv(GL_FRONT, GL_DIFFUSE, menuki_diffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, menuki_specular);
        glMaterialf(GL_FRONT, GL_SHININESS, 50.0f);
        
        // Première décoration
        glTranslatef(0.02f, 0.0f, 0.08f);
        glScalef(0.01f, 0.01f, 0.01f);
        gluSphere(handle, 1.0f, 8, 8);
        
        // Deuxième décoration  
        glTranslatef(-4.0f, 0.0f, 8.0f);
        gluSphere(handle, 1.0f, 8, 8);
        glPopMatrix();
        
        // Tressage de la poignée (tsuka-ito)
        glPushMatrix();
        GLfloat ito_diffuse[] = {0.1f, 0.1f, 0.1f, 1.0f};  // Noir traditionnel
        glMaterialfv(GL_FRONT, GL_DIFFUSE, ito_diffuse);
        
        // Simuler le tressage avec de petits rectangles en relief
        float wrap_spacing = 0.025f;
        int num_wraps = 8;
        float wrap_width = 0.005f;
        float wrap_height = 0.029f;
        
        for (int i = 0; i < num_wraps; i++) {
            glPushMatrix();
            glTranslatef(0.0f, 0.0f, 0.03f + i * wrap_spacing);
            
            // Alternance du pattern
            if (i % 2 == 0) {
                // Tressage horizontal
                glBegin(GL_QUADS);
                glNormal3f(1.0f, 0.0f, 0.0f);
                glVertex3f(wrap_height/2, -wrap_width/2, 0.0f);
                glVertex3f(wrap_height/2, wrap_width/2, 0.0f);
                glVertex3f(wrap_height/2, wrap_width/2, wrap_width);
                glVertex3f(wrap_height/2, -wrap_width/2, wrap_width);
                
                glVertex3f(-wrap_height/2, -wrap_width/2, 0.0f);
                glVertex3f(-wrap_height/2, wrap_width/2, 0.0f);
                glVertex3f(-wrap_height/2, wrap_width/2, wrap_width);
                glVertex3f(-wrap_height/2, -wrap_width/2, wrap_width);
                glEnd();
            } else {
                // Tressage vertical
                glBegin(GL_QUADS);
                glNormal3f(0.0f, 1.0f, 0.0f);
                glVertex3f(-wrap_width/2, wrap_height/2, 0.0f);
                glVertex3f(wrap_width/2, wrap_height/2, 0.0f);
                glVertex3f(wrap_width/2, wrap_height/2, wrap_width);
                glVertex3f(-wrap_width/2, wrap_height/2, wrap_width);
                
                glVertex3f(-wrap_width/2, -wrap_height/2, 0.0f);
                glVertex3f(wrap_width/2, -wrap_height/2, 0.0f);
                glVertex3f(wrap_width/2, -wrap_height/2, wrap_width);
                glVertex3f(-wrap_width/2, -wrap_height/2, wrap_width);
                glEnd();
            }
            
            glPopMatrix();
        }
        glPopMatrix();
        
        // Garde (tsuba) - Croix du Tensa Zangetsu
        glPushMatrix();
        
        // Activer les textures
        glEnable(GL_TEXTURE_2D);
        
        // Créer une texture pour la garde en croix noire
        static GLuint tsubaTexture = 0;
        if (tsubaTexture == 0) {
            // Créer la texture si elle n'existe pas
            glGenTextures(1, &tsubaTexture);
            glBindTexture(GL_TEXTURE_2D, tsubaTexture);
            
            // Générer une texture noire avec motif métallique pour la garde en croix
            const int TEX_SIZE = 128; // Plus détaillé
            unsigned char texData[TEX_SIZE * TEX_SIZE * 4];
            for (int y = 0; y < TEX_SIZE; y++) {
                for (int x = 0; x < TEX_SIZE; x++) {
                    int idx = (y * TEX_SIZE + x) * 4;
                    
                    // Base noir métallique pour la garde du Tensa Zangetsu
                    int value_r = 8;   // Noir avec teinte rouge subtile
                    int value_g = 6;
                    int value_b = 6;
                    
                    // Simuler des traces d'usure caractéristiques de l'arme d'Ichigo
                    float noiseA = sin(x * 0.2f + y * 0.3f) * 8.0f;
                    float noiseB = cos(x * 0.1f - y * 0.15f) * 5.0f;
                    float combinedNoise = noiseA + noiseB;
                    
                    if (combinedNoise > 10.0f) {
                        value_r += 8;  // Traces rougeâtres
                        value_g += 2;
                    } else if (combinedNoise < -10.0f) {
                        value_r += 2;  // Traces plus sombres
                        value_g += 1;
                        value_b += 3;
                    }
                    
                    // Ajouter un motif métallique
                    int patternNoise = (x*7 + y*5) % 17;
                    if (patternNoise > 14) {
                        value_r += 7;
                        value_g += 7;
                        value_b += 10;  // Légères marques bleutées
                    }
                    
                    // Bord légèrement plus brillant (éclat métallique)
                    int distToEdge = std::min(std::min(x, TEX_SIZE-x-1), std::min(y, TEX_SIZE-y-1));
                    if (distToEdge < 3) {
                        value_r += 20;
                        value_g += 20;
                        value_b += 25;  // Éclat bleuté sur les bords
                    }
                    
                    // Limiter aux valeurs valides (0-255)
                    texData[idx]     = std::min(255, std::max(0, value_r));      // R
                    texData[idx + 1] = std::min(255, std::max(0, value_g));      // G
                    texData[idx + 2] = std::min(255, std::max(0, value_b));      // B
                    texData[idx + 3] = 255;    // A - complètement opaque
                }
            }
            
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TEX_SIZE, TEX_SIZE, 0, GL_RGBA, 
                         GL_UNSIGNED_BYTE, texData);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            
            // Génération des mipmaps pour la qualité visuelle
            glGenerateMipmap(GL_TEXTURE_2D);
        } else {
            glBindTexture(GL_TEXTURE_2D, tsubaTexture);
        }
        
        // Configurer matériau pour voir la texture - amélioré pour Tensa Zangetsu
        GLfloat tsuba_ambient[] = {0.18f, 0.15f, 0.18f, 1.0f};  // Noir avec légère teinte bleutée
        GLfloat tsuba_diffuse[] = {0.4f, 0.37f, 0.42f, 1.0f};   // Gris foncé légèrement bleuté
        GLfloat tsuba_specular[] = {0.7f, 0.7f, 0.9f, 1.0f};    // Éclat métallique bleuté
        glMaterialfv(GL_FRONT, GL_AMBIENT, tsuba_ambient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, tsuba_diffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, tsuba_specular);
        glMaterialf(GL_FRONT, GL_SHININESS, 40.0f);  // Plus brillant
        
        // Mode de texture modulé avec le matériau
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        
        glTranslatef(0.0f, 0.0f, 0.28f);  // Positionner à la fin de la poignée
        
        // Garde en forme de croix (Tensa Zangetsu)
        const float crossWidth = 0.02f;    // Épaisseur de la croix
        const float crossLength = 0.08f;   // Longueur des bras
        
        // Barre horizontale de la croix
        glBegin(GL_QUADS);
        // Face avant
        glNormal3f(0.0f, 0.0f, 1.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-crossLength, -crossWidth/2, 0.0f);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(crossLength, -crossWidth/2, 0.0f);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(crossLength, crossWidth/2, 0.0f);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(-crossLength, crossWidth/2, 0.0f);
        
        // Face arrière
        glNormal3f(0.0f, 0.0f, -1.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-crossLength, -crossWidth/2, -crossWidth);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(-crossLength, crossWidth/2, -crossWidth);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(crossLength, crossWidth/2, -crossWidth);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(crossLength, -crossWidth/2, -crossWidth);
        
        // Côté haut
        glNormal3f(0.0f, 1.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-crossLength, crossWidth/2, 0.0f);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(crossLength, crossWidth/2, 0.0f);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(crossLength, crossWidth/2, -crossWidth);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(-crossLength, crossWidth/2, -crossWidth);
        
        // Côté bas
        glNormal3f(0.0f, -1.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-crossLength, -crossWidth/2, 0.0f);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(-crossLength, -crossWidth/2, -crossWidth);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(crossLength, -crossWidth/2, -crossWidth);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(crossLength, -crossWidth/2, 0.0f);
        
        // Extrémité gauche
        glNormal3f(-1.0f, 0.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-crossLength, -crossWidth/2, 0.0f);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(-crossLength, crossWidth/2, 0.0f);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(-crossLength, crossWidth/2, -crossWidth);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(-crossLength, -crossWidth/2, -crossWidth);
        
        // Extrémité droite
        glNormal3f(1.0f, 0.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(crossLength, -crossWidth/2, 0.0f);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(crossLength, -crossWidth/2, -crossWidth);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(crossLength, crossWidth/2, -crossWidth);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(crossLength, crossWidth/2, 0.0f);
        glEnd();
        
        // Barre verticale de la croix
        glBegin(GL_QUADS);
        // Face avant
        glNormal3f(0.0f, 0.0f, 1.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-crossWidth/2, -crossLength, 0.0f);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(crossWidth/2, -crossLength, 0.0f);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(crossWidth/2, crossLength, 0.0f);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(-crossWidth/2, crossLength, 0.0f);
        
        // Face arrière
        glNormal3f(0.0f, 0.0f, -1.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-crossWidth/2, -crossLength, -crossWidth);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(-crossWidth/2, crossLength, -crossWidth);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(crossWidth/2, crossLength, -crossWidth);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(crossWidth/2, -crossLength, -crossWidth);
        
        // Côté haut
        glNormal3f(0.0f, 1.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-crossWidth/2, crossLength, 0.0f);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(crossWidth/2, crossLength, 0.0f);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(crossWidth/2, crossLength, -crossWidth);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(-crossWidth/2, crossLength, -crossWidth);
        
        // Côté bas
        glNormal3f(0.0f, -1.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-crossWidth/2, -crossLength, 0.0f);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(-crossWidth/2, -crossLength, -crossWidth);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(crossWidth/2, -crossLength, -crossWidth);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(crossWidth/2, -crossLength, 0.0f);
        
        // Extrémité gauche
        glNormal3f(-1.0f, 0.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-crossWidth/2, -crossLength, 0.0f);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(-crossWidth/2, crossLength, 0.0f);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(-crossWidth/2, crossLength, -crossWidth);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(-crossWidth/2, -crossLength, -crossWidth);
        
        // Extrémité droite
        glNormal3f(1.0f, 0.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(crossWidth/2, -crossLength, 0.0f);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(crossWidth/2, -crossLength, -crossWidth);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(crossWidth/2, crossLength, -crossWidth);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(crossWidth/2, crossLength, 0.0f);
        glEnd();
        
        // Désactiver la texture après avoir terminé
        glDisable(GL_TEXTURE_2D);
        
        // Trou central pour la lame
        GLfloat hole_diffuse[] = {0.01f, 0.01f, 0.01f, 1.0f};  // Noir profond
        glMaterialfv(GL_FRONT, GL_DIFFUSE, hole_diffuse);
        glTranslatef(0.0f, 0.0f, 0.001f);  // Légèrement devant
        gluDisk(handle, 0.0f, 0.011f, 12, 1);  // Trou pour la lame
        glPopMatrix();
        
        // Collier (habaki) - pièce métallique entre la lame et la garde
        glPushMatrix();
        GLfloat habaki_diffuse[] = {0.7f, 0.6f, 0.3f, 1.0f};  // Bronze-doré
        GLfloat habaki_specular[] = {0.9f, 0.8f, 0.4f, 1.0f};
        glMaterialfv(GL_FRONT, GL_DIFFUSE, habaki_diffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, habaki_specular);
        glMaterialf(GL_FRONT, GL_SHININESS, 70.0f);
        
        glTranslatef(0.0f, 0.0f, 0.28f);  // À la même position que la garde
        gluCylinder(handle, 0.01f, 0.008f, 0.02f, 12, 1);  // Petite pièce conique
        glPopMatrix();
        
        // Lame (ha) - Lame noire du Bankai de Ichigo (Tensa Zangetsu)
        glPushMatrix();
        
        // Activer les textures pour la lame
        glEnable(GL_TEXTURE_2D);
        
        // Créer une texture spécifique pour la lame de Tensa Zangetsu
        static GLuint bladeTexture = 0;
        if (bladeTexture == 0) {
            // Créer la texture si elle n'existe pas
            glGenTextures(1, &bladeTexture);
            glBindTexture(GL_TEXTURE_2D, bladeTexture);
            
            // Générer une texture noire avec un léger motif pour Tensa Zangetsu
            const int BLADE_TEX_SIZE = 128;
            unsigned char bladeTexData[BLADE_TEX_SIZE * BLADE_TEX_SIZE * 4];
            
            for (int y = 0; y < BLADE_TEX_SIZE; y++) {
                for (int x = 0; x < BLADE_TEX_SIZE; x++) {
                    int idx = (y * BLADE_TEX_SIZE + x) * 4;
                    
                    // Base noir profond pour la lame de Tensa Zangetsu
                    int baseColor = 5;  // Noir très profond
                    
                    // Ajouter motif de "vagues" pour simuler l'énergie spirituelle
                    float wave = sin(y * 0.2f + x * 0.1f) * 3.0f;
                    
                    // Ajouter un effet de bord rougeoyant (Reiatsu d'Ichigo)
                    int distToEdgeX = std::min(x, BLADE_TEX_SIZE - x - 1);
                    int distToEdgeY = std::min(y, BLADE_TEX_SIZE - y - 1);
                    int distToEdge = std::min(distToEdgeX, distToEdgeY);
                    
                    int redGlow = 0;
                    int blueGlow = 0;
                    
                    if (distToEdge < 5) {
                        redGlow = (5 - distToEdge) * 20;  // Lueur rouge sur les bords
                        blueGlow = (5 - distToEdge) * 5;   // Légère teinte bleue
                    }
                    
                    // Effets de détails sur la lame
                    float noise = (((x*3)^(y*7)) % 13) / 13.0f;
                    int detail = (int)(noise * 5.0f);
                    
                    // Couleurs finales
                    bladeTexData[idx]     = baseColor + detail + redGlow;           // R
                    bladeTexData[idx + 1] = baseColor + detail;                     // G
                    bladeTexData[idx + 2] = baseColor + detail + blueGlow;          // B
                    bladeTexData[idx + 3] = 255;                                    // A
                }
            }
            
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, BLADE_TEX_SIZE, BLADE_TEX_SIZE, 0, GL_RGBA, 
                         GL_UNSIGNED_BYTE, bladeTexData);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        } else {
            glBindTexture(GL_TEXTURE_2D, bladeTexture);
        }
        
        // Configuration du matériau pour que la texture soit bien visible
        GLfloat blade_ambient[] = {0.15f, 0.05f, 0.05f, 1.0f};   // Légère teinte rouge
        GLfloat blade_diffuse[] = {0.3f, 0.3f, 0.3f, 1.0f};      // Gris foncé pour bien voir la texture
        GLfloat blade_specular[] = {0.9f, 0.6f, 0.6f, 1.0f};     // Reflet rougeâtre brillant
        glMaterialfv(GL_FRONT, GL_AMBIENT, blade_ambient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, blade_diffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, blade_specular);
        glMaterialf(GL_FRONT, GL_SHININESS, 128.0f);  // Très brillant
        
        // Mode de texture combiné avec le matériau pour un meilleur rendu
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); - valeur maximale
        
        // Position de départ de la lame
        glTranslatef(0.0f, 0.0f, 0.3f);
        
        // Dessiner la lame (plus fine et moins courbée pour le Bankai)
        const int blade_segments = 15;            // Plus de segments pour un rendu plus lisse
        const float blade_length = 1.0f;          // Encore plus longue pour être fidèle à Tensa Zangetsu
        const float curve_amount = 0.01f;         // Très légère courbure (quasiment droit)
        const float blade_width_start = 0.022f;   // Plus fine
        const float blade_width_end = 0.003f;     // Pointe très affûtée
        const float blade_height = 0.002f;        // Encore plus fine et tranchante
        
        // Côté supérieur de la lame (partie plate)
        glBegin(GL_TRIANGLE_STRIP);
        for (int i = 0; i <= blade_segments; i++) {
            float t = (float)i / blade_segments;
            float z = t * blade_length;
            float width = blade_width_start * (1.0f - t) + blade_width_end * t;
            
            // Coordonnées de texture qui s'étirent le long de la lame
            float texU = t * 3.0f;  // Répéter la texture 3 fois
            
            // Côté droit
            glNormal3f(0.0f, 1.0f, 0.1f);
            glTexCoord2f(texU, 0.0f);
            glVertex3f(width/2, blade_height/2, z);
            
            // Côté gauche
            glTexCoord2f(texU, 1.0f);
            glVertex3f(-width/2, blade_height/2, z);
        }
        glEnd();
        
        // Côté inférieur de la lame (partie plate)
        glBegin(GL_TRIANGLE_STRIP);
        for (int i = 0; i <= blade_segments; i++) {
            float t = (float)i / blade_segments;
            float z = t * blade_length;
            float width = blade_width_start * (1.0f - t) + blade_width_end * t;
            
            // Coordonnées de texture qui s'étirent le long de la lame
            float texU = t * 3.0f;  // Répéter la texture 3 fois
            
            // Côté droit
            glNormal3f(0.0f, -1.0f, 0.1f);
            glTexCoord2f(texU, 0.0f);
            glVertex3f(width/2, -blade_height/2, z);
            
            // Côté gauche
            glTexCoord2f(texU, 1.0f);
            glVertex3f(-width/2, -blade_height/2, z);
        }
        glEnd();
        
        // Bord tranchant (ha) - côté convexe
        glBegin(GL_TRIANGLE_STRIP);
        for (int i = 0; i <= blade_segments; i++) {
            float t = (float)i / blade_segments;
            float z = t * blade_length;
            float width = blade_width_start * (1.0f - t) + blade_width_end * t;
            
            // Coordonnées de texture pour le bord
            float texU = t * 5.0f;  // Étirer différemment sur le bord
            
            glNormal3f(1.0f, 0.0f, 0.1f);
            glTexCoord2f(texU, 0.0f);
            glVertex3f(width/2, blade_height/2, z);
            glTexCoord2f(texU, 0.1f);
            glVertex3f(width/2, -blade_height/2, z);
        }
        glEnd();
        
        // Dos de la lame (mune) - côté concave
        glBegin(GL_TRIANGLE_STRIP);
        for (int i = 0; i <= blade_segments; i++) {
            float t = (float)i / blade_segments;
            float z = t * blade_length;
            float width = blade_width_start * (1.0f - t) + blade_width_end * t;
            
            // Coordonnées de texture pour le dos
            float texU = t * 5.0f;  // Étirer différemment sur le dos
            
            glNormal3f(-1.0f, 0.0f, 0.1f);
            glTexCoord2f(texU, 0.9f);
            glVertex3f(-width/2, blade_height/2, z);
            glTexCoord2f(texU, 1.0f);
            glVertex3f(-width/2, -blade_height/2, z);
        }
        glEnd();
        
        // Pointe de la lame (kissaki) - améliorée pour Tensa Zangetsu
        glBegin(GL_TRIANGLES);
        glNormal3f(0.0f, 0.0f, 1.0f);
        
        // La pointe de la lame - face supérieure
        glTexCoord2f(1.0f, 0.5f);  // Utiliser un point plus chaud de la texture
        glVertex3f(0.0f, 0.0f, blade_length + 0.025f);  // Pointe légèrement plus longue
        glTexCoord2f(0.8f, 0.0f);
        glVertex3f(blade_width_end/2, blade_height/2, blade_length);
        glTexCoord2f(0.8f, 1.0f);
        glVertex3f(-blade_width_end/2, blade_height/2, blade_length);
        
        // La pointe de la lame - face inférieure
        glTexCoord2f(1.0f, 0.5f);
        glVertex3f(0.0f, 0.0f, blade_length + 0.025f);  // Pointe
        glTexCoord2f(0.8f, 0.0f);
        glVertex3f(blade_width_end/2, -blade_height/2, blade_length);
        glTexCoord2f(0.8f, 1.0f);
        glVertex3f(-blade_width_end/2, -blade_height/2, blade_length);
        
        // Côté droit de la pointe
        glTexCoord2f(1.0f, 0.5f);
        glVertex3f(0.0f, 0.0f, blade_length + 0.025f);  // Pointe
        glTexCoord2f(0.9f, 0.0f);
        glVertex3f(blade_width_end/2, blade_height/2, blade_length);
        glTexCoord2f(0.9f, 1.0f);
        glVertex3f(blade_width_end/2, -blade_height/2, blade_length);
        
        // Côté gauche de la pointe
        glTexCoord2f(1.0f, 0.5f);
        glVertex3f(0.0f, 0.0f, blade_length + 0.025f);  // Pointe
        glTexCoord2f(0.9f, 0.0f);
        glVertex3f(-blade_width_end/2, blade_height/2, blade_length);
        glTexCoord2f(0.9f, 1.0f);
        glVertex3f(-blade_width_end/2, -blade_height/2, blade_length);
        glEnd();
        
        // Effet de lueur sur la pointe (caractéristique du Bankai)
        glDepthMask(GL_FALSE);  // Désactiver écriture dans le depth buffer
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        
        // Matériau émissif pour la lueur
        GLfloat glow_emission[] = {0.3f, 0.0f, 0.0f, 0.2f};  // Lueur rouge subtile
        glMaterialfv(GL_FRONT, GL_EMISSION, glow_emission);
        
        // Dessiner une petite sphère lumineuse à la pointe
        GLUquadric* glowSphere = gluNewQuadric();
        glTranslatef(0.0f, 0.0f, blade_length + 0.02f);
        gluSphere(glowSphere, 0.005f, 8, 8);
        gluDeleteQuadric(glowSphere);
        
        // Réinitialiser les paramètres
        GLfloat no_emission[] = {0.0f, 0.0f, 0.0f, 1.0f};
        glMaterialfv(GL_FRONT, GL_EMISSION, no_emission);
        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
        
        // Désactiver les textures après avoir fini
        glDisable(GL_TEXTURE_2D);
        
        glPopMatrix();
        
        // Nettoyer les ressources
        gluDeleteQuadric(handle);

        glPopMatrix();
    }

    // Display camera feed in top-left corner
    if (displayCamera && cameraInitialized && !currentFrame.empty() && ui->openGLWidget)
    {
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        // Use the QOpenGLWidget's dimensions for ortho projection
        gluOrtho2D(0, ui->openGLWidget->width(), ui->openGLWidget->height(), 0);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_TEXTURE_2D);

        cv::Mat displayFrame;
        // Convert BGR (OpenCV default) to RGBA for OpenGL
        cv::cvtColor(currentFrame, displayFrame, cv::COLOR_BGR2RGBA);

        glBindTexture(GL_TEXTURE_2D, m_cameraTextureId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, displayFrame.cols, displayFrame.rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, displayFrame.data);

        float camFeedWidth = ui->openGLWidget->width() / 4.0f;
        float camFeedHeight = ui->openGLWidget->height() / 4.0f;

        // Preserve aspect ratio of the camera feed
        float camAspectRatio = (float)displayFrame.cols / (float)displayFrame.rows;
        if (camAspectRatio > 0)
        {
            // Adjust height based on width to maintain aspect ratio
            camFeedHeight = camFeedWidth / camAspectRatio;
        }

        glColor3f(1.0f, 1.0f, 1.0f); // Ensure texture is not tinted

        glBegin(GL_QUADS);
        glTexCoord2f(0, 0);
        glVertex2f(0, 0); // Top-left
        glTexCoord2f(1, 0);
        glVertex2f(camFeedWidth, 0); // Top-right
        glTexCoord2f(1, 1);
        glVertex2f(camFeedWidth, camFeedHeight); // Bottom-right
        glTexCoord2f(0, 1);
        glVertex2f(0, camFeedHeight); // Bottom-left
        glEnd();

        glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture

        // Restore GL state
        glEnable(GL_DEPTH_TEST); // Re-enable depth testing
        glEnable(GL_LIGHTING);   // Re-enable lighting (if it was on for 3D scene)

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }
}

void GameWidget::initializeCamera()
{
    cameraHandler = new CameraHandler();
    // Try to open the camera
    int openStatus = cameraHandler->openCamera();

    if (openStatus > 0)
    {
        // Camera opened successfully
        cameraInitialized = true;

        // Set up timer for camera frame updates
        cameraTimer = new QTimer(this);
        connect(cameraTimer, &QTimer::timeout, this, &GameWidget::updateFrame);
        cameraTimer->start(60); // ~15 fps - réduit pour moins charger le CPU

        qDebug() << "Camera initialized successfully";
    }
    else
    {
        // Camera failed to open
        qDebug() << "Failed to initialize camera, error code:" << openStatus;
    }
}

// Variable pour limiter les détections de main
static int frameCounter = 0;
static const int FRAME_SKIP = 2; // Traiter une frame sur 2 pour réduire la charge CPU

void GameWidget::updateFrame()
{
    if (!cameraInitialized || !cameraHandler->isOpened())
    {
        return;
    }

    // Get new frame from camera
    if (cameraHandler->getFrame(currentFrame))
    {
        // Augmenter le compteur de frames
        frameCounter++;
        
        // Ne traiter qu'une frame sur FRAME_SKIP pour réduire la charge CPU
        if (frameCounter % FRAME_SKIP != 0) {
            return;
        }

        // Convert to grayscale for face detection
        cv::cvtColor(currentFrame, grayFrame, cv::COLOR_BGR2GRAY);

        // Detect faces
        std::vector<cv::Point> detectedPoints = cameraHandler->detectFaces(currentFrame, grayFrame, false);

        // Check if any detected point intersects with fruits
        QTime currentTime = QTime::currentTime();

        for (const auto &point : detectedPoints)
        {

            for (auto fruit : m_fruit)
            {
                if (!fruit->isCut() && isFruitHit(point, fruit, currentTime))
                {
                    QVector3D fruitCenter = fruit->getPosition(currentTime);

                    // random cut normal
                    float nx = (rand() % 200 - 100) / 100.0f;
                    float ny = (rand() % 200 - 100) / 100.0f;
                    float nz = 0.f;
                    QVector3D cutNormal(nx, ny, nz);
                    if (cutNormal.lengthSquared() < 0.01f)
                    {
                        cutNormal = QVector3D(1.0f, 0.0f, 0.0f);
                    }
                    cutNormal.normalize();

                    fruit->cut(fruitCenter, cutNormal, currentTime);

                    if (fruit->isBomb())
                    {
                        emit lifeDecrease();
                    }
                    else
                    {
                        emit scoreIncreased();
                        if (m_sliceSound->isLoaded())
                        {
                            m_sliceSound->play();
                        }
                    }
                }
            }
        }

        update();
    }
}

void GameWidget::convertCameraPointToGameSpace(const cv::Point &cameraPoint, float &gameX, float &gameZ)
{
    // Get camera dimensions
    int camWidth = currentFrame.cols;
    int camHeight = currentFrame.rows;

    // Map camera X coordinate to angle around cylinder (0 to π)
    float angle = M_PI - ((float)cameraPoint.x / camWidth) * M_PI;

    // Map camera Y coordinate to height on cylinder (0 to 4)
    float cylinderHeight = 4.0f * (1.0f - (float)cameraPoint.y / camHeight);

    // Set coordinates based on cylinder projection
    gameX = cos(angle); // X coordinate on cylinder
    gameZ = sin(angle); // Z coordinate on cylinder

    // Store the projected point for visualization
    projectedPoint = QVector3D(gameX, cylinderHeight, gameZ);
    hasProjectedPoint = true;

    qDebug() << "Projected point:" << projectedPoint.x() << projectedPoint.y() << projectedPoint.z();
}

// Vecteurs pré-calculés pour les transformations du katana (optimisation des performances)
// Pour ne pas recalculer ces valeurs à chaque frame
static QVector3D cachedBladeDirection;
static bool directionCalculated = false;

bool GameWidget::isFruitHit(const cv::Point &point, Fruit *fruit, QTime currentTime)
{
    // Convert camera point to game space
    float gameX, gameZ;
    convertCameraPointToGameSpace(point, gameX, gameZ);
    
    // Get fruit position
    QVector3D fruitPos = fruit->getPosition(currentTime);
    
    // Optimisation: calcul de la direction une seule fois, et réutilisation
    if (!directionCalculated) {
        // Start with a vector pointing in +Z (sword blade direction)
        QVector3D bladeDirection(0.0f, 0.0f, 1.0f);
        
        // Apply the rotations in reverse order (since we're calculating a direction vector)
        // Pré-calculer les constantes trigonométriques
        const float angleZ = -5.0f * M_PI / 180.0f;
        const float cosZ = cos(angleZ);
        const float sinZ = sin(angleZ);
        const float angleX = -10.0f * M_PI / 180.0f;
        const float cosX = cos(angleX);
        const float sinX = sin(angleX);
        const float angleY = -90.0f * M_PI / 180.0f;
        const float cosY = cos(angleY);
        const float sinY = sin(angleY);
        
        // Appliquer les rotations
        bladeDirection = QVector3D(
            bladeDirection.x() * cosZ - bladeDirection.y() * sinZ,
            bladeDirection.x() * sinZ + bladeDirection.y() * cosZ,
            bladeDirection.z()
        );
        
        bladeDirection = QVector3D(
            bladeDirection.x(),
            bladeDirection.y() * cosX - bladeDirection.z() * sinX,
            bladeDirection.y() * sinX + bladeDirection.z() * cosX
        );
        
        bladeDirection = QVector3D(
            bladeDirection.x() * cosY + bladeDirection.z() * sinY,
            bladeDirection.y(),
            -bladeDirection.x() * sinY + bladeDirection.z() * cosY
        );
        
        // Normalize the direction vector
        bladeDirection.normalize();
        
        // Mémoriser pour les prochains appels
        cachedBladeDirection = bladeDirection;
        directionCalculated = true;
    }
    
    // Calculate blade position using cached direction
    QVector3D bladeBase = projectedPoint;
    QVector3D bladeStartPos = bladeBase + cachedBladeDirection * 0.3f;
    
    // Optimisation: vérifier d'abord si le fruit est proche du milieu de la lame pour un test rapide
    float midPointT = 0.5f;
    QVector3D midBladePoint = bladeStartPos + cachedBladeDirection * (midPointT * 0.8f);
    float dx = midBladePoint.x() - fruitPos.x();
    float dy = midBladePoint.y() - fruitPos.y();
    float dz = midBladePoint.z() - fruitPos.z();
    float midPointDistance = sqrt(dx * dx + dy * dy + dz * dz);
    
    // Si le point central est déjà trop loin, pas besoin de vérifier les autres points
    if (midPointDistance > 0.75f || fruitPos.y() <= 0.5f) {
        return false;
    }
    
    // Si on est assez proche, faire un test plus précis avec 3 points au lieu de 5
    const int numPoints = 3;  // Réduit de 5 à 3 points pour l'optimisation
    
    for (int i = 0; i < numPoints; i++) {
        float t = (i / (float)(numPoints - 1));
        QVector3D bladePoint = bladeStartPos + cachedBladeDirection * (t * 0.8f);
        
        dx = bladePoint.x() - fruitPos.x();
        dy = bladePoint.y() - fruitPos.y();
        dz = bladePoint.z() - fruitPos.z();
        float distance = sqrt(dx * dx + dy * dy + dz * dz);
        
        // Check if distance is less than fruit radius plus a small tolerance
        if (distance < 0.5f) {
            return true;
        }
    }
    
    return false;
}

void GameWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Space)
    {
        displayCamera = !displayCamera;
        if (ui->openGLWidget)
        {
            ui->openGLWidget->update(); // Request a repaint to show/hide the camera feed
        }
        qDebug() << "Camera display toggled:" << displayCamera;
    }
    else
    {
        QWidget::keyPressEvent(event); // Call base class implementation for other keys
    }
}
