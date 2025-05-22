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
    timer->start(50); // ~60 FPS

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

    // Draw a small ball at the projected point if it exists
    if (hasProjectedPoint)
    {
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

        GLUquadric *ball = gluNewQuadric();
        gluQuadricDrawStyle(ball, GLU_FILL);
        gluSphere(ball, 0.05f, 16, 16);
        gluDeleteQuadric(ball);

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
        cameraTimer->start(33); // ~30 fps

        qDebug() << "Camera initialized successfully";
    }
    else
    {
        // Camera failed to open
        qDebug() << "Failed to initialize camera, error code:" << openStatus;
    }
}

void GameWidget::updateFrame()
{
    if (!cameraInitialized || !cameraHandler->isOpened())
    {
        return;
    }

    // Get new frame from camera
    if (cameraHandler->getFrame(currentFrame))
    {
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

bool GameWidget::isFruitHit(const cv::Point &point, Fruit *fruit, QTime currentTime)
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
    float distance = sqrt(dx * dx + dy * dy + dz * dz);

    // Check if distance is less than combined radii (0.5 for fruit + 0.15 for ball = 0.65)
    return distance < 0.65f && fruitPos.y() > 0.5f; // Only count hits when fruit is above ground
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
