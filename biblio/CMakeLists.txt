cmake_minimum_required(VERSION 3.19)
project(biblio LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets OpenGLWidgets)

# Add OpenCV
find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

cmake_minimum_required(VERSION 3.16)
project(Biblio LANGUAGES CXX)

set(CMAKE_PREFIX_PATH "$ENV{CMAKE_PREFIX_PATH}")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
qt_standard_project_setup()


qt_add_executable(biblio
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    gamewindow.h gamewindow.cpp gamewindow.ui
    camerawidget.h camerawidget.cpp camerawidget.ui
    gamewidget.h gamewidget.cpp gamewidget.ui
    
    fruit.h fruit.cpp
)

target_link_libraries(biblio
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::OpenGLWidgets
        ${OpenCV_LIBS}  # Link OpenCV libraries
        GLU
)

include(GNUInstallDirs)

install(TARGETS biblio
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET biblio
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
