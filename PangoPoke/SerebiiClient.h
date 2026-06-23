#pragma once

#include <QObject>
#include <QPixmap>
#include <QString>
#include <QUrl>

// Forward declarations
class QNetworkAccessManager;
class QNetworkReply;

class SerebiiClient : public QObject
{
    Q_OBJECT

public:
    // Constructor
    explicit SerebiiClient(QObject* parent = nullptr);

    // Starts search using the Pokemon name from MainWindow
    void searchPokemon(const QString& pokemonName);

signals:
    // Sends formatted Pokemon text back to MainWindow
    void pokemonFound(const QString& formattedInformation);

    // Sends downloaded Pokemon image back to MainWindow
    void pokemonImageReady(const QPixmap& image);

    // Sends error message back to MainWindow
    void requestFailed(const QString& errorMessage);

private:
    // Handles downloaded Serebii webpage
    void processReply(QNetworkReply* reply);

    // Starts download for Pokemon image
    void downloadPokemonImage(const QUrl& imageUrl);

    // Handles downloaded image data
    void processImageReply(QNetworkReply* reply);

    // Object used to send all webpage and image requests
    QNetworkAccessManager* networkManager_;
};