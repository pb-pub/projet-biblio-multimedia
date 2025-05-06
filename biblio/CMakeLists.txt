cmake_minimum_required(VERSION 3.19)
project(biblio LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets OpenGLWidgets)

# Add OpenCV
find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

set(CMAKE_PREFIX_PATH "$ENV{CMAKE_PREFIX_PATH}")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
qt_standard_project_setup()

# Set macOS bundle information
set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.biblio")
set(MACOSX_BUNDLE_BUNDLE_NAME "Biblio")
set(MACOSX_BUNDLE_BUNDLE_VERSION "1.0.0")
set(MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0")
set(MACOSX_BUNDLE_LONG_VERSION_STRING "1.0.0")
set(MACOSX_BUNDLE_COPYRIGHT "© 2025 Biblio")
set(MACOSX_BUNDLE_INFO_STRING "Biblio Multimedia Application")

qt_add_executable(biblio
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    gamewindow.h gamewindow.cpp gamewindow.ui
    camerawidget.h camerawidget.cpp camerawidget.ui
    gamewidget.h gamewidget.cpp gamewidget.ui
    camerahandler.h camerahandler.cpp
    cannon.h cannon.cpp
    gameoverdialog.h gameoverdialog.cpp
    fruit.h fruit.cpp
    settingswindow.h settingswindow.cpp settingswindow.ui
    haarcascade_frontalface_alt.xml
    assets/haarcascade_frontalface_alt.xml
)

# Copy assets to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Fix for macOS bundle resources
if(APPLE)
    # Use MACOSX_BUNDLE_INFO_PLIST property correctly
    set_target_properties(biblio PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in
    )
    
    # Add resources to the bundle using target_sources
    file(GLOB TEXTURE_FILES 
         "${CMAKE_CURRENT_SOURCE_DIR}/assets/textures/*.jpg"
         "${CMAKE_CURRENT_SOURCE_DIR}/assets/textures/*.png"
    )
    file(GLOB FONT_FILES 
         "${CMAKE_CURRENT_SOURCE_DIR}/assets/*.otf"
         "${CMAKE_CURRENT_SOURCE_DIR}/assets/*.ttf"
    )
    
    target_sources(biblio PRIVATE
        ${TEXTURE_FILES}
        ${FONT_FILES}
    )
    
    # Set properties for resource files
    set_source_files_properties(
        ${TEXTURE_FILES}
        ${FONT_FILES}
        PROPERTIES
        MACOSX_PACKAGE_LOCATION Resources
    )
endif()

# Platform-specific OpenGL handling
if(APPLE)
    # On macOS, link to the required frameworks instead of using GLU directly
    target_link_libraries(biblio
        PRIVATE
            Qt::Core
            Qt::Widgets
            Qt::OpenGLWidgets
            ${OpenCV_LIBS}  # Link OpenCV libraries
            "-framework OpenGL"
            "-framework AGL"
    )
else()
    # On other platforms, link to GLU as before
    target_link_libraries(biblio
        PRIVATE
            Qt::Core
            Qt::Widgets
            Qt::OpenGLWidgets
            ${OpenCV_LIBS}  # Link OpenCV libraries
            GLU
    )
endif()

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
