#include "pientertain.hh"

#include <QCoreApplication>
#include <iostream>

int main(int argc, char *argv[]) {

  QCoreApplication::setOrganizationName("appkellner");
  QCoreApplication::setOrganizationDomain("www.appkellner.de");
  QCoreApplication::setApplicationName("pientertain");

  QCoreApplication app(argc, argv);

  qRegisterMetaType< QVector<QColor> >();
  qRegisterMetaType< QVector<QPoint> >();

  PiEntertain controller;

  if (!controller.isSetup()) {
    QSettings settings(QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName());
    QString fname = settings.fileName();
    std::cerr << "please put your hue bridge IP in the configuration file\n"
              << fname.toStdString() << std::endl;
    return 0;
  }

  controller.startServer();

  return app.exec();
}
