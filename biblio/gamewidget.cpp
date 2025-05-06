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

// Constants
const float MAX_DIMENSION = 33.0f;

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
    
    // Initialize camera
    initializeCamera();
}

GameWidget::~GameWidget()
{
    // Stop camera timer before destruction
    if (cameraTimer) {
        cameraTimer->stop();
        delete cameraTimer;
    }
    
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

    // Clear color - Change to blue for the sky
    glClearColor(0.53f, 0.81f, 0.98f, 1.0f);  // Sky blue color

    initializeTextures();
}

void GameWidget::initializeTextures() {
    // Increase array size to include cannon texture
    textures = new GLuint[7];  // Changed from 6 to 7 to include cannon texture
    glGenTextures(7, textures);
    
    // Get the application directory and build absolute paths
    QDir appDir(QCoreApplication::applicationDirPath());
    qDebug() << "Application directory: " << appDir.absolutePath();
    
    // Try multiple possible texture locations
    QStringList possibleBasePaths = {
        // Relative to executable
        appDir.absolutePath() + "/../../../biblio/assets/textures/",
        // Relative to current directory
        "./assets/textures/",
        // Try going up directories
        "../assets/textures/",
        "../../assets/textures/",
        "../../../assets/textures/",
        // Absolute path for debugging
        "/Users/ismail/projet-biblio-multimedia/biblio/assets/textures/"
    };
    
    QString base_path;
    bool foundPath = false;
    
    // Find the first valid path that contains at least one texture
    for (const auto &path : possibleBasePaths) {
        QDir dir(path);
        if (dir.exists("apple.jpg") || dir.exists("orange.jpg")) {
            base_path = path;
            foundPath = true;
            qDebug() << "Found texture path: " << base_path;
            break;
        }
    }
    
    if (!foundPath) {
        qCritical() << "Cannot find texture directory. Tried paths:";
        for (const auto &path : possibleBasePaths) {
            qCritical() << " - " << path;
        }
    }

    // Load images
    QImage appleImage(base_path + "apple.jpg");
    QImage orangeImage(base_path + "orange.jpg");
    QImage bananaImage(base_path + "banana.jpg");
    QImage pearImage(base_path + "pear.jpg");
    QImage bombImage(base_path + "bomb.jpg");
    QImage floorImage(base_path + "floor.jpg");
    QImage cannonImage(base_path + "cannon.jpg");  // New cannon texture
    
    // Try loading backup textures if original textures failed
    if (appleImage.isNull()) {
        qWarning() << "Failed to load apple.jpg, trying backup red.jpg";
        appleImage = QImage(base_path + "red.jpg");
    }
    if (orangeImage.isNull()) {
        qWarning() << "Failed to load orange.jpg, trying backup orange_alt.jpg";
        orangeImage = QImage(base_path + "orange_alt.jpg");
    }

    appleImage = appleImage.convertToFormat(QImage::Format_RGBA8888);
    orangeImage = orangeImage.convertToFormat(QImage::Format_RGBA8888);
    bananaImage = bananaImage.convertToFormat(QImage::Format_RGBA8888);
    pearImage = pearImage.convertToFormat(QImage::Format_RGBA8888);
    bombImage = bombImage.convertToFormat(QImage::Format_RGBA8888);
    floorImage = floorImage.convertToFormat(QImage::Format_RGBA8888);  // Convert floor image
    cannonImage = cannonImage.convertToFormat(QImage::Format_RGBA8888);  // Convert cannon image

    // check if images are loaded correctly
    if (appleImage.isNull() || orangeImage.isNull() || bananaImage.isNull() || 
        pearImage.isNull() || bombImage.isNull() || floorImage.isNull() || 
        cannonImage.isNull()) {
        qCritical() << "Error loading texture images";
        
        // Create fallback colored textures
        appleImage = createColorTexture(QColor(255, 0, 0));     // Red for apple
        orangeImage = createColorTexture(QColor(255, 165, 0));  // Orange
        bananaImage = createColorTexture(QColor(255, 255, 0));  // Yellow for banana
        pearImage = createColorTexture(QColor(0, 255, 0));      // Green for pear
        bombImage = createColorTexture(QColor(50, 50, 50));     // Dark gray for bomb
        floorImage = createColorTexture(QColor(0, 0, 255));     // Blue for floor
        cannonImage = createColorTexture(QColor(100, 100, 100)); // Gray for cannon
    }

    // Apple Texture
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, appleImage.width(), appleImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, appleImage.bits());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Orange Texture
    glBindTexture(GL_TEXTURE_2D, textures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, orangeImage.width(), orangeImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, orangeImage.bits());
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
    glBindTexture(GL_TEXTURE_2D, textures[4]);  // Use index 4 for bomb (was using 4 even though array was sized 4)
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

    glFlush();  // Ensure texture uploads are finished
    qDebug() << "Texture IDs: " << textures[0] << " " << textures[1] << " " << textures[2] << " " 
             << textures[3] << " " << textures[4] << " " << textures[5] << " " << textures[6];
             
    // Set the cannon texture
    cannon.setTexture(textures[6]);
}

