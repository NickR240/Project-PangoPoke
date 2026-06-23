#include "SerebiiClient.h"

#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPixmap>
#include <QString>
#include <QStringList>
#include <QUrl>


SerebiiClient::SerebiiClient(QObject* parent)
    : QObject(parent)
{
    // Create one network manager for this object.
    // "this" makes SerebiiClient its parent.
    networkManager_ = new QNetworkAccessManager(this);
}


void SerebiiClient::searchPokemon(const QString& pokemonName)
{
    // Make a copy because we need to change the text.
    QString normalizedName = pokemonName;

    // Remove spaces from beginning and end.
    normalizedName = normalizedName.trimmed();

    // Convert text to lowercase.
    normalizedName = normalizedName.toLower();

    // Stop if user entered nothing.
    if (normalizedName.isEmpty())
    {
        emit requestFailed("Enter a Pokémon name.");
        return;
    }

    // Replace spaces with hyphens for the URL.
    // Example: "mr mime" becomes "mr-mime".
    normalizedName.replace(" ", "-");

    // Start with the base Serebii address.
    QString urlText = "https://www.serebii.net/pokemon/";

    // Add the entered Pokemon name.
    urlText = urlText + normalizedName;

    // Convert the QString into a QUrl.
    QUrl url(urlText);

    // Create the website request.
    QNetworkRequest request(url);

    // Tell the website which program sent the request.
    request.setRawHeader(
        "User-Agent",
        "PangoPoke/1.0 educational desktop application"
    );

    // Send the GET request.
    QNetworkReply* reply = networkManager_->get(request);

    // When the request finishes, process its response.
    connect(
        reply,
        &QNetworkReply::finished,
        this,
        [this, reply]()
        {
            processReply(reply);
        }
    );
}


