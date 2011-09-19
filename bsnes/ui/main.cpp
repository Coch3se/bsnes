#include "base.hpp"

Application *application = 0;
nall::DSP dspaudio;

void Application::run() {
  inputManager->scan();

  autopause = (mainWindow->focused() == false && config->input.focusPolicy == 2);

  if(interface->loaded() == false || autopause) {
    audio.clear();
    usleep(20 * 1000);
    return;
  }

  interface->run();
}

Application::Application(int argc, char **argv) : quit(false) {
  application = this;
  {
    char path[PATH_MAX];
    auto unused = ::realpath(argv[0], path);
    realpath = path;
    unused = ::userpath(path);
    userpath = path;
    #if defined(PLATFORM_WIN)
    userpath.append("batch/");
    #else
    userpath.append(".config/batch/");
    #endif
    mkdir(userpath, 0755);
  }
  config = new Config;
  interface = new Interface;
  inputManager = new InputManager;
  utility = new Utility;

  title = "bsnes";

  #if defined(PLATFORM_WIN)
  normalFont = "Tahoma, 8";
  boldFont = "Tahoma, 8, Bold";
  titleFont = "Tahoma, 16, Bold";
  #else
  normalFont = "Sans, 8";
  boldFont = "Sans, 8, Bold";
  titleFont = "Sans, 16, Bold";
  #endif

  windowManager = new WindowManager;
  mainWindow = new MainWindow;
  fileBrowser = new FileBrowser;
  settingsWindow = new SettingsWindow;
  cheatEditor = new CheatEditor;
  stateManager = new StateManager;
  windowManager->loadGeometry();

  utility->setMode(Interface::Mode::None);
  mainWindow->setVisible();

  video.driver(config->video.driver);
  video.set(Video::Handle, mainWindow->viewport.handle());
  video.set(Video::Synchronize, config->video.synchronize);
  video.set(Video::Filter, config->video.smooth == false ? 0u : 1u);
  video.init();
  utility->bindVideoShader();

  audio.driver(config->audio.driver);
  audio.set(Audio::Handle, mainWindow->viewport.handle());
  audio.set(Audio::Synchronize, config->audio.synchronize);
  audio.set(Audio::Latency, 60u);
  audio.set(Audio::Frequency, 48000u);
  audio.init();

  dspaudio.setPrecision(16);
  dspaudio.setVolume(config->audio.mute == false ? 1.0 : 0.0);
  dspaudio.setBalance(0.0);
  dspaudio.setResampler(DSP::Resampler::Average);
  dspaudio.setResamplerFrequency(48000.0);

  input.driver(config->input.driver);
  input.set(Input::Handle, mainWindow->viewport.handle());
  input.init();

  if(argc == 2) interface->loadCartridge(argv[1]);

  while(quit == false) {
    OS::processEvents();
    Application::run();
  }

  interface->unloadCartridge();
  windowManager->saveGeometry();
}

Application::~Application() {
  delete stateManager;
  delete cheatEditor;
  delete settingsWindow;
  delete fileBrowser;
  delete mainWindow;
  delete windowManager;
  delete utility;
  delete inputManager;
  delete interface;
  delete config;
}

int main(int argc, char **argv) {
  new Application(argc, argv);
  delete application;
  return 0;
}
