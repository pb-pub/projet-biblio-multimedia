#include "gamewidget.h"
#include "ui_gamewidget.h"
#include <QFontDatabase>
#include <QTimer>
#include <GL/glu.h>
#include <iostream>
#include <QTime>

// Constants
const float MAX_DIMENSION = 10.0f;

GameWidget::GameWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::GameWidget)
    , m_fruit(std::vector<Fruit*>())
{
    ui->setupUi(this);
    
    // Set up a timer for animation updates 
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &GameWidget::updateFruitDisplay);
    timer->start(16); // ~60 FPS

    if (ui->openGLWidget) {
        class CustomGLWidget : public QOpenGLWidget {
        public:
            CustomGLWidget(GameWidget* parent) : QOpenGLWidget(parent), m_parent(parent) {}
        protected:
            void initializeGL() override { m_parent->initializeGL(); }
            void resizeGL(int w, int h) override { m_parent->resizeGL(w, h); }
            void paintGL() override { m_parent->paintGL(); }
        private:
            GameWidget* m_parent;
        };
        
        // Replace the existing widget with our custom one
        QOpenGLWidget* oldWidget = ui->openGLWidget;
        QLayout* layout = oldWidget->parentWidget()->layout();
        
        CustomGLWidget* customWidget = new CustomGLWidget(this);
        if (layout) {
            layout->replaceWidget(oldWidget, customWidget);
        }
        
        ui->openGLWidget = customWidget;
        delete oldWidget;
    }
    

    label = new QLabel("Fruit Ninja", this);
    
    int fontId = QFontDatabase::addApplicationFont("./../../../biblio/assets/NinjaStrike.otf");
    QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
    if (!fontFamilies.isEmpty()) {
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
}

GameWidget::~GameWidget()
{
    delete ui;
    delete label;
    delete[] textures;
    for (auto fruit : m_fruit) {
        delete fruit;
    }
    m_fruit.clear();
    if (cylinder) {
        gluDeleteQuadric(cylinder);
    }
}

void GameWidget::updateFruitDisplay()
{
    // Update time for animation
    
    if (ui->openGLWidget) {
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
    GLfloat light_position[] = { 0.0f, 5.0f, 5.0f, 1.0f };
    GLfloat light_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat light_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

    // Clear color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    initializeTextures();
}

void GameWidget::initializeTextures() {

    // Initialisation des textures
    textures = new GLuint[4];  // Don't initialize to 0
    glGenTextures(4, textures);
    
    QString base_path = "./../../../biblio/assets/textures/";

    // Load images
    QImage appleImage(base_path + "apple.jpg");
    QImage orangeImage(base_path + "orange.jpg");
    QImage bananaImage(base_path + "banana.jpg");
    QImage pearImage(base_path + "pear.jpg");
    QImage bombImage(base_path + "bomb.jpg");

    appleImage = appleImage.convertToFormat(QImage::Format_RGBA8888);
    orangeImage = orangeImage.convertToFormat(QImage::Format_RGBA8888);
    bananaImage = bananaImage.convertToFormat(QImage::Format_RGBA8888);
    pearImage = pearImage.convertToFormat(QImage::Format_RGBA8888);
    bombImage = bombImage.convertToFormat(QImage::Format_RGBA8888);

    // check if images are loaded correctly
    if (appleImage.isNull() || orangeImage.isNull() || bananaImage.isNull() || pearImage.isNull() || bombImage.isNull()) {
        std::cerr << "Error loading texture images" << std::endl;
        return;
    }

    // Apple Texture
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, appleImage.width(), appleImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, appleImage.bits());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    // Orange Texture
    glBindTexture(GL_TEXTURE_2D, textures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, orangeImage.width(), orangeImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, orangeImage.bits());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Banana Texture
    glBindTexture(GL_TEXTURE_2D, textures[2]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bananaImage.width(), bananaImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bananaImage.bits());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Pear Texture
    glBindTexture(GL_TEXTURE_2D, textures[3]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pearImage.width(), pearImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, pearImage.bits());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    // Bomb Texture
    glBindTexture(GL_TEXTURE_2D, textures[4]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bombImage.width(), bombImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bombImage.bits());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


    glFlush();  // Ensure texture uploads are finished
    std::cout << "Texture IDs: " << textures[0] << " " << textures[1] << " " << textures[2] << " " << textures[3] << " " << textures[4] << std::endl;
}

void GameWidget::startCountdown(int seconds) {
    // Start a countdown and change the label texts
    QTimer* countdownTimer = new QTimer(this);
    connect(countdownTimer, &QTimer::timeout, [this, countdownTimer, seconds]() mutable {
        if (seconds > 0) {
            label->setText(QString::number(seconds));
            seconds--;
        } else {
            countdownTimer->stop();
            delete countdownTimer;
            delete label; // Delete the label after countdown

            m_fruit.push_back(new Fruit(textures, QTime::currentTime()));
        }
    });
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
    gluLookAt(0.0f, 1.8f, 0.0f,   // Eye position
              0.0f, 1.0f, 25.0f,     // Look at position (center)
              0.0f, 1.0f, 0.0f);    // Up vector

    // Réinitialiser la position de la lumière après le changement de vue
    GLfloat light_position[] = { 5.0f, 5.0f, 5.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    
    // Draw a cylinder around the player (in 0,y,0)
    glPushMatrix();
    glTranslatef(0.0f, 4.0f, 0.0f);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f); // Rotate to align with the Z-axis
    cylinder = gluNewQuadric();
    gluQuadricDrawStyle(cylinder, GLU_LINE);
    gluCylinder(cylinder, 1.0f, 1.0f, 4.0f, 32, 32);
    glPopMatrix();

    // Draw the ground
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.0f);

    // Set material properties for the floor
    GLfloat floor_ambient[] = {0.2f, 0.2f, 0.2f, 1.0f};
    GLfloat floor_diffuse[] = {0.5f, 0.5f, 0.5f, 1.0f};
    GLfloat floor_specular[] = {0.0f, 0.0f, 0.0f, 1.0f}; // Non-shiny floor
    glMaterialfv(GL_FRONT, GL_AMBIENT, floor_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, floor_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, floor_specular);
    glMaterialf(GL_FRONT, GL_SHININESS, 0.0f);

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f); // Normal pointing up
    glVertex3f(-MAX_DIMENSION, 0.0f, -MAX_DIMENSION);
    glVertex3f(MAX_DIMENSION, 0.0f, -MAX_DIMENSION);
    glVertex3f(MAX_DIMENSION, 0.0f, MAX_DIMENSION);
    glVertex3f(-MAX_DIMENSION, 0.0f, MAX_DIMENSION);
    glEnd();
    glPopMatrix();
    

    // Draw the fruit after re-enabling lighting
    for(auto fruit : m_fruit) {

        // if the fruit y coordinate is less than 0, remove it
        if (fruit->getPosition(QTime::currentTime()).y() < 0) {
            delete fruit;
            m_fruit.erase(std::remove(m_fruit.begin(), m_fruit.end(), fruit), m_fruit.end());
            
            m_fruit.push_back(new Fruit(textures, QTime::currentTime()));
            continue;
        }
        else {
            fruit->draw(QTime::currentTime());
        }

    }
}
