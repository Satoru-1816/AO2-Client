#include "pathing/aocache.hpp"
#include "options.h"

#include <QDataStream>
#include <QDebug>
#include <QFile>
#include <QDir>

AttorneyOnline::Core::AOCache::AOCache(QObject *parent, const QString f_base_path)
    : QObject(parent), m_base_path(f_base_path)
{
  qInfo() << "[Cache]::CTOR: Starting";

  if (!Options::getInstance().enablePersistentCache()) {
    qInfo() << "[Cache]::CTOR: Cache restore skipped.";
    return;
  }

  m_asset_cache.reserve(2048);
  m_directory_cache.reserve(1024);


  if (!QFile::exists(m_base_path+ "cache/cache.lock")) {
    qInfo() << "[Cache]::CTOR: Creating cache lock.";
    m_primary_client = true;
    QFile l_cache_lock(m_base_path+ "cache/cache.lock");
    l_cache_lock.open(QIODevice::ReadWrite);
    l_cache_lock.flush();
    l_cache_lock.close();
  }

  QString l_directory_path = m_base_path + "cache/dir_cache.ao";
  if (QFile::exists(l_directory_path)) {
    qInfo() << "[Cache]::CTOR: Restoring directory cache.";

    QFile l_directory_file(l_directory_path);
    if (l_directory_file.open(QIODevice::ReadOnly)) {
      QDataStream l_read(&l_directory_file);
      l_read >> m_directory_cache;
      l_directory_file.close();
    }
  }

  QString l_asset_path = m_base_path + "cache/asset_cache.ao";
  if (QFile::exists(l_asset_path)) {
    qInfo() << "[Cache]::CTOR: Restoring asset cache.";

    QFile l_asset_file(l_asset_path);
    if (l_asset_file.open(QIODevice::ReadOnly)) {
      QDataStream l_read(&l_asset_file);
      l_read >> m_asset_cache;
    }
  }
}

AttorneyOnline::Core::AOCache::~AOCache()
{
  if (!Options::getInstance().enablePersistentCache()) {
    QFile::remove(m_base_path + "cache/dir_cache.ao");
    QFile::remove(m_base_path + "cache/asset_cache.ao");
    return;
  }

  if (isPrimaryClient()) {
    if(!QDir().exists(m_base_path + "cache")) {
      QDir().mkdir(m_base_path + "cache");
    }

    QString l_directory_path = m_base_path + "cache/dir_cache.ao";
    qInfo() << "[Cache]::DTOR: Saving directory cache.";

    QFile l_directory_file(l_directory_path);
    l_directory_file.resize(0);
    if (l_directory_file.open(QIODevice::WriteOnly)) {
      QDataStream l_out(&l_directory_file);
      l_out << m_directory_cache;
      l_directory_file.close();
    }

    QString l_asset_path = m_base_path + "cache/asset_cache.ao";
    qInfo() << "[Cache]::DTOR: Saving asset cache.";

    QFile l_asset_file(l_asset_path);
    l_asset_file.resize(0);
    if (l_asset_file.open(QIODevice::WriteOnly)) {
      QDataStream l_out(&l_asset_file);
      l_out << m_asset_cache;
      l_asset_file.close();
    }

    qInfo() << "[Cache]::DTOR: Releasing cache lock.";
    QFile::remove(m_base_path + "cache/cache.lock");
  }
}

void AttorneyOnline::Core::AOCache::populateDirectoryCache(QStringList f_base_mounts)
{
  QStringList l_mountlist;
  l_mountlist << m_base_path << f_base_mounts;

  QStringList l_asset_folder {
      "background",
      "characters",
      "evidence",
      "misc",
      "themes",
      "sounds/blips"
      "sounds/general"
      "sounds/music"
  };

  for (QString &l_mount : l_mountlist) {
    for (QString &l_folder : l_asset_folder) {
      Q_UNUSED(l_mount)
      Q_UNUSED(l_folder)
      //To be added later.
    }
  }
}

QString AttorneyOnline::Core::AOCache::checkDirectoryCache(VPath f_directory)
{
#if DEBUG_CACHE
  QString l_path = m_asset_cache.value(qHash(f_directory));
  if (l_path.isEmpty()) {
    qDebug() << "[Cache]::DirectoryCache: VPath" << f_directory.toQString() << "not found in directory cache.";
  }
  return l_path;
#else
  return m_directory_cache.value(qHash(f_directory));
#endif
}

QString AttorneyOnline::Core::AOCache::checkAssetCache(VPath f_asset)
{
#if DEBUG_CACHE
  QString l_path = m_asset_cache.value(qHash(f_asset));
  if (l_path.isEmpty()) {
    qDebug() << "[Cache]::AssetCache: VPath" + f_asset.toQString() + "not found in asset cache.";
  }
  return l_path;
#else
  return m_asset_cache.value(qHash(f_asset));
#endif
}

void AttorneyOnline::Core::AOCache::insertIntoDirectoryCache(VPath f_directory, QString f_physical_path)
{
#if DEBUG_CACHE
  qDebug() << "[Cache]::DirectoryCache: Inserting VPath" << f_directory.toQString() << "with physical" << f_physical_path;;
#endif
  m_directory_cache.insert(qHash(f_directory), f_physical_path);

}

void AttorneyOnline::Core::AOCache::insertIntoAssetCache(VPath f_asset, QString f_physical_path)
{
#if DEBUG_CACHE
  qDebug() << "[Cache]::AssetCache: Inserting VPath" << f_asset.toQString() << "with physical" << f_physical_path;
#endif
  m_directory_cache.insert(qHash(f_asset), f_physical_path);
}

void AttorneyOnline::Core::AOCache::invalidateCache(QString f_reason)
{
#if DEBUG_CACHE
  qDebug() << "[CACHE]::DELETE: Deleting cache due to" << f_reason;
#else
  Q_UNUSED(f_reason)
#endif
  m_asset_cache.clear();
  m_directory_cache.clear();
}

int AttorneyOnline::Core::AOCache::assetCacheSize()
{
  return m_asset_cache.size();
}

bool AttorneyOnline::Core::AOCache::isPrimaryClient()
{
  return m_primary_client;
}