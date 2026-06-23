#define QT_NO_DEPRECATED_WARNINGS

#include <QApplication>
#include "MainWindow.h"

int main(int argc, char* argv[])
{
    // Creates the Qt application object.
    // argc and argv allow Qt to process command-line options.
    QApplication app(argc, argv);

    // Create the main program window.
    MainWindow window;

    // Make the window visible.
    window.show();

    // Start Qt's event loop.
    // Keeps the window open and handles clicks, typing, signals, etc.
    return app.exec();
}