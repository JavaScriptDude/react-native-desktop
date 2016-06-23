
/**
 * Copyright (C) 2016, Canonical Ltd.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#include <QUrl>
#include <QGuiApplication>
#include <QCommandLineParser>
#include <QQuickView>

#include "reactattachedproperties.h"
#include "reactflexlayout.h"
#include "reacttextproperties.h"
#include "reactrawtextproperties.h"
#include "reactitem.h"
#include "reactview.h"


// TODO: some way to change while running
class ReactNativeProperties : public QObject {
  Q_OBJECT
  Q_PROPERTY(bool liveReload READ liveReload WRITE setLiveReload NOTIFY liveReloadChanged)
  Q_PROPERTY(QUrl codeLocation READ codeLocation WRITE setCodeLocation NOTIFY codeLocationChanged)
  Q_PROPERTY(QString pluginsPath READ pluginsPath WRITE setPluginsPath NOTIFY pluginsPathChanged)
public:
  ReactNativeProperties(QObject* parent = 0): QObject(parent) {
    m_codeLocation = m_packagerTemplate.arg(m_packagerHost).arg(m_packagerPort);
  }
  bool liveReload() const {
    return m_liveReload;
  }
  void setLiveReload(bool liveReload) {
    if (m_liveReload == liveReload)
      return;
    m_liveReload = liveReload;
    Q_EMIT liveReloadChanged();
  }
  QUrl codeLocation() const {
    return m_codeLocation;
  }
  void setCodeLocation(const QUrl& codeLocation) {
    if (m_codeLocation == codeLocation)
      return;
    m_codeLocation = codeLocation;
    Q_EMIT codeLocationChanged();
  }
  QString pluginsPath() const {
    return m_pluginsPath;
  }
  void setPluginsPath(const QString& pluginsPath) {
    if (m_pluginsPath == pluginsPath)
      return;
    m_pluginsPath = pluginsPath;
    Q_EMIT pluginsPathChanged();
  }
  QString packagerHost() const {
    return m_packagerHost;
  }
  void setPackagerHost(const QString& packagerHost) {
    if (m_packagerHost == packagerHost)
      return;
    m_packagerHost = packagerHost;
    setCodeLocation(m_packagerTemplate.arg(m_packagerHost).arg(m_packagerPort));
  }
  QString packagerPort() const {
    return m_packagerPort;
  }
  void setPackagerPort(const QString& packagerPort) {
    if (m_packagerPort == packagerPort)
      return;
    m_packagerPort = packagerPort;
    setCodeLocation(m_packagerTemplate.arg(m_packagerHost).arg(m_packagerPort));
  }
  void setLocalSource(const QString& source) {
    if (m_localSource == source)
      return;

    // overrides packager*
    if (source.startsWith("file:")) {
      setCodeLocation(source);
    } else {
      QFileInfo fi(source);
      if (!fi.exists()) {
        qWarning() << "Attempt to set non-existent local source file";
        return;
      }
      setCodeLocation(QUrl::fromLocalFile(fi.absoluteFilePath()));
      setLiveReload(false);
    }
  }
Q_SIGNALS:
  void liveReloadChanged();
  void codeLocationChanged();
  void pluginsPathChanged();
private:
  bool m_liveReload = false;
  QString m_packagerHost = "localhost";
  QString m_packagerPort = "8081";
  QString m_localSource;
  QString m_packagerTemplate = "http://%1:%2/index.ubuntu.bundle?platform=ubuntu&dev=true";
  QUrl m_codeLocation;
  QString m_pluginsPath;
};

void registerTypes()
{
  qmlRegisterUncreatableType<ReactAttachedProperties>("React", 0, 1, "React", "React is not meant to be created directly");
  qmlRegisterUncreatableType<ReactFlexLayout>("React", 0, 1, "Flex", "Flex is not meant to be created directly");
  qmlRegisterUncreatableType<ReactTextProperties>("React", 0, 1, "Text", "Text is not meant to be created directly");
  qmlRegisterUncreatableType<ReactRawTextProperties>("React", 0, 1, "RawText", "Text is not meant to be created directly");
  qmlRegisterType<ReactItem>("React", 0, 1, "Item");
  qmlRegisterType<ReactView>("React", 0, 1, "RootView");
}

int main(int argc, char** argv)
{
  QGuiApplication app(argc, argv);
  QQuickView view;
  ReactNativeProperties* rnp = new ReactNativeProperties(&view);

  registerTypes();

  QCommandLineParser p;
  p.setApplicationDescription("React Native host application");
  p.addHelpOption();
  p.addOptions({
    {{"R", "live-reload"}, "Enable live reload."},
    {{"H", "host"}, "Set packager host address.", rnp->packagerHost()},
    {{"P", "port"}, "Set packager port number.", rnp->packagerPort()},
    {{"L", "local"}, "Set path to the local packaged source", "not set"},
    {{"M", "plugins-path"}, "Set path to node modules", "./plugins"},
  });
  p.process(app);
  rnp->setLiveReload(p.isSet("live-reload"));
  if (p.isSet("host"))
    rnp->setPackagerHost(p.value("host"));
  if (p.isSet("port"))
    rnp->setPackagerPort(p.value("port"));
  if (p.isSet("local"))
    rnp->setLocalSource(p.value("local"));
  if (p.isSet("plugins-path"))
    rnp->setPluginsPath(p.value("plugins-path"));

  view.rootContext()->setContextProperty("ReactNativeProperties", rnp);
  view.setSource(QUrl("qrc:///main.qml"));
  view.setResizeMode(QQuickView::SizeRootObjectToView);
  view.show();

  return app.exec();
}

#include "main.moc"

