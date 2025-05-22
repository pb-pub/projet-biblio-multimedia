/**
 * @file camerahandler.h
 * @brief Déclaration de la classe CameraHandler.
 * @author Boutet Paul, El Gote Ismaïl
 */

#ifndef CAMERAHANDLER_H
#define CAMERAHANDLER_H

#include <opencv2/opencv.hpp>
#include <opencv2/objdetect.hpp> 
#include <QString>
#include <vector>
#include <opencv2/core/types.hpp> 

/**
 * @class CameraHandler
 * @brief Gère l'accès à la caméra, la capture d'images et la détection de visages.
 *
 * Cette classe encapsule les fonctionnalités OpenCV pour ouvrir une caméra,
 * récupérer des images (frames) et effectuer une détection de visages simple
 * à l'aide d'un classificateur en cascade.
 */
class CameraHandler
{
public:
    /**
     * @brief Constructeur de la classe CameraHandler.
     * Initialise les membres, notamment en tentant de charger le classificateur en cascade.
     */
    CameraHandler();

    /**
     * @brief Destructeur de la classe CameraHandler.
     * Libère la capture vidéo si elle est ouverte.
     */
    ~CameraHandler();

    /**
     * @brief Ouvre la caméra par défaut.
     * @return int Code de statut : 1 en cas de succès, 0 si aucune caméra n'est trouvée, -1 en cas de problème de permission ou autre erreur.
     */
    int openCamera();

    /**
     * @brief Vérifie si la caméra est ouverte et prête à capturer.
     * @return true si la caméra est ouverte, false sinon.
     */
    bool isOpened() const;

    /**
     * @brief Récupère une nouvelle image (frame) de la caméra.
     * @param frame Référence vers un objet cv::Mat qui recevra l'image capturée. (paramètre de sortie)
     * @return true si une image a été récupérée avec succès, false sinon.
     */
    bool getFrame(cv::Mat& frame);
    
    /**
     * @brief Détecte une main dans une image donnée (on remarque que les variables s'appellent
     * face à chaque fois car nous avons reprise le fichier haarcascade que nous avions fait
     * au premiers TDs et pas renommé les variables).
     * @param frame Image couleur d'entrée sur laquelle effectuer la détection. Peut être modifiée pour dessiner les détections.
     * @param grayFrame Image en niveaux de gris (utilisée pour la détection). Si vide, elle sera générée à partir de `frame`.
     * @param thresholdingEnabled Si true, un seuillage peut être appliqué (non implémenté dans la signature actuelle, mais suggéré par le nom).
     * @return std::vector<cv::Point> Liste des centres des visages détectés.
     * @note L'algorithme utilise un classificateur en cascade Haar pour la détection de visages.
     */
    std::vector<cv::Point> detectFaces(cv::Mat& frame, cv::Mat& grayFrame, bool thresholdingEnabled);

    
    
private:
    cv::VideoCapture cap; ///< Objet VideoCapture d'OpenCV pour gérer le flux de la caméra.
    cv::CascadeClassifier faceCascade; ///< Classificateur en cascade OpenCV pour la détection de visages.

    /**
     * @brief Charge le fichier XML du classificateur en cascade pour la détection de main.
     * @return true si le chargement est réussi, false sinon.
     * @note Le chemin vers le fichier cascade est généralement codé en dur ou configurable.
     */
    bool loadFaceCascade();
};

#endif // CAMERAHANDLER_H
