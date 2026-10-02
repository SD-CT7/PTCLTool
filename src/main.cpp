#include "editor/mainWindow.h"

#include <QApplication>
#include <QtGlobal>


int main(int argc, char* argv[]) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    // Qt5 needs high-DPI scaling enabled explicitly (always on in Qt6).
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName("Explos");
    QCoreApplication::setApplicationName("PTCLEditor");

    PtclEditor::MainWindow win;
    win.show();

    return app.exec();
}
