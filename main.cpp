#include <iostream>
#include "hueauthentication.hh"
#include "huestream.hh"
#include "lightpacket.hh"
#include "videosource.hh"
#include "restserver.hh"
#include "pientertain.hh"

#include <QCoreApplication>


int main(int argc, char *argv[]) {
  std::cout << "hello pi" << std::endl;

  QCoreApplication::setOrganizationName("appkellner");
  QCoreApplication::setOrganizationDomain("www.appkellner.de");
  QCoreApplication::setApplicationName("PiEntertain");

  QCoreApplication app(argc, argv);

  qRegisterMetaType< QVector<QColor> >();
  qRegisterMetaType< QVector<QPoint> >();

  PiEntertain controller;
  controller.startServer();

  return app.exec();
}

#include "main.moc"
