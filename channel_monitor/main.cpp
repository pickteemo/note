#include "cyber_topology_message.h"
#include "general_channel_message.h"
#include "screen.h"

#include <csignal>
#include <iostream>

void SigResizeHandle(int) {
  Screen::Instance()->Resize();
}
void SigCtrlCHandle(int) {
  Screen::Instance()->Stop();
}
void printHelp(const char* cmdName) {
  std::cout << "Usage:\n"
            << cmdName << "  [option]\nOption:\n"
            << "   -h print help info\n"
            << "   -c specify one channel\n"
            << "Interactive Command:\n"
            << Screen::InteractiveCmdStr << std::endl;
}
enum COMMAND {
  TOO_MANY_PARAMETER,
  HELP,       // 2
  NO_OPTION,  // 1
  CHANNEL     // 3 -> 4
};

void spin() {
  LcmAdapter::Instance()->Spin();
}

COMMAND parseOption(int argc, char* const argv[], std::string& commandVal) {
  if (argc > 4) {
    return TOO_MANY_PARAMETER;
  }
  int index = 1;
  while (true) {
    const char* opt = argv[index];
    if (opt == nullptr) {
      break;
    }
    if (strcmp(opt, "-h") == 0) {
      return HELP;
    }
    if (strcmp(opt, "-c") == 0) {
      if (argv[index + 1]) {
        commandVal = argv[index + 1];
        return CHANNEL;
      }
    }

    ++index;
  }

  return NO_OPTION;
}

int main(int argc, char* argv[]) {
  std::string val;

  COMMAND com = parseOption(argc, argv, val);

  switch (com) {
    case TOO_MANY_PARAMETER:
      std::cout << "Too many paramtes\n";
    case HELP:
      printHelp(argv[0]);
      return 0;
    default:;
  }

  std::thread t_lcm = std::thread(&spin);
  t_lcm.detach();

  CyberTopologyMessage topologyMsg(val);

  auto topologyCallback =
      [&topologyMsg](const apollo::cyber::proto::ChangeMsg& change_msg) {
        topologyMsg.TopologyChanged(change_msg);
      };

  Screen* s = Screen::Instance();

  signal(SIGWINCH, SigResizeHandle);
  signal(SIGINT, SigCtrlCHandle);

  s->SetCurrentRenderMessage(&topologyMsg);

  s->Init();
  s->Run();

  return 0;
}
