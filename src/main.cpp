#include <QApplication>
#include <QDir>
#include <QDesktopServices>
#include <QDate>
#include <QIcon>
#include <QPointer>

#include <QLocalServer>
#include <QLocalSocket>

#include "window.h"
#include "tracker.h"

#include "updater.h"
#ifdef Q_WS_MAC
#include "cocoa_initializer.h"
#include "sparkle_updater.h"
#endif

int main(int argc, char **argv)
{
  const char serverName[] = "trackobot";

  // Enforce single instance
  QLocalSocket socket;
  socket.connectToServer(serverName);
  if(socket.waitForConnected(500)) {
    return 1; // already running
  }

  QLocalServer::removeServer(serverName);
  QLocalServer server(NULL);
  if(!server.listen(serverName)) {
    return 2;
  }

  // Basic setup
  QApplication app(argc, argv);
#ifdef Q_WS_MAC
  QIcon icon = QIcon(":/icons/tray_mac.png");
#else
  QIcon icon = QIcon(":/icons/tray_win.png");
#endif
  app.setApplicationName("Track-o-Bot"); // for proper DataLocation handling
  app.setOrganizationName("spidy.ch");
  app.setOrganizationDomain("spidy.ch");
  app.setWindowIcon(icon);

  // Logging
  QString dataLocation = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
  if(!QFile::exists(dataLocation)) {
    QDir dir;
    dir.mkpath(dataLocation);
  }
  string logFilePath = (dataLocation + QDir::separator() + app.applicationName() + ".log").toStdString();
  Logger::Instance()->SetLogPath(logFilePath);

  // Start
  LOG("--> Launched v%s on %s", VERSION, QDate::currentDate().toString(Qt::ISODate).toStdString().c_str());

  Updater *updater = NULL;

#ifdef Q_WS_MAC
  CocoaInitializer cocoaInitiarizer;
  updater = new SparkleUpdater(Tracker::Instance()->WebserviceURL("/appcast.xml"));
#endif

  // Initalize Windows n stuff
  Window window;

  // Make sure Account exists or create one
  Tracker::Instance()->EnsureAccountIsSetUp();

  // Main Loop
  int exitCode = app.exec();

  // Tear down
  LOG("<-- Shutdown");

  return exitCode;
}
