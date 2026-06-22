#include "MainWindow.h"
#include "SerebiiClient.h"
// Places widgets next to each other horizontally
#include <QHBoxLayout>
// Single-line text input box
#include <QLineEdit>
// Clickable button
#include <QPushButton>
// Larger multi-line text box
#include <QTextEdit>
// Places widgets above and below each other vertically
#include <QVBoxLayout>


// Constructor for MainWindow
// parent allows another Qt object to own this window if needed
MainWindow::MainWindow(QWidget* parent)
    : QWidget(parent),

    // Create top input box
    // passing "this" makes MainWindow its parent
    inputTextBox_(new QLineEdit(this)),

    // Create Search button
    searchButton_(new QPushButton("Search", this)),

    // Create large output text box
    outputTextBox_(new QTextEdit(this)),

    // Create object that handles website requests and HTML parsing
    // MainWindow owns it and Qt deletes it automatically
    serebiiClient_(new SerebiiClient(this))
{
    // Text displayed in title bar
    setWindowTitle("PangoPoke");

    // Starting size of window in pixels
    // width = 600, height = 400
    resize(600, 400);


    // Gray hint text shown before user types anything
    inputTextBox_->setPlaceholderText(
        "Enter a Pokémon name..."
    );

    // User can select/copy output but cannot edit it
    outputTextBox_->setReadOnly(true);


    // Create horizontal layout for textbox and Search button
    // No parent passed here because it will be owned by mainLayout
    auto* searchLayout = new QHBoxLayout;

    // Add input textbox to left side
    searchLayout->addWidget(inputTextBox_);

    // Add Search button to right side
    searchLayout->addWidget(searchButton_);


    // Create main vertical layout
    // Passing "this" installs layout on MainWindow
    auto* mainLayout = new QVBoxLayout(this);

    // Add top horizontal row
    mainLayout->addLayout(searchLayout);

    // Add large output textbox underneath search row
    mainLayout->addWidget(outputTextBox_);


    // Connect Search button click to code inside lambda
    connect(
        // Object sending signal
        searchButton_,

        // Signal sent when button is clicked
        &QPushButton::clicked,

        // Object receiving connection
        this,

        // Code that runs when button is clicked
        // [this] lets lambda access MainWindow members
        [this]()
        {
            // Show feedback while waiting for website response
            outputTextBox_->setPlainText("Loading...");

            // Ask SerebiiClient to search using text entered by user
            serebiiClient_->searchPokemon(
                inputTextBox_->text()
            );
        }
    );


    // Makes pressing Enter behave the same as clicking Search
    connect(
        // Input box sends signal
        inputTextBox_,

        // Signal sent when Enter is pressed
        &QLineEdit::returnPressed,

        // Search button receives signal
        searchButton_,

        // Simulate clicking the Search button
        &QPushButton::click
    );


    // Runs when SerebiiClient successfully finds Pokemon information
    connect(
        // Object sending signal
        serebiiClient_,

        // Custom success signal from SerebiiClient
        &SerebiiClient::pokemonFound,

        // MainWindow receives signal
        this,

        // information contains formatted Pokemon result
        [this](const QString& information)
        {
            // Replace current textbox contents with result
            outputTextBox_->setPlainText(information);
        }
    );


    // Runs when SerebiiClient reports a request or parsing error
    connect(
        // Object sending signal
        serebiiClient_,

        // Custom failure signal from SerebiiClient
        &SerebiiClient::requestFailed,

        // MainWindow receives signal
        this,

        // errorMessage contains readable error text
        [this](const QString& errorMessage)
        {
            // Display error inside large output textbox
            outputTextBox_->setPlainText(errorMessage);
        }
    );
}