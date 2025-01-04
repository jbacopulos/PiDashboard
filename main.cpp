#include "MainWindow.h"

#include <QApplication>
#include <QTimer>
#include <QFile>

bool mapHasAllEntries(const QStringList &list, const std::map<QString, QString> &map) {
    for (const QString &key : list) {
        if (map.find(key) == map.end()) {
            return false;
        }
    }
    return true;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    const QStringList REQUIRED_ENV_VARS = {"ZIP", "UNIT", "OW_API_KEY", "W_API_KEY"};

    if (QCoreApplication::arguments().size() != 2) {
        qCritical() << "Env file must be specified!";
        return EXIT_FAILURE;
    }

    QFile envFile(QCoreApplication::arguments().at(1));
    if(!envFile.open(QIODevice::ReadOnly)) {
        qCritical() << envFile.errorString();
        return EXIT_FAILURE;
    }

    QTextStream in(&envFile);
    std::map<QString, QString> envVars;

    while(!in.atEnd()) {
        QString line = in.readLine();
        if (line.isEmpty()) continue;

        line.remove(" ");
        line.remove("\"");
        line.remove("'");

        QStringList list = line.split("=");

        if (REQUIRED_ENV_VARS.contains(list[0])) {
            envVars[list[0]] = list[1];
        }
    }

    envFile.close();

    if (!mapHasAllEntries(REQUIRED_ENV_VARS, envVars)) {
        qCritical() << "Missing required env variables!";
        return EXIT_FAILURE;
    }

    MainWindow w(envVars);
    w.show();

    return a.exec();
}
