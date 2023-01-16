#include "aoapplication.h"

#include "courtroom.h"
#include "debug_functions.h"
#include "lobby.h"
#include "networkmanager.h"
#include "options.h"

#include "aocaseannouncerdialog.h"
#include "widgets/aooptionsdialog.h"

static QtMessageHandler original_message_handler;
static AOApplication *message_handler_context;
void message_handler(QtMsgType type, const QMessageLogContext &context,
                     const QString &msg)
{
  emit message_handler_context->qt_log_message(type, context, msg);
  original_message_handler(type, context, msg);
}

AOApplication::AOApplication(int &argc, char **argv) : QApplication(argc, argv)
{
  net_manager = new NetworkManager(this);
  discord = new AttorneyOnline::Discord();

  asset_lookup_cache.reserve(2048);

  message_handler_context = this;
  original_message_handler = qInstallMessageHandler(message_handler);

  setApplicationVersion(get_version_string());
  setApplicationDisplayName(tr("Attorney Online %1").arg(applicationVersion()));
}

AOApplication::~AOApplication()
{
  destruct_lobby();
  destruct_courtroom();
  delete discord;
  qInstallMessageHandler(original_message_handler);
}

void AOApplication::open_lobby()
{
    auto lobby = new Lobby(this, net_manager);
      lobby->setAttribute(Qt::WA_DeleteOnClose);

      if (options.discordEnabled())
        discord->state_lobby();

      if (demo_server)
          demo_server->deleteLater();
      demo_server = new DemoServer(this);

      connect(lobby, &Lobby::settings_requested, this, &AOApplication::call_settings_menu);
      net_manager->get_server_list(std::bind(&Lobby::list_servers, lobby));
      lobby->show();
}

void AOApplication::destruct_lobby()
{
  if (!lobby_constructed) {
    qWarning() << "lobby was attempted destructed when it did not exist";
    return;
  }

  delete w_lobby;
  w_lobby = nullptr;
  lobby_constructed = false;
}

void AOApplication::construct_courtroom()
{
  if (courtroom_constructed) {
    qWarning() << "courtroom was attempted constructed when it already exists";
    return;
  }

  w_courtroom = new Courtroom(this);
  courtroom_constructed = true;

  QRect geometry = QGuiApplication::primaryScreen()->geometry();
  int x = (geometry.width() - w_courtroom->width()) / 2;
  int y = (geometry.height() - w_courtroom->height()) / 2;
  w_courtroom->move(x, y);

  if (demo_server != nullptr) {
    QObject::connect(demo_server, &DemoServer::skip_timers,
                     w_courtroom, &Courtroom::skip_clocks);
  }
  else {
    qWarning() << "demo server did not exist during courtroom construction";
  }
}

void AOApplication::destruct_courtroom()
{
  if (!courtroom_constructed) {
    qWarning() << "courtroom was attempted destructed when it did not exist";
    return;
  }

  delete w_courtroom;
  w_courtroom = nullptr;
  courtroom_constructed = false;
}

QString AOApplication::get_version_string()
{
  return QString::number(RELEASE) + "." + QString::number(MAJOR_VERSION) + "." +
         QString::number(MINOR_VERSION);
}

void AOApplication::load_favorite_list()
{
  favorite_list = read_favorite_servers();
}

void AOApplication::save_favorite_list()
{
  QSettings favorite_servers_ini(get_base_path() + "favorite_servers.ini", QSettings::IniFormat);
  favorite_servers_ini.setIniCodec("UTF-8");

  favorite_servers_ini.clear();
  for(int i = 0; i < favorite_list.size(); ++i) {
    auto fav_server = favorite_list.at(i);
    favorite_servers_ini.beginGroup(QString::number(i));
    favorite_servers_ini.setValue("name", fav_server.name);
    favorite_servers_ini.setValue("address", fav_server.ip);
    favorite_servers_ini.setValue("port", fav_server.port);
    favorite_servers_ini.setValue("desc", fav_server.desc);

    if (fav_server.socket_type == TCP) {
      favorite_servers_ini.setValue("protocol", "tcp");
    } else {
      favorite_servers_ini.setValue("protocol", "ws");
    }
    favorite_servers_ini.endGroup();
  }
  favorite_servers_ini.sync();
}

