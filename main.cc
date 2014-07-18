#include "fetcher_main_runner.h"

int main(int argc, const char* argv[]) {
  base::AtExitManager exit_manager;

  FetcherMainRunner main_runner;
  main_runner.SetCommandLine(argc, argv);
  main_runner.Initialize();

  

  return main_runner.Run();
}