// Add helper method to create fallback textures
QImage GameWidget::createColorTexture(const QColor& color) {
    QImage img(256, 256, QImage::Format_RGBA8888);
    img.fill(color);
    return img;
}

// Add new createFruit function
Fruit* GameWidget::createFruit() {
    Fruit* newFruit = new Fruit(textures, QTime::currentTime());
    m_fruit.push_back(newFruit);
    
    // Notify the cannon about the new fruit's direction
    QVector3D fruitDirection = newFruit->getInitialDirection();
    cannon.onFruitCreated(fruitDirection);
    
    return newFruit;
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

            // Use the new createFruit function instead of direct creation
            createFruit();
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
    gluLookAt(0.0f, 1.8f, -1.0f,   // Eye position
    // gluLookAt(2.0f, 1.8f, 20.f,        // testing eye position
              0.0f, 1.0f, 25.0f,     // Look at position (center)
              0.0f, 1.0f, 0.0f);    // Up vector

    // Réinitialiser la position de la lumière après le changement de vue
    GLfloat light_position[] = { 5.0f, 5.0f, 5.0f, 1.0f };
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
    GLfloat floor_ambient[] = {0.7f, 0.7f, 0.7f, 1.0f};  // Brighter to show texture better
    GLfloat floor_diffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};  // Full diffuse to show texture
    GLfloat floor_specular[] = {0.2f, 0.2f, 0.2f, 1.0f}; // Light specular
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
    glNormal3f(0.0f, 1.0f, 0.0f); // Normal pointing up
    
    const float textureRepetition = 20.0f;  // Higher number = more repetition, less stretching
    
    // Add texture coordinates to the ground quad with more repetition
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-MAX_DIMENSION, 0.0f, -MAX_DIMENSION);
    glTexCoord2f(textureRepetition, 0.0f); glVertex3f(MAX_DIMENSION, 0.0f, -MAX_DIMENSION);
    glTexCoord2f(textureRepetition, textureRepetition); glVertex3f(MAX_DIMENSION, 0.0f, MAX_DIMENSION);
    glTexCoord2f(0.0f, textureRepetition); glVertex3f(-MAX_DIMENSION, 0.0f, MAX_DIMENSION);
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
    for (float x = -MAX_DIMENSION; x <= MAX_DIMENSION; x += gridSize) {
        glVertex3f(x, gridY, -MAX_DIMENSION);
        glVertex3f(x, gridY, MAX_DIMENSION);
    }
    
    // Draw lines along the X axis
    for (float z = -MAX_DIMENSION; z <= MAX_DIMENSION; z += gridSize) {
        glVertex3f(-MAX_DIMENSION, gridY, z);
        glVertex3f(MAX_DIMENSION, gridY, z);
    }
    glEnd();
    
    glPopMatrix();
    
    // Draw the fruit after re-enabling lighting
    for(auto fruit : m_fruit) {

        // if the fruit y coordinate is less than 0, remove it
        if (fruit->getPosition(QTime::currentTime()).y() < 0) {
            delete fruit;
            m_fruit.erase(std::remove(m_fruit.begin(), m_fruit.end(), fruit), m_fruit.end());
            
            // Use the new createFruit function instead of direct creation
            createFruit();
            continue;
        }
        else {
            fruit->draw(QTime::currentTime());
        }

    }
    
    // Draw a small ball at the projected point if it exists
    if (hasProjectedPoint) {
        glPushMatrix();
        
        // Set material for the ball (bright red)
        GLfloat ball_ambient[] = {0.5f, 0.0f, 0.0f, 1.0f};
        GLfloat ball_diffuse[] = {1.0f, 0.0f, 0.0f, 1.0f};
        GLfloat ball_specular[] = {1.0f, 0.5f, 0.5f, 1.0f};
        glMaterialfv(GL_FRONT, GL_AMBIENT, ball_ambient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, ball_diffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, ball_specular);
        glMaterialf(GL_FRONT, GL_SHININESS, 50.0f);
        
        // Move to the projected point
        glTranslatef(projectedPoint.x(), projectedPoint.y(), projectedPoint.z());
        
        // Draw a sphere (small ball with radius 0.15)
        GLUquadric* ball = gluNewQuadric();
        gluQuadricDrawStyle(ball, GLU_FILL);
        gluSphere(ball, 0.15f, 16, 16);
        gluDeleteQuadric(ball);
        
        glPopMatrix();
    }
}

