#pragma once

#include <QWidget>

// Forward declarations
class QLabel;
class QLineEdit;
class QPushButton;
class QTextEdit;
class SerebiiClient;

class MainWindow : public QWidget
{
public:
    // Constructor
    explicit MainWindow(QWidget* parent = nullptr);

private:
    // Text box where user enters Pokemon name
    QLineEdit* inputTextBox_;

    // Button that starts search
    QPushButton* searchButton_;

    // Displays downloaded Pokemon image
    QLabel* pokemonImageLabel_;

    // Displays Pokemon information and errors
    QTextEdit* outputTextBox_;

    // Handles website requests and parsing
    SerebiiClient* serebiiClient_;
};