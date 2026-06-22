#include "MainWindow.h"
#include "SerebiiClient.h"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget* parent)
    : QWidget(parent),
    inputTextBox_(new QLineEdit(this)),
    searchButton_(new QPushButton("Search", this)),
    outputTextBox_(new QTextEdit(this)),
    serebiiClient_(new SerebiiClient(this))
{
    setWindowTitle("PangoPoke");
    resize(600, 400);

    inputTextBox_->setPlaceholderText(
        "Enter a Pokémon name..."
    );

    outputTextBox_->setReadOnly(true);

    auto* searchLayout = new QHBoxLayout;
    searchLayout->addWidget(inputTextBox_);
    searchLayout->addWidget(searchButton_);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(searchLayout);
    mainLayout->addWidget(outputTextBox_);

    connect(
        searchButton_,
        &QPushButton::clicked,
        this,
        [this]()
        {
            outputTextBox_->setPlainText("Loading...");

            serebiiClient_->searchPokemon(
                inputTextBox_->text()
            );
        }
    );

    connect(
        inputTextBox_,
        &QLineEdit::returnPressed,
        searchButton_,
        &QPushButton::click
    );

    connect(
        serebiiClient_,
        &SerebiiClient::pokemonFound,
        this,
        [this](const QString& information)
        {
            outputTextBox_->setPlainText(information);
        }
    );

    connect(
        serebiiClient_,
        &SerebiiClient::requestFailed,
        this,
        [this](const QString& errorMessage)
        {
            outputTextBox_->setPlainText(errorMessage);
        }
    );
}