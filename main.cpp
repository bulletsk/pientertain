#include <QCoreApplication>
#include "pientertain.hh"

int main(int argc, char *argv[]) {
  QCoreApplication::setOrganizationName("appkellner");
  QCoreApplication::setOrganizationDomain("www.appkellner.de");
  QCoreApplication::setApplicationName("pientertain");

  QCoreApplication app(argc, argv);

  qRegisterMetaType< QVector<QColor> >();
  qRegisterMetaType< QVector<QPoint> >();

  PiEntertain controller;
  controller.startServer();

  return app.exec();
}
