#pragma once

#include <QWidget>

class QLineEdit;
class QPushButton;
class QTextEdit;
class SerebiiClient;

class MainWindow : public QWidget
{
public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    QLineEdit* inputTextBox_;
    QPushButton* searchButton_;
    QTextEdit* outputTextBox_;
    SerebiiClient* serebiiClient_;
};