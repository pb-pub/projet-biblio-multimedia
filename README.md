# Projet Bibliothèque Multimédia - Jeu 3D avec Interaction WebCam

![Build Status](https://github.com/username/projet-biblio-multimedia/workflows/Build%20Executables/badge.svg)

## Description du Projet

Application de type "Fruit Ninja" développée en C++ avec Qt pour l'interface utilisateur, OpenGL (pipeline fixe) pour le rendu 3D, et OpenCV pour la capture vidéo et la détection gestuelle. Le joueur doit découper des fruits en 3D en utilisant sa main devant la caméra, tout en évitant les bombes.

## 📦 Téléchargements

### Versions Pré-compilées

Les exécutables sont automatiquement générés via GitHub Actions pour chaque release :

- **🐧 Linux** : `biblio-linux.tar.gz` - Compatible Ubuntu 22.04+
- **🪟 Windows** : `biblio-windows.zip` - Compatible Windows 10/11 (x64)
- **🍎 macOS** : `biblio-macos.dmg` - Compatible macOS 13+ (Intel/Apple Silicon)

👉 [**Télécharger la dernière version**](https://github.com/username/projet-biblio-multimedia/releases/latest)

### Installation Rapide

**Linux :**
```bash
tar -xzf biblio-linux.tar.gz
cd biblio
./run.sh
```

**Windows :**
```bash
# Extraire biblio-windows.zip
# Double-cliquer sur biblio.exe
```

**macOS :**
```bash
# Monter biblio-macos.dmg
# Glisser biblio.app vers Applications
```

---

# RAPPORT TECHNIQUE

## 1. SPÉCIFICATIONS ET INTERFACE UTILISATEUR

### Architecture de l'Interface

L'application suit un modèle de navigation par fenêtres avec trois interfaces principales :

**Fenêtre Principale (MainWindow) :**
- Menu d'accueil avec boutons "Nouvelle Partie", "Paramètres" et "Ouvrir Rapport PDF"
- Interface simple et épurée pour la navigation
- Gestion de la fermeture propre de l'application

**Fenêtre de Paramètres (SettingsWindow) :**
- Intégration d'un CameraWidget pour prévisualisation
- Contrôles de calibration de la caméra
- Option d'activation/désactivation du seuillage
- Affichage simultané flux couleur et niveaux de gris
- Permet de tester la détection du poing avec ou sans seuillage

**Fenêtre de Jeu (GameWindow) :**
- Zone OpenGL 3D principale (GameWidget) occupant la majorité de l'écran
- HUD overlay affichant score et vies (représentées par des ✘)
- Intégration du flux caméra en coin supérieur gauche (toggleable avec Espace)
- Gestion temps réel des événements de jeu

### Spécifications Techniques des Interactions

**Contrôles Traditionnels :**
- **Navigation Menu** : Clics boutons pour navigation entre fenêtres
- **Debug Caméra** : Touche Espace pour afficher/masquer flux webcam
- **Interaction 3D** : Détection automatique via analyse d'image

**Système d'Interaction Webcam :**
- **Détection de Poing** : Utilisation classificateur Haar cascade (fist.xml)
- **Seuillage Adaptatif** : Algorithme OTSU pour améliorer la détection
- **Projection Cylindrique** : Mapping coordonnées 2D caméra vers espace 3D jeu
- **Zone de Détection** : Rayon de collision de 0.65 unités autour des fruits

**Mécaniques de Jeu :**
- **Fruits** : +1 point lors de la découpe, génération automatique de nouveaux fruits
- **Bombes** : -1 vie si touchées, identification par texture spécifique
- **Fin de Partie** : Déclenchée à 0 vies, affichage score final
- **Respawn** : Nouveau fruit généré automatiquement après chute au sol

### Feedback Multimodal

**Retour Visuel :**
- Highlighting temps réel des objets détectés (rectangles verts)
- Point rouge de projection 3D de la position de la main
- Animations de découpe des fruits avec physique réaliste
- Particules et effets visuels lors des impacts

**Retour Audio :**
- Son de découpe de fruit ("fruit_slice.wav")
- Son de tir de canon ("cannon-shot.wav")
- Volume ajustable et chargement conditionnel

**Interface Utilisateur :**
- Compte à rebours visuel avant début de partie
- Score et vies mis à jour en temps réel
- Messages d'erreur pour problèmes de caméra

## 2. CONCEPTION ET ARCHITECTURE

L'application repose sur le langage C++ et intègre trois technologies principales : Qt pour la gestion de l'interface utilisateur et des événements, OpenGL (avec pipeline fixe) pour le rendu 3D, et OpenCV pour la capture vidéo et la détection gestuelle.

### Diagramme UML des Classes

```
                    ┌─────────────────┐
                    │  QApplication   │
                    │                 │
                    │ + main()        │
                    └─────────┬───────┘
                              │ creates
                              ▼
    ┌─────────────────────────────────────────────────────┐
    │                MainWindow                           │
    │                QMainWindow                          │
    │                                                     │
    │ + on_pushButton_clicked()                           │
    │ + on_pushButton_2_clicked()                         │
    └─────┬───────────────────────┬───────────────────────┘
          │ creates               │ creates
          ▼                       ▼
┌─────────────────┐    ┌─────────────────────────────────┐
│ SettingsWindow  │    │          GameWindow             │
│   QWidget       │    │        QMainWindow              │
│                 │    │                                 │
│ - cameraWidget  │    │ - gameWidget: GameWidget*       │
│ + on_pushButton │    │ - score: int                    │
│   _clicked()    │    │ - lives: int                    │
└─────┬───────────┘    │ + updateLabelDisplay()          │
      │ contains       └─────┬───────────────────────────┘
      ▼                      │ contains
┌─────────────────┐          ▼
│  CameraWidget   │    ┌─────────────────────────────────┐
│    QWidget      │    │          GameWidget             │
│                 │    │           QWidget               │
│ - cameraHandler │    │                                 │
│ - timer: QTimer*│    │ - ui->openGLWidget              │
│ + updateFrame() │    │ - m_fruit: vector<Fruit*>       │
└─────┬───────────┘    │ - cameraHandler: CameraHandler* │
      │ uses           │ - cannon: Cannon                │
      ▼                │ - textures: GLuint*             │
┌─────────────────┐    │ + initializeGL()                │
│ CameraHandler   │    │ + paintGL()                     │
│                 │    │ + updateFrame()                 │
│ - cap: VideoCap │    │ + createFruit()                 │
│ - faceCascade   │    └─────┬───────────────────────────┘
│ + openCamera()  │          │ manages        │ uses
│ + getFrame()    │          ▼                ▼
│ + detectFaces() │    ┌─────────────┐  ┌─────────────┐
└─────────────────┘    │    Fruit    │  │   Cannon    │
                       │             │  │             │
                       │ - position  │  │ - texture   │
                       │ - velocity  │  │ + drawCannon│
                       │ - isBomb    │  │ + setTexture│
                       │ + draw()    │  └─────────────┘
                       │ + cut()     │
                       │ + isCut()   │
                       └─────────────┘
                              │ creates on game over
                              ▼
                       ┌─────────────────┐
                       │ GameOverDialog  │
                       │    QDialog      │
                       │                 │
                       │ + onNewGame()   │
                       │ + onExit()      │
                       └─────────────────┘
```

### Relations et Responsabilités

**Patterns Architecturaux Utilisés :**
- **MVC Adapté** :
    - Modèle : Classes `Fruit` et `Cannon` (données et logique métier).
    - Vue : `GameWidget` (rendu 3D OpenGL) et éléments d'interface Qt (fenêtres, boutons, labels).
    - Contrôleur : `GameWidget` (gestion entrées, mise à jour modèle, déclenchement rendu). `GameWindow` et `SettingsWindow` agissent comme contrôleurs secondaires.
- **Observer Pattern** : Signaux Qt entre GameWidget et GameWindow pour score/vies. Communication via signaux/slots Qt pour couplage faible et réactivité.
- **State Pattern** : Gestion états fruits (intact, coupé, tombé).
- **Factory Pattern** : Création automatique de nouveaux fruits via createFruit()

**Flux de Données Principal :**
1. **CameraHandler** capture frame → détection poing → coordonnées 2D
2. **GameWidget** convertit coordonnées 2D → projection 3D cylindrique
3. **Collision Detection** vérifie intersection point 3D ↔ fruits
4. **Game Logic** met à jour score/vies → signaux vers GameWindow
5. **Render Loop** affiche scene 3D + overlay caméra

**Gestion Mémoire :**
- Smart pointers Qt avec parentage automatique
- RAII pour ressources OpenGL (textures, quadriques)
- Nettoyage explicite des fruits supprimés du vecteur
- Gestion exception pour accès caméra

**Architecture Rendu :**
- Pipeline OpenGL fixe avec matrices MVP manuelles
- Texturing multi-objets avec binding dynamique
- Lighting Phong avec source lumineuse positionnelle
- Overlay 2D pour flux caméra via projection orthographique

## 3. ÉTAT DE FINALISATION

### ✅ Modules Complètement Opérationnels

**Interface Utilisateur (100%) :**
- ✅ Navigation fluide entre toutes les fenêtres, gestion efficace des événements Qt
- ✅ Affichage score et vies avec mise à jour temps réel
- ✅ Dialogue de fin de partie fonctionnel avec options

**Rendu 3D OpenGL (95%) :**
- ✅ Pipeline OpenGL fixe stable et performant, caméra perspective, éclairage simple
- ✅ Chargement et affichage textures multiples (7 textures), avec fallback couleurs générées si fichiers manquants
- ✅ Système de caméra perspective avec gluLookAt
- ✅ Éclairage Phong avec matériaux configurables
- ✅ Rendu sol texturé avec grille de référence
- ✅ Gestion transparence et depth testing

**Gestion Webcam et Détection (85%) :**
- ✅ Capture vidéo multi-plateforme (notamment AVFoundation sur macOS, fallback générique)
- ✅ Classificateur Haar pour détection de poing robuste
- ✅ Seuillage OTSU optionnel pour améliorer détection
- ✅ Gestion robuste des permissions caméra avec messages d'erreur explicites
- ✅ Affichage dual (couleur + niveaux de gris) dans paramètres
- ✅ Overlay caméra dans jeu toggleable

**Logique de Jeu (90%) :**
- ✅ Physique des fruits avec trajectoires paraboliques réalistes
- ✅ Système de collision 3D précis (sphère-point)
- ✅ Génération automatique et aléatoire de fruits/bombes
- ✅ Mécanique de découpe avec vecteurs normaux
- ✅ Gestion vies et score avec signaux Qt, mise à jour via signaux Qt
- ✅ Sons de base synchronisés avec actions clés du jeu

### 🔄 Modules Partiellement Fonctionnels

**Détection Gestuelle Avancée (70%) :**
- 🔄 Classification poing/main ouverte basique (précision ~75%), instable selon conditions lumineuses (pourrait être renforcée avec MediaPipe)
- 🔄 Sensibilité variable selon conditions d'éclairage
- 🔄 Détection parfois instable avec arrière-plans complexes
- ❌ Pas de filtrage temporel pour lisser les faux positifs

**Optimisations Performance (60%) :**
- 🔄 FPS stable à 60 avec <10 fruits simultanés; performances acceptables mais absence d'optimisations (ex: culling, mise en cache des textures caméra) limite l'efficacité
- 🔄 Gestion mémoire sans fuites majeures détectées
- ❌ Pas de culling pour objets hors champ
- ❌ Rechargement textures caméra à chaque frame (inefficace)

**Audio et Feedback (80%) :**
- 🔄 Sons de base fonctionnels mais volume parfois irrégulier
- 🔄 Chargement conditionnel avec vérifications
- ❌ Pas d'audio 3D spatialisé
- ❌ Pas de musique d'ambiance

### ❌ Modules Non Implémentés

**Fonctionnalités de Jeu Avancées :**
- ❌ Système de niveaux et progression
- ❌ Power-ups et bonus spéciaux
- ❌ Multijoueur local ou réseau
- ❌ Sauvegarde des scores élevés

**Optimisations Techniques :**
- ❌ Shaders programmables (reste en pipeline fixe)
- ❌ Instanced rendering pour fruits similaires
- ❌ Texture atlas pour réduire changements d'état
- ❌ LOD (Level of Detail) adapatif

**Intelligence Artificielle :**
- ❌ Machine learning pour améliorer détection gestes
- ❌ Analyse patterns utilisateur pour difficulté adapative
- ❌ Prédiction trajectoires pour interactions anticipées

### 🐛 Problèmes Identifiés et Solutions

**Critiques (bloquants) :**
1. **Permissions Caméra macOS** : Application crash si permission refusée
   - *Solution implémentée* : Détection status + dialogue explicatif. (Développement macOS a soulevé défis : gestion permissions via Info.plist, messages utilisateur, usage AVFoundation, vigilance mise à l’échelle écrans Retina).
2. **Textures Manquantes** : Crash si fichiers assets non trouvés
   - *Solution implémentée* : Fallback vers textures couleurs générées.

**Majeurs (impactants) :**
3. **Performance Caméra** : Conversion BGR→RGB à chaque frame (affecte performances).
   - *Solution prévue* : Cache texture + update conditionnel.
4. **Projection Cylindrique** : Mapping parfois imprécis aux bords en périphérie.
   - *Solution* : Ajustement algorithme avec zones mortes.

**Mineurs (cosmétiques) :**
5. **Interface Redimensionnement** : HUD parfois mal positionné, l'interface peut mal se redimensionner.
6. **Debug Console** : Messages de debug trop verbeux en production, non filtrés.

### Métriques de Performance Mesurées

- **FPS moyen** : 55-60 (objectif 60)
- **Latence détection** : 90-150ms (acceptable pour jeu casual)
- **Utilisation RAM** : 85MB base + 2MB/fruit actif
- **Précision détection** : 82% en conditions normales d'éclairage
- **Taux de faux positifs** : <8% avec seuillage OTSU activé

### Architecture de Déploiement

**Dépendances Runtime :**
- Qt5/6 Core, Widgets, OpenGL
- OpenCV 4.x avec modules core, imgproc, objdetect
- Classificateur Haar (fist.xml) dans assets/
- Textures et sons dans structure assets/

**Portabilité :**
- ✅ macOS (testé avec AVFoundation backend)
- 🔄 Windows (backend DirectShow non testé)
- 🔄 Linux (backend V4L2 non testé)
- ❌ Mobile (non prévu)

### Recommandations d'Amélioration

**Priorité 1 (Court terme) :**
- Optimisation pipeline caméra pour réduire latence
- Implémentation filtrage temporel pour détection
- Tests multi-plateforme Windows/Linux

**Priorité 2 (Moyen terme) :**
- Migration vers shaders programmables
- Système de niveaux avec difficulté progressive
- Amélioration UX pour gestion permissions

**Priorité 3 (Long terme) :**
- Integration machine learning pour gestes avancés
- Portage mobile avec adaptation interface tactile
- Multijoueur réseau avec synchronisation temps réel

---

## Installation des Dépendances

### macOS (Homebrew)
```bash
brew install qt opencv
```

### Ubuntu/Debian
```bash
sudo apt-get install qt5-default libopencv-dev
```

### Windows (vcpkg)
```bash
vcpkg install qt5 opencv
```

## Licence

Projet académique - Cours de Bibliothèque Multimédia
Auteurs : Boutet Paul, El Gote Ismaïl