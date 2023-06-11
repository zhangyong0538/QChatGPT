#include <QApplication>
#include <QSslSocket>
#include "ChatGPTClient.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QThread>
#include "ChatData.h"
#include <QTranslator>
int main(int argc, char* argv[]) 
{
    QApplication app(argc, argv);

    QTranslator translator;//支持英文，support the English
    translator.load("./chatworkstation_en.qm");
    app.installTranslator(&translator);

    ChatGPTClient client; 
    client.show();

    return app.exec();
}