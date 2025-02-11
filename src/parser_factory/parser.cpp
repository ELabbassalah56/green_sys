#include "parser_factory/parser.h"

#include <fcntl.h>
#include <sys/inotify.h>
#include <unistd.h>

#include <chrono>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <thread>

using namespace parser_factory;
namespace fs = std::filesystem;

// -----------------------------
// CpuParser Implementation
std::string CpuParser::GetCPUUsage() {
  const int num_samples = 5;
  long total_time_diff = 0;
  long active_time_diff = 0;

  // Take multiple samples for averaging
  for (int i = 0; i < num_samples; ++i) {
    std::vector<cpu_data_t> old_cpu_data_list = GetCpuUtilization();
    if (old_cpu_data_list.empty()) {
      logger_.Log(LogLevel::ERROR, "Failed to retrieve CPU data.");
    }

    std::this_thread::sleep_for(
        std::chrono::milliseconds(500));  // Shorter sleep

    std::vector<cpu_data_t> new_cpu_data_list = GetCpuUtilization();
    if (new_cpu_data_list.empty()) {
      logger_.Log(LogLevel::ERROR, "Failed to retrieve new CPU data.");
    }

    long old_total_time = old_cpu_data_list.front().getTotalJiffies();
    long old_active_time = old_cpu_data_list.front().getActiveJiffies();
    long new_total_time = new_cpu_data_list.front().getTotalJiffies();
    long new_active_time = new_cpu_data_list.front().getActiveJiffies();

    if (new_total_time == old_total_time) {
      logger_.Log(
          LogLevel::INFO,
          "Total time difference is zero, unable to calculate CPU usage.");
    }

    total_time_diff += new_total_time - old_total_time;
    active_time_diff += new_active_time - old_active_time;
  }

  // Calculate average CPU usage
  double cpu_utilization_percentage = (static_cast<double>(active_time_diff) /
                                       static_cast<double>(total_time_diff)) *
                                      100;
  return std::to_string(cpu_utilization_percentage) + "%";
}

std::string CpuParser::GetCPUInfo() {
  // Implementation to retrieve CPU Info
  std::ifstream cpu_info_file(LinuxFilesSet.at("kCpuinfoFilename"));
  if (!cpu_info_file.is_open()) {
    logger_.Log(LogLevel::ERROR, "Failed to open CPU info file.");
  }
  std::stringstream buffer;
  buffer << cpu_info_file.rdbuf();
  return buffer.str();
}

std::vector<cpu_data_t> CpuParser::GetCpuUtilization() {
  // Implementation to retrieve CPU utilization statistics
  if (!fs::exists(LinuxFilesSet.at("kStatFilename"))) {
    throw std::runtime_error("File not found: " +
                             LinuxFilesSet.at("kStatFilename"));
  }
  std::ifstream stat_file(LinuxFilesSet.at("kStatFilename"));
  if (!stat_file.is_open()) {
    logger_.Log(LogLevel::ERROR, "Failed to open stat file.");
  }
  // Ensure that pid_data_ is initialized before use
  if (!cpu_data_list_) {
    cpu_data_list_ = std::make_shared<std::vector<cpu_data_t>>();
  }
  std::string rLine;
  while (std::getline(stat_file, rLine)) {
    std::istringstream iss(rLine);
    std::string key;
    iss >> key;

    // Look for CPU usage statistics
    if (key.find("cpu") == 0) {
      iss >> cpu_data_.user >> cpu_data_.nice >> cpu_data_.system >>
          cpu_data_.idle >> cpu_data_.iowait >> cpu_data_.irq >>
          cpu_data_.softirq >> cpu_data_.steal >> cpu_data_.guest >>
          cpu_data_.guest_nice;
      cpu_data_list_->push_back(cpu_data_);
    } else {
      break;  // Stop reading after CPUs
    }
  }
  return *cpu_data_list_;
}

long CpuParser::GetJiffies() {
  // Implementation to retrieve jiffies (system ticks)

  return cpu_data_list_->front().getTotalJiffies();
}

