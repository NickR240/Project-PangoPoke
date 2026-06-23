#include "MainWindow.h"
#include "SerebiiClient.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>


MainWindow::MainWindow(QWidget* parent)
    : QWidget(parent)
{
    // Create each GUI object one at a time.
    inputTextBox_ = new QLineEdit(this);

    searchButton_ = new QPushButton(
        "Search",
        this
    );

    pokemonImageLabel_ = new QLabel(this);

    outputTextBox_ = new QTextEdit(this);

    serebiiClient_ = new SerebiiClient(this);


    // Set basic window settings.
    setWindowTitle("PangoPoke");

    resize(600, 650);


    // Setup search input.
    inputTextBox_->setPlaceholderText(
        "Enter a Pokémon name..."
    );


    // Make output text read-only.
    outputTextBox_->setReadOnly(true);


    // Setup image area.
    pokemonImageLabel_->setAlignment(
        Qt::AlignCenter
    );

    pokemonImageLabel_->setMinimumSize(
        250,
        250
    );

    pokemonImageLabel_->setText(
        "Pokemon image will appear here"
    );


    // Create the top row layout.
    QHBoxLayout* searchLayout =
        new QHBoxLayout();

    searchLayout->addWidget(
        inputTextBox_
    );

    searchLayout->addWidget(
        searchButton_
    );


    // Create the main vertical layout.
    QVBoxLayout* mainLayout =
        new QVBoxLayout(this);

    // Add the search row first.
    mainLayout->addLayout(
        searchLayout
    );

    // Add the image below the search row.
    mainLayout->addWidget(
        pokemonImageLabel_
    );

    // Add text output under the image.
    mainLayout->addWidget(
        outputTextBox_
    );


    // Connect Search button to search code.
    connect(
        searchButton_,
        &QPushButton::clicked,
        this,
        [this]()
        {
            // Show that the search has started.
            outputTextBox_->setPlainText(
                "Loading..."
            );

            // Remove the previous image.
            pokemonImageLabel_->clear();

            pokemonImageLabel_->setText(
                "Loading image..."
            );

            // Get the name entered in the textbox.
            QString enteredName =
                inputTextBox_->text();

            // Ask SerebiiClient to search for it.
            serebiiClient_->searchPokemon(
                enteredName
            );
        }
    );


    // Pressing Enter clicks the Search button.
    connect(
        inputTextBox_,
        &QLineEdit::returnPressed,
        searchButton_,
        &QPushButton::click
    );


    // Receive formatted Pokemon text.
    connect(
        serebiiClient_,
        &SerebiiClient::pokemonFound,
        this,
        [this](const QString& information)
        {
            outputTextBox_->setPlainText(
                information
            );
        }
    );


    // Receive downloaded Pokemon image.
    connect(
        serebiiClient_,
        &SerebiiClient::pokemonImageReady,
        this,
        [this](const QPixmap& image)
        {
            // Get the current label width.
            int labelWidth =
                pokemonImageLabel_->width();

            // Get the current label height.
            int labelHeight =
                pokemonImageLabel_->height();

            // Scale image while keeping its original shape.
            QPixmap scaledImage = image.scaled(
                labelWidth,
                labelHeight,
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation
            );

            // Display the scaled image.
            pokemonImageLabel_->setPixmap(
                scaledImage
            );
        }
    );


    // Receive errors from SerebiiClient.
    connect(
        serebiiClient_,
        &SerebiiClient::requestFailed,
        this,
        [this](const QString& errorMessage)
        {
            outputTextBox_->setPlainText(
                errorMessage
            );
        }
    );
}