void GameWidget::initializeCamera()
{
    // Try to open the camera
    int openStatus = cameraHandler.openCamera();
    
    if (openStatus > 0) {
        // Camera opened successfully
        cameraInitialized = true;
        
        // Set up timer for camera frame updates
        cameraTimer = new QTimer(this);
        connect(cameraTimer, &QTimer::timeout, this, &GameWidget::updateFrame);
        cameraTimer->start(33); // ~30 fps
        
        qDebug() << "Camera initialized successfully";
    } else {
        // Camera failed to open
        qDebug() << "Failed to initialize camera, error code:" << openStatus;
    }
}

void GameWidget::updateFrame()
{
    if (!cameraInitialized || !cameraHandler.isOpened()) {
        return;
    }
    
    // Get new frame from camera
    if (cameraHandler.getFrame(currentFrame)) {
        // Convert to grayscale for face detection
        cv::cvtColor(currentFrame, grayFrame, cv::COLOR_BGR2GRAY);
        
        // Detect faces
        std::vector<cv::Point> detectedPoints = cameraHandler.detectFaces(currentFrame, grayFrame, false);
        
        // Check if any detected point intersects with fruits
        QTime currentTime = QTime::currentTime();
        
        
        for (const auto& point : detectedPoints) {
            std::vector<Fruit*> fruitsToRemove;
            
            for (auto fruit : m_fruit) {
                if (isFruitHit(point, fruit, currentTime)) {
                    fruitsToRemove.push_back(fruit);

                    if (fruit->isBomb()) {
                        // Emit signal for game over
                        emit lifeDecrease();
                    } else {
                        // Emit signal to increase score
                        emit scoreIncreased();
                    }
                }
            }
            
            // Remove hit fruits and add new ones
            for (auto fruitToRemove : fruitsToRemove) {
                m_fruit.erase(std::remove(m_fruit.begin(), m_fruit.end(), fruitToRemove), m_fruit.end());
                delete fruitToRemove;
                // Use the new createFruit function instead of direct creation
                createFruit();
            }
        }
        
        update();
    }
}

void GameWidget::convertCameraPointToGameSpace(const cv::Point& cameraPoint, float& gameX, float& gameZ)
{
    // Get camera dimensions
    int camWidth = currentFrame.cols;
    int camHeight = currentFrame.rows;
    
    // Map camera X coordinate to angle around cylinder (0 to 2π)
    float angle = M_PI - ((float)cameraPoint.x / camWidth) * M_PI;
    
    // Map camera Y coordinate to height on cylinder (0 to 4)
    float cylinderHeight = 4.0f * (1.0f - (float)cameraPoint.y / camHeight);
    
    // Set coordinates based on cylinder projection
    gameX = cos(angle);  // X coordinate on cylinder
    gameZ = sin(angle);  // Z coordinate on cylinder
    
    // Store the projected point for visualization
    projectedPoint = QVector3D(gameX, cylinderHeight, gameZ);
    hasProjectedPoint = true;
    
    qDebug() << "Projected point:" << projectedPoint.x() << projectedPoint.y() << projectedPoint.z();
}

bool GameWidget::isFruitHit(const cv::Point& point, Fruit* fruit, QTime currentTime)
{
    // Convert camera point to game space
    float gameX, gameZ;
    convertCameraPointToGameSpace(point, gameX, gameZ);
    
    // Get fruit position
    QVector3D fruitPos = fruit->getPosition(currentTime);
    
    // Calculate distance in 3D space using the projected point
    float dx = projectedPoint.x() - fruitPos.x();
    float dy = projectedPoint.y() - fruitPos.y();
    float dz = projectedPoint.z() - fruitPos.z();
    float distance = sqrt(dx*dx + dy*dy + dz*dz);
    
    // Check if distance is less than combined radii (0.5 for fruit + 0.15 for ball = 0.65)
    return distance < 0.65f && fruitPos.y() > 0.5f; // Only count hits when fruit is above ground
}

