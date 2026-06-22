#include "SerebiiClient.h"
// Handles sending network requests
#include <QNetworkAccessManager>
// Represents the response received from website
#include <QNetworkReply>
// Stores information about the request we are sending
#include <QNetworkRequest>
// Used to search through HTML using patterns
#include <QRegularExpression>
// List of QString objects, used to store Pokemon types
#include <QStringList>
// Stores and validates website URLs
#include <QUrl>


// Constructor for SerebiiClient
// parent lets Qt manage ownership and cleanup of this object
SerebiiClient::SerebiiClient(QObject* parent)
    : QObject(parent),

    // Create network manager
    // passing "this" makes SerebiiClient its parent
    // Qt will automatically delete it when SerebiiClient is deleted
    networkManager_(new QNetworkAccessManager(this))
{
}


// Called when user wants to search for a Pokemon
void SerebiiClient::searchPokemon(const QString& pokemonName)
{
    // Remove spaces from beginning/end and make name lowercase
    // Example: "  Pikachu  " becomes "pikachu"
    QString normalizedName = pokemonName.trimmed().toLower();

    // Do not send a request if textbox is empty
    if (normalizedName.isEmpty())
    {
        // Signal MainWindow that request could not be made
        emit requestFailed("Enter a Pokémon name.");
        return;
    }

    // Replace spaces with hyphens for URL
    // Example: "mr mime" becomes "mr-mime"
    normalizedName.replace(' ', '-');

    // Build URL using Pokemon name entered by user
    const QUrl url(
        "https://www.serebii.net/pokemon/" + normalizedName
    );

    // Create request object for this URL
    QNetworkRequest request(url);

    // Tell website what application is making request
    // Better than pretending our program is a web browser
    request.setRawHeader(
        "User-Agent",
        "PangoPoke/1.0 educational desktop application"
    );

    // Send GET request to website
    // GET means we are asking server to give us information
    // reply will contain result after website responds
    QNetworkReply* reply = networkManager_->get(request);

    // Network requests do not finish immediately
    // This waits for Qt to emit the finished signal
    connect(
        reply,
        &QNetworkReply::finished,
        this,

        // Lambda captures this SerebiiClient and reply pointer
        [this, reply]()
        {
            // Process downloaded information once request finishes
            processReply(reply);
        }
    );
}


// Handles response returned from Serebii
void SerebiiClient::processReply(QNetworkReply* reply)
{
    // Check if request failed
    // Could fail because no internet, bad URL, server problem, etc.
    if (reply->error() != QNetworkReply::NoError)
    {
        // Send readable error message back to MainWindow
        emit requestFailed(
            "Website request failed: " + reply->errorString()
        );

        // Network reply should not be deleted immediately
        // deleteLater safely deletes it when Qt event loop is ready
        reply->deleteLater();
        return;
    }

    // Read all downloaded bytes from response
    // Convert UTF-8 website data into QString
    const QString html =
        QString::fromUtf8(reply->readAll());

    // DEBUG ONLY:
    // Displays first 10,000 characters of raw HTML
    // Leave commented out when using formatted output
    // Otherwise pokemonFound gets emitted twice
    // emit pokemonFound(html.left(10000));


    // Regular expression pattern used to find Pokemon name
    // Searches inside website's <title> HTML tag
    const QRegularExpression titleExpression(
        R"(<title[^>]*>\s*([^-<]+?)\s*-\s*#)",

        // Ignore uppercase/lowercase differences
        QRegularExpression::CaseInsensitiveOption
    );

    // Search HTML using title pattern
    const QRegularExpressionMatch titleMatch =
        titleExpression.match(html);

    // If pattern does not match, Pokemon name could not be parsed
    if (!titleMatch.hasMatch())
    {
        emit requestFailed("Could not find the Pokémon name.");

        reply->deleteLater();
        return;
    }

    // captured(1) gets text inside first parentheses in regex
    // trim extra spaces from captured name
    QString pokemonName =
        titleMatch.captured(1).trimmed();


    // Pattern used to find links containing Pokemon types
    // ([a-z]+) captures type name from URL
    const QRegularExpression typeExpression(
        R"(href\s*=\s*["'][^"']*/type/([a-z]+)[^"']*["'])",

        // Allows Electric, electric, ELECTRIC, etc.
        QRegularExpression::CaseInsensitiveOption
    );

    // globalMatch searches for every matching type link
    // not just the first one
    QRegularExpressionMatchIterator typeMatches =
        typeExpression.globalMatch(html);

    // Empty list where found types will be stored
    QStringList types;

    // Continue while another matching type exists
    while (typeMatches.hasNext())
    {
        // Get next regex match
        const QRegularExpressionMatch match =
            typeMatches.next();

        // captured(1) contains type name from ([a-z]+)
        QString type = match.captured(1).trimmed();

        // Ignore empty matches
        if (type.isEmpty())
        {
            continue;
        }

        // Capitalize first letter
        // Example: "electric" becomes "Electric"
        type[0] = type[0].toUpper();

        // Only add type if it is not already in list
        // CaseInsensitive means "Fire" and "fire" count as same type
        if (!types.contains(type, Qt::CaseInsensitive))
        {
            types.append(type);
        }

        // Pokemon can only have one or two types
        // Stop searching once two unique types are found
        if (types.size() == 2)
        {
            break;
        }
    }

    // No types were found in HTML
    if (types.isEmpty())
    {
        emit requestFailed(
            "The page was downloaded, but no Pokémon types were found."
        );

        reply->deleteLater();
        return;
    }


    // Build final formatted text
    // %1 replaced with Pokemon name
    // %2 replaced with type list
    const QString formattedInformation =
        QString(
            "Pokemon: %1\n"
            "Type: %2"
        )
        .arg(pokemonName)

        // Join types together with comma
        // Example: ["Fire", "Flying"] becomes "Fire, Flying"
        .arg(types.join(", "));

    // Send completed information back to MainWindow
    emit pokemonFound(formattedInformation);

    // Schedule reply object to be safely deleted
    // prevents memory leak after request is finished
    reply->deleteLater();
}
```
