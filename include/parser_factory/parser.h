#ifndef LINUX_PARSER_H
#define LINUX_PARSER_H

//external includes liberaries
#include <filesystem>
#include <fstream>
#include <regex>
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <limits.h>

//internal includes liberaries
#include "logger/logger_singletone.h"



namespace parser_factory {

// -----------------------------
// File Paths (Centralized)
static const std::unordered_map<std::string, std::string> LinuxFilesSet{
    {"kCmdlineFilename", "/proc/cmdline"},
    {"kCpuinfoFilename", "/proc/cpuinfo"},
    {"kStatFilename", "/proc/stat"},
    {"kUptimeFilename", "/proc/uptime"},
    {"kMeminfoFilename", "/proc/meminfo"},
    {"kVersionFilename", "/proc/version"},
    {"kOSReleaseFilename", "/etc/os-release"},
    {"kPasswordFilename", "/etc/passwd"}};

// -----------------------------
// CPU State Enum
enum class CPUStates {
  kUser_ = 0,
  kNice_,
  kSystem_,
  kIdle_,
  kIOwait_,
  kIRQ_,
  kSoftIRQ_,
  kSteal_,
  kGuest_,
  kGuestNice_
};

/*
    pid (comm) 
    state 
    ppid 
    pgrp 
    session 
    tty_nr 
    tpgid 
    flags 
    minflt 
    cminflt 
    majflt 
    cmajflt 
    utime 
    stime 
    cutime 
    cstime 
    priority 
    nice 
    num_threads 
    itrealvalue 
    starttime 
    vsize 
    rss 
    rsslim 
    startcode 
    endcode 
    startstack 
    kstkesp 
    kstkeip 
    signal 
    blocked 
    sigignore 
    sigcatch
    wchan 
    nswap 
    cnswap 
    exit_signal 
    processor 
    rt_priority 
    policy 
    delayacct_blkio_ticks 
    gtime 
    cgtime
*/
typedef struct PidState {
  int pid;                     /** process id **/
  std::string tcomm;  /** filename of the executable **/
  char state;                   /** state (R is running, S is sleeping, D is sleeping in an
                                     uninterruptible wait, Z is zombie, T is traced or stopped) **/
  int ppid;                     /** process id of the parent process **/
  int pgrp;                     /** pgrp of the process **/
  int sid;                      /** session id **/
  int tty_nr;                   /** tty the process uses **/
  int tty_pgrp;                 /** pgrp of the tty **/
  unsigned int flags;           /** task flags **/
  unsigned int min_flt;         /** number of minor faults **/
  unsigned int cmin_flt;        /** number of minor faults with child's **/
  unsigned int maj_flt;         /** number of major faults **/
  unsigned int cmaj_flt;        /** number of major faults with child's **/
  int utime;                    /** user mode jiffies **/
  int stime;                    /** kernel mode jiffies **/
  int cutime;                   /** user mode jiffies with child's **/
  int cstime;                   /** kernel mode jiffies with child's **/
  int priority;                 /** priority level **/
  int nice;                     /** nice level **/
  int num_threads;              /** number of threads **/
  int it_real_value;            /** (obsolete, always 0) **/
  int start_time;               /** time the process started after system boot **/
  unsigned int vsize;           /** virtual memory size **/
  unsigned int rss;             /** resident set memory size **/
  unsigned long long rsslim;          /** current limit in bytes on the rss **/
  unsigned int start_code;      /** address above which program text can run **/
  unsigned int end_code;        /** address below which program text can run **/
  unsigned int start_stack;     /** address of the start of the main process stack **/
  unsigned int esp;             /** current value of ESP **/
  unsigned int eip;             /** current value of EIP **/
  int pending;                  /** bitmap of pending signals **/
  int blocked;                  /** bitmap of blocked signals **/
  int sigign;                   /** bitmap of ignored signals **/
  int sigcatch;                 /** bitmap of caught signals **/
  unsigned int wchan;           /** (place holder, use /proc/PID/wchan instead) **/
  unsigned int zero1;           /** (place holder) **/
  unsigned int zero2;           /** (place holder) **/
  int exit_signal;              /** signal to send to parent thread on exit **/
  int task_cpu;                 /** which CPU the task is scheduled on **/
  int rt_priority;              /** realtime priority **/
  int policy;                   /** scheduling policy (man sched_setscheduler) **/
  unsigned int blkio_ticks;     /** time spent waiting for block IO **/
  int gtime;                    /** guest time of the task in jiffies **/
  int cgtime;                   /** guest time of the task children in jiffies **/
  unsigned int start_data;      /** address above which program data+bss is placed **/
  unsigned int end_data;        /** address below which program data+bss is placed **/
  unsigned int start_brk;       /** address above which program heap can be expanded with brk() **/
  unsigned int arg_start;       /** address above which program command line is placed **/
  unsigned int arg_end;         /** address below which program command line is placed **/
  unsigned int env_start;       /** address above which program environment is placed **/
  unsigned int env_end;         /** address below which program environment is placed **/
  int exit_code;                /** the thread's exit_code in the form reported by the waitpid system call **/
  unsigned long getActiveJiffies() const {return (utime + stime + cutime + cstime);}
}pid_state_t;

/*
User – Time in user mode.
Nice – Time in low-priority user mode.
System – Time in kernel mode.
Idle – Time spent idle.
I/O Wait – Time waiting for I/O.
IRQ – Time servicing interrupts.
SoftIRQ – Time servicing soft interrupts.
Steal – Time stolen by other virtual machines.
*/
typedef struct CPUData {
  long user;
  long nice;
  long system;
  long idle;
  long iowait;
  long irq;
  long softirq;
  long steal;
  long guest;
  long guest_nice;
  long getActiveJiffies() const {return user + nice + system + irq + softirq + steal;}
  long getIdleJiffies() const {return idle + iowait;}
  long getTotalJiffies() const { return getActiveJiffies() + idle + iowait; }
} cpu_data_t;

// -----------------------------
// Interfaces for Each Component
class ICpuParser {
 public:
  virtual std::string GetCPUUsage() = 0;
  virtual std::string GetCPUInfo() = 0;
  virtual std::vector<cpu_data_t> GetCpuUtilization() = 0;
  virtual pid_state_t GetProcessorUtilization(int pid) = 0;
  virtual long GetJiffies() = 0;
  virtual long GetActiveJiffies() = 0;
  virtual long GetActiveJiffies(int pid) = 0;
  virtual long GetIdleJiffies() = 0;
  virtual ~ICpuParser() = default;
};

class IMemoryParser {
 public:
  virtual std::string GetMemoryUsage() = 0;
  virtual std::string GetRAMInfo() = 0;
  virtual ~IMemoryParser() = default;
};

class INetworkParser {
 public:
  virtual std::string GetNetworkUsage() = 0;
  virtual ~INetworkParser() = default;
};

class IProcessParser {
 public:
  virtual std::string GetCommand(int pid) = 0;
  virtual std::string GetRam(int pid) = 0;
  virtual std::string GetUid(int pid) = 0;
  virtual std::string GetUser(int pid) = 0;
  virtual long GetUpTime(int pid) = 0;
  virtual std::vector<int> GetPids() = 0;
  virtual int GetTotalProcesses() = 0;
  virtual int GetRunningProcesses() = 0;
  virtual ~IProcessParser() = default;
};

class ISystemParser {
 public:
  virtual std::string GetSystemInfo() = 0;
  virtual std::string GetSystemUptime() = 0;
  virtual std::string GetTemperature() = 0;
  virtual std::string GetDiskUsage() = 0;
  virtual std::string GetLogs() = 0;
  virtual std::string GetHistoricalUsageData() = 0;
  virtual std::string GetResponseTime() = 0;
  virtual std::string GetLatency() = 0;
  virtual std::string GetPlatformSpecificData() = 0;
  virtual ~ISystemParser() = default;
};

// -----------------------------
// Specific Parsers for Each Component
class CpuParser : public ICpuParser {
 public:
 CpuParser() : cpu_data_list_(std::make_shared<std::vector<cpu_data_t>>()) {}
  std::string GetCPUUsage() override;
  std::string GetCPUInfo() override;
  std::vector<cpu_data_t> GetCpuUtilization() override;
  pid_state_t GetProcessorUtilization(int pid) override;
  long GetJiffies() override;
  long GetActiveJiffies() override;
  long GetActiveJiffies(int pid) override;
  long GetIdleJiffies() override;
  ~CpuParser() = default;
  private:
  cpu_data_t cpu_data_;
  Logger& logger_ = Logger::GetInstance();
  // Shared pointer to hold the vector, shared across functions
  std::shared_ptr<std::vector<cpu_data_t>> cpu_data_list_;
  std::shared_ptr<pid_state_t> pid_data_;
};

class MemoryParser : public IMemoryParser {
 public:
  std::string GetMemoryUsage() override;
  std::string GetRAMInfo() override;
};

class NetworkParser : public INetworkParser {
 public:
  std::string GetNetworkUsage() override;
};

class ProcessParser : public IProcessParser {
 public:
  std::string GetCommand(int pid) override;
  std::string GetRam(int pid) override;
  std::string GetUid(int pid) override;
  std::string GetUser(int pid) override;
  long GetUpTime(int pid) override;
  std::vector<int> GetPids() override;
  int GetTotalProcesses() override;
  int GetRunningProcesses() override;
};

class SystemParser : public ISystemParser {
 public:
  SystemParser(CpuParser& cpuParser, MemoryParser& memoryParser,
               ProcessParser& processParser);

  std::string GetSystemInfo() override;
  std::string GetSystemUptime() override;
  std::string GetTemperature() override;
  std::string GetDiskUsage() override;
  std::string GetLogs() override;
  std::string GetHistoricalUsageData() override;
  std::string GetResponseTime() override;
  std::string GetLatency() override;
  std::string GetPlatformSpecificData() override;

 private:
  CpuParser& cpuParser_;
  MemoryParser& memoryParser_;
  ProcessParser& processParser_;
};

}  // namespace parser_factory

#endif  // LINUX_PARSER_H