QString AOApplication::get_current_char()
{
  if (courtroom_constructed)
    return w_courtroom->get_current_char();
  else
    return "";
}

void AOApplication::add_favorite_server(int p_server)
{
  if (p_server < 0 || p_server >= server_list.size())
    return;
  favorite_list.append(server_list.at(p_server));
  save_favorite_list();
}

void AOApplication::remove_favorite_server(int p_server)
{
  if (p_server < 0 || p_server >= favorite_list.size())
    return;
  favorite_list.removeAt(p_server);
  save_favorite_list();
}

void AOApplication::server_disconnected()
{
  if (courtroom_constructed) {
    call_notice(tr("Disconnected from server."));
    open_lobby();
    destruct_courtroom();
  }
}

void AOApplication::loading_cancelled()
{
  destruct_courtroom();
}

void AOApplication::call_settings_menu()
{
    AOOptionsDialog* l_dialog = new AOOptionsDialog(nullptr, this);
    if (courtroom_constructed) {
        connect(l_dialog, &AOOptionsDialog::reloadThemeRequest,
                w_courtroom, &Courtroom::on_reload_theme_clicked);
    }

    if(lobby_constructed) {
    }
    l_dialog->exec();
    delete l_dialog;
}

void AOApplication::call_announce_menu(Courtroom *court)
{
  AOCaseAnnouncerDialog announcer(nullptr, this, court);
  announcer.exec();
}

// Callback for when BASS device is lost
// Only actually used for music syncs
void CALLBACK AOApplication::BASSreset(HSTREAM handle, DWORD channel,
                                       DWORD data, void *user)
{
  Q_UNUSED(handle);
  Q_UNUSED(channel);
  Q_UNUSED(data);
  Q_UNUSED(user);
  doBASSreset();
}

void AOApplication::doBASSreset()
{
  BASS_Free();
  BASS_Init(-1, 48000, BASS_DEVICE_LATENCY, nullptr, nullptr);
  load_bass_plugins();
}

void AOApplication::initBASS()
{
  BASS_SetConfig(BASS_CONFIG_DEV_DEFAULT, 1);
  BASS_Free();
  // Change the default audio output device to be the one the user has given
  // in his config.ini file for now.
  unsigned int a = 0;
  BASS_DEVICEINFO info;

  if (options.audioOutputDevice() == "default") {
    BASS_Init(-1, 48000, BASS_DEVICE_LATENCY, nullptr, nullptr);
    load_bass_plugins();
  }
  else {
    for (a = 0; BASS_GetDeviceInfo(a, &info); a++) {
      if (options.audioOutputDevice() == info.name) {
        BASS_SetDevice(a);
        BASS_Init(static_cast<int>(a), 48000, BASS_DEVICE_LATENCY, nullptr,
                  nullptr);
        load_bass_plugins();
        qInfo() << info.name << "was set as the default audio output device.";
        return;
      }
    }
    BASS_Init(-1, 48000, BASS_DEVICE_LATENCY, nullptr, nullptr);
    load_bass_plugins();
  }
}

#if (defined(_WIN32) || defined(_WIN64))
void AOApplication::load_bass_plugins()
{
  BASS_PluginLoad("bassopus.dll", 0);
  BASS_PluginLoad("bassmidi.dll", 0);
}
#elif defined __APPLE__
void AOApplication::load_bass_plugins()
{
  BASS_PluginLoad("libbassopus.dylib", 0);
  BASS_PluginLoad("libbassmidi.dylib", 0);
}
#elif (defined(LINUX) || defined(__linux__))
void AOApplication::load_bass_plugins()
{
  BASS_PluginLoad("libbassopus.so", 0);
  BASS_PluginLoad("libbassmidi.so", 0);
}
#else
#error This operating system is unsupported for BASS plugins.
#endif