pid_state_t CpuParser::GetProcessorUtilization(int pid) {
  fs::path pid_stat_path = "/proc/" + std::to_string(pid) + "/stat";

  if (!fs::exists(pid_stat_path)) {
    throw std::runtime_error("File not found: " + pid_stat_path.string());
  }

  std::ifstream pid_stat_file(pid_stat_path);
  if (!pid_stat_file.is_open()) {
    logger_.Log(LogLevel::ERROR, "Failed to open PID stat file.");
  }

  pid_data_ = std::make_shared<pid_state_t>();
  std::string rLine;
  std::getline(pid_stat_file, rLine);
  std::istringstream iss(rLine);  
  // Read values into token struct
  iss >> pid_data_->pid;

  // Manually handle the command name enclosed in parentheses
  iss.ignore(); // Ignore the space before the command name
  std::getline(iss, pid_data_->tcomm, ')');
  pid_data_->tcomm = pid_data_->tcomm.substr(1); // Remove the leading '('

  // Continue parsing the rest of the fields
  if (!(iss >> pid_data_->state >> pid_data_->ppid >> pid_data_->pgrp >>
        pid_data_->sid >> pid_data_->tty_nr >> pid_data_->tty_pgrp >>
        pid_data_->flags >> pid_data_->min_flt >> pid_data_->cmin_flt >>
        pid_data_->maj_flt >> pid_data_->cmaj_flt >> pid_data_->utime >>
        pid_data_->stime >> pid_data_->cutime >> pid_data_->cstime >>
        pid_data_->priority >> pid_data_->nice >> pid_data_->num_threads >>
        pid_data_->it_real_value >> pid_data_->start_time >> pid_data_->vsize >>
        pid_data_->rss >> pid_data_->rsslim >> pid_data_->start_code >>
        pid_data_->end_code >> pid_data_->start_stack >> pid_data_->esp >>
        pid_data_->eip >> pid_data_->pending >> pid_data_->blocked >>
        pid_data_->sigign >> pid_data_->sigcatch >> pid_data_->wchan >>
        pid_data_->zero1 >> pid_data_->zero2 >> pid_data_->exit_signal >>
        pid_data_->task_cpu >> pid_data_->rt_priority >> pid_data_->policy >>
        pid_data_->blkio_ticks >> pid_data_->gtime >> pid_data_->cgtime >>
        pid_data_->start_data >> pid_data_->end_data >> pid_data_->start_brk >>
        pid_data_->arg_start >> pid_data_->arg_end >> pid_data_->env_start >>
        pid_data_->env_end >> pid_data_->exit_code)) {
    // Log failure with specific details
    logger_.Log(LogLevel::FATAL, "Failed to parse /proc/[pid]/stat correctly.");
    logger_.Log(LogLevel::FATAL, "Read line: " + rLine);
  } else {
    logger_.Log(LogLevel::INFO,"Successfully parsed /proc/[pid]/stat.");
  }

  return *pid_data_;
}

long CpuParser::GetActiveJiffies() {
  // Implementation to retrieve active jiffies
  // Example:
  if(cpu_data_list_->empty()) {
    GetCpuUtilization();
  }
  return cpu_data_list_->front().getActiveJiffies();
}

long CpuParser::GetActiveJiffies(int pid) {
  // Implementation to retrieve active jiffies for a specific process
  GetProcessorUtilization(pid);
  return pid_data_->getActiveJiffies();
}

long CpuParser::GetIdleJiffies() {
  // Implementation to retrieve idle jiffies
  if(cpu_data_list_->empty()) {
    GetCpuUtilization();
  }
  return cpu_data_list_->front().getIdleJiffies();
}

// -----------------------------
// MemoryParser Implementation

std::string MemoryParser::GetMemoryUsage() {
  // Implementation to retrieve memory usage
  // Example:
  return "Memory Usage Data";
}

std::string MemoryParser::GetRAMInfo() {
  // Implementation to retrieve RAM info
  std::ifstream ram_info_file(LinuxFilesSet.at("kMeminfoFilename"));
  if (!ram_info_file.is_open()) {
    throw std::runtime_error("Failed to open RAM info file.");
  }
  std::stringstream buffer;
  buffer << ram_info_file.rdbuf();
  return buffer.str();
}

// -----------------------------
// NetworkParser Implementation

std::string NetworkParser::GetNetworkUsage() {
  // Implementation to retrieve network usage data
  // Example:
  return "Network Usage Data";
}

// -----------------------------
// ProcessParser Implementation

std::string ProcessParser::GetCommand(int pid) {
  // Implementation to retrieve the command line for a process
  std::ifstream cmd_file(LinuxFilesSet.at("kProcDirectory") +
                         std::to_string(pid) +
                         LinuxFilesSet.at("kCmdlineFilename"));
  if (!cmd_file.is_open()) {
    throw std::runtime_error("Failed to open command file.");
  }
  std::stringstream buffer;
  buffer << cmd_file.rdbuf();
  return buffer.str();
}

std::string ProcessParser::GetRam(int pid) {
  // Implementation to retrieve RAM usage for a specific process
  // Example:
  return "RAM Usage Data";
}

std::string ProcessParser::GetUid(int pid) {
  // Implementation to retrieve UID for a specific process
  // Example:
  return "User ID";
}

std::string ProcessParser::GetUser(int pid) {
  // Implementation to retrieve the user name for a specific process
  // Example:
  return "User Name";
}

long ProcessParser::GetUpTime(int pid) {
  // Implementation to retrieve uptime for a specific process
  // Example:
  return 120;
}

std::vector<int> ProcessParser::GetPids() {
  // Implementation to retrieve the list of process IDs
  std::vector<int> pids = {1, 2, 3};  // Example PIDs
  return pids;
}

int ProcessParser::GetTotalProcesses() {
  // Implementation to retrieve total number of processes
  // Example:
  return 1000;
}

int ProcessParser::GetRunningProcesses() {
  // Implementation to retrieve number of running processes
  // Example:
  return 500;
}

// -----------------------------
// SystemParser Implementation

SystemParser::SystemParser(CpuParser& cpuParser, MemoryParser& memoryParser,
                           ProcessParser& processParser)
    : cpuParser_(cpuParser),
      memoryParser_(memoryParser),
      processParser_(processParser) {}

std::string SystemParser::GetSystemInfo() {
  // Implementation to retrieve system information
  // Example:
  return "System Information Data";
}

std::string SystemParser::GetSystemUptime() {
  // Implementation to retrieve system uptime
  std::ifstream uptime_file(LinuxFilesSet.at("kUptimeFilename"));
  if (!uptime_file.is_open()) {
    throw std::runtime_error("Failed to open uptime file.");
  }
  std::stringstream buffer;
  buffer << uptime_file.rdbuf();
  return buffer.str();
}

std::string SystemParser::GetTemperature() {
  // Implementation to retrieve system temperature (if available)
  // Example:
  return "Temperature Data";
}

std::string SystemParser::GetDiskUsage() {
  // Implementation to retrieve disk usage data
  // Example:
  return "Disk Usage Data";
}

std::string SystemParser::GetLogs() {
  // Implementation to retrieve system logs
  // Example:
  return "Logs Data";
}

std::string SystemParser::GetHistoricalUsageData() {
  // Implementation to retrieve historical usage data
  // Example:
  return "Historical Data";
}

std::string SystemParser::GetResponseTime() {
  // Implementation to retrieve response time data
  // Example:
  return "Response Time Data";
}

std::string SystemParser::GetLatency() {
  // Implementation to retrieve system latency
  // Example:
  return "Latency Data";
}

std::string SystemParser::GetPlatformSpecificData() {
  // Implementation to retrieve platform-specific data
  // Example:
  return "Platform Specific Data";
}