void SerebiiClient::processReply(QNetworkReply* reply)
{
    // Check if the website request failed.
    if (reply->error() != QNetworkReply::NoError)
    {
        QString errorMessage = "Website request failed: ";
        errorMessage = errorMessage + reply->errorString();

        emit requestFailed(errorMessage);

        reply->deleteLater();
        return;
    }

    // Downloaded webpage data arrives as bytes.
    QByteArray websiteData = reply->readAll();

    // Convert the bytes into normal text.
    QString html = QString::fromUtf8(websiteData);


    // =========================================================
    // FIND THE POKEMON NAME
    // =========================================================

    // Look for the opening title tag.
    int titleStart = html.indexOf(
        "<title>",
        0,
        Qt::CaseInsensitive
    );

    if (titleStart == -1)
    {
        emit requestFailed("Could not find the page title.");

        reply->deleteLater();
        return;
    }

    // Move past the text "<title>".
    titleStart = titleStart + 7;

    // Serebii titles normally place " - #" after the Pokemon name.
    int titleEnd = html.indexOf(
        " - #",
        titleStart,
        Qt::CaseInsensitive
    );

    if (titleEnd == -1)
    {
        emit requestFailed("Could not find the Pokémon name.");

        reply->deleteLater();
        return;
    }

    // Get the text between <title> and " - #".
    int nameLength = titleEnd - titleStart;

    QString pokemonName = html.mid(
        titleStart,
        nameLength
    );

    pokemonName = pokemonName.trimmed();


    // =========================================================
    // FIND THE POKEMON TYPES
    // =========================================================

    QStringList types;

    // Start searching from the beginning of the page.
    int searchPosition = 0;

    // Continue until two types are found.
    while (types.size() < 2)
    {
        // Search for a type URL.
        int typeStart = html.indexOf(
            "/type/",
            searchPosition,
            Qt::CaseInsensitive
        );

        // -1 means no more type links were found.
        if (typeStart == -1)
        {
            break;
        }

        // Move past "/type/".
        typeStart = typeStart + 6;

        // Find the end of the type name.
        int typeEnd = html.indexOf(
            "\"",
            typeStart
        );

        if (typeEnd == -1)
        {
            break;
        }

        // Get the type text.
        int typeLength = typeEnd - typeStart;

        QString type = html.mid(
            typeStart,
            typeLength
        );

        type = type.trimmed();

        // Sometimes the URL may contain more path information.
        // Only keep text before another slash.
        int slashPosition = type.indexOf("/");

        if (slashPosition != -1)
        {
            type = type.left(slashPosition);
        }

        // Remove possible HTML or URL characters.
        int questionMarkPosition = type.indexOf("?");

        if (questionMarkPosition != -1)
        {
            type = type.left(questionMarkPosition);
        }

        if (!type.isEmpty())
        {
            // Make the first letter uppercase.
            QString firstLetter = type.left(1);
            firstLetter = firstLetter.toUpper();

            QString remainingLetters = type.mid(1);
            remainingLetters = remainingLetters.toLower();

            type = firstLetter + remainingLetters;

            // Do not add the same type twice.
            if (!types.contains(type, Qt::CaseInsensitive))
            {
                types.append(type);
            }
        }

        // Continue searching after the current type.
        searchPosition = typeEnd + 1;
    }

    if (types.isEmpty())
    {
        emit requestFailed(
            "The page was downloaded, but no Pokémon types were found."
        );

        reply->deleteLater();
        return;
    }


    // =========================================================
    // FORMAT THE TEXT
    // =========================================================

    QString formattedInformation;

    formattedInformation = "Pokemon: ";
    formattedInformation = formattedInformation + pokemonName;

    formattedInformation = formattedInformation + "\nType: ";
    formattedInformation = formattedInformation + types.join(", ");

    // Send formatted text back to MainWindow.
    emit pokemonFound(formattedInformation);


    // =========================================================
    // FIND THE NORMAL SPRITE IMAGE
    // =========================================================

    // Search for the Normal Sprite alt text.
    int altPosition = html.indexOf(
        "alt=\"Normal Sprite\"",
        0,
        Qt::CaseInsensitive
    );

    if (altPosition == -1)
    {
        // Text information can still work without an image.
        reply->deleteLater();
        return;
    }

    // Search backward from alt text to find the beginning
    // of the image element.
    int imageTagStart = html.lastIndexOf(
        "<img",
        altPosition,
        Qt::CaseInsensitive
    );

    if (imageTagStart == -1)
    {
        reply->deleteLater();
        return;
    }

    // Find src=" inside this image element.
    int sourceStart = html.indexOf(
        "src=\"",
        imageTagStart,
        Qt::CaseInsensitive
    );

    if (sourceStart == -1 || sourceStart > altPosition)
    {
        reply->deleteLater();
        return;
    }

    // Move past src=".
    sourceStart = sourceStart + 5;

    // Find the closing quote after the image path.
    int sourceEnd = html.indexOf(
        "\"",
        sourceStart
    );

    if (sourceEnd == -1)
    {
        reply->deleteLater();
        return;
    }

    // Extract the image path.
    int sourceLength = sourceEnd - sourceStart;

    QString imagePath = html.mid(
        sourceStart,
        sourceLength
    );

    imagePath = imagePath.trimmed();

    // Convert the relative image path into a full URL.
    QUrl relativeImageUrl(imagePath);

    QUrl completeImageUrl =
        reply->url().resolved(relativeImageUrl);

    // Begin downloading the sprite.
    downloadPokemonImage(completeImageUrl);

    // Finished with the webpage response.
    reply->deleteLater();
}


void SerebiiClient::downloadPokemonImage(const QUrl& imageUrl)
{
    // Create a request for the image.
    QNetworkRequest request(imageUrl);

    request.setRawHeader(
        "User-Agent",
        "PangoPoke/1.0 educational desktop application"
    );

    // Send the image request.
    QNetworkReply* reply = networkManager_->get(request);

    // Process it after downloading finishes.
    connect(
        reply,
        &QNetworkReply::finished,
        this,
        [this, reply]()
        {
            processImageReply(reply);
        }
    );
}


void SerebiiClient::processImageReply(QNetworkReply* reply)
{
    // Check if the image request failed.
    if (reply->error() != QNetworkReply::NoError)
    {
        QString errorMessage = "Image request failed: ";
        errorMessage = errorMessage + reply->errorString();

        emit requestFailed(errorMessage);

        reply->deleteLater();
        return;
    }

    // Read the downloaded image bytes.
    QByteArray imageData = reply->readAll();

    // Create an empty image object.
    QPixmap pokemonImage;

    // Try loading the downloaded bytes as an image.
    bool imageLoaded = pokemonImage.loadFromData(imageData);

    if (imageLoaded == false)
    {
        emit requestFailed(
            "The image downloaded, but Qt could not open it."
        );

        reply->deleteLater();
        return;
    }

    // Send the image to MainWindow.
    emit pokemonImageReady(pokemonImage);

    reply->deleteLater();
}