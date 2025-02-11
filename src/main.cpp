#include "ncurses_display.h"
#include "system.h"
#include "logger/logger_singletone.h"

int main(int argc, char **argv) {

  Logger& logger_ = Logger::GetInstance();
  logger_.Log(LogLevel::INFO, "Starting System Monitor");
  System system;
  NCursesDisplay::Display(system);
  return EXIT_SUCCESS;
}