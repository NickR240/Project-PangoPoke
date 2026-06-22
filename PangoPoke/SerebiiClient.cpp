#include "SerebiiClient.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QStringList>
#include <QUrl>

SerebiiClient::SerebiiClient(QObject* parent)
    : QObject(parent),
    networkManager_(new QNetworkAccessManager(this))
{
}

void SerebiiClient::searchPokemon(const QString& pokemonName)
{
    QString normalizedName = pokemonName.trimmed().toLower();

    if (normalizedName.isEmpty())
    {
        emit requestFailed("Enter a Pokémon name.");
        return;
    }

    // Basic handling for names containing spaces.
    normalizedName.replace(' ', '-');

    const QUrl url(
        "https://www.serebii.net/pokemon/" + normalizedName
    );

    QNetworkRequest request(url);

    // Identify the application rather than pretending to be a browser.
    request.setRawHeader(
        "User-Agent",
        "PangoPoke/1.0 educational desktop application"
    );

    QNetworkReply* reply = networkManager_->get(request);

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
    if (reply->error() != QNetworkReply::NoError)
    {
        emit requestFailed(
            "Website request failed: " + reply->errorString()
        );

        reply->deleteLater();
        return;
    }

    const QString html =
        QString::fromUtf8(reply->readAll());
    emit pokemonFound(html.left(10000));

    // Extract the Pokémon name from the page title.
    const QRegularExpression titleExpression(
        R"(<title[^>]*>\s*([^-<]+?)\s*-\s*#)",
        QRegularExpression::CaseInsensitiveOption
    );

    const QRegularExpressionMatch titleMatch =
        titleExpression.match(html);

    if (!titleMatch.hasMatch())
    {
        emit requestFailed("Could not find the Pokémon name.");
        reply->deleteLater();
        return;
    }

    QString pokemonName =
        titleMatch.captured(1).trimmed();

    // Search for Serebii type links.
    const QRegularExpression typeExpression(
        R"(href\s*=\s*["'][^"']*/type/([a-z]+)[^"']*["'])",
        QRegularExpression::CaseInsensitiveOption
    );

    QRegularExpressionMatchIterator typeMatches =
        typeExpression.globalMatch(html);

    QStringList types;

    while (typeMatches.hasNext())
    {
        const QRegularExpressionMatch match =
            typeMatches.next();

        QString type = match.captured(1).trimmed();

        if (type.isEmpty())
        {
            continue;
        }

        type[0] = type[0].toUpper();

        if (!types.contains(type, Qt::CaseInsensitive))
        {
            types.append(type);
        }

        // A Pokémon can have at most two types.
        if (types.size() == 2)
        {
            break;
        }
    }

    if (types.isEmpty())
    {
        emit requestFailed(
            "The page was downloaded, but no Pokémon types were found."
        );

        reply->deleteLater();
        return;
    }

    const QString formattedInformation =
        QString(
            "Pokemon: %1\n"
            "Type: %2"
        )
        .arg(pokemonName)
        .arg(types.join(", "));

    emit pokemonFound(formattedInformation);

    reply->deleteLater();
}