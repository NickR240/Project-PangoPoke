#pragma once

#include <QObject>
#include <QString>

class QNetworkAccessManager;
class QNetworkReply;

class SerebiiClient : public QObject
{
    Q_OBJECT

public:
    explicit SerebiiClient(QObject* parent = nullptr);

    void searchPokemon(const QString& pokemonName);

signals:
    void pokemonFound(const QString& formattedInformation);
    void requestFailed(const QString& errorMessage);

private:
    void processReply(QNetworkReply* reply);

    QNetworkAccessManager* networkManager_;
};