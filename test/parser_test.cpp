#include <gtest/gtest.h>
#include "parser_factory/parser.h"
#include <fstream>
#include <sstream>
#include <thread>
#include <vector>

using namespace parser_factory;

class CpuParserTest : public ::testing::Test {
protected:
    CpuParser cpuParser;
};

// Test GetCPUUsage()
TEST_F(CpuParserTest, GetCPUUsage_ReturnsValidPercentage) {
    std::string usage = cpuParser.GetCPUUsage();
    EXPECT_FALSE(usage.empty());
    std::cerr << "Output: " << usage << std::endl;
    EXPECT_TRUE(usage.find('%') != std::string::npos);  // Check if output contains '%'
}

// Test GetCPUInfo()
TEST_F(CpuParserTest, GetCPUInfo_ReturnsNonEmpty) {
    std::string info = cpuParser.GetCPUInfo();
    EXPECT_FALSE(info.empty());
}

// Test GetCpuUtilization()
TEST_F(CpuParserTest, GetCpuUtilization_ReturnsValidData) {
    std::vector<cpu_data_t> data = cpuParser.GetCpuUtilization();
    ASSERT_FALSE(data.empty());
    EXPECT_GE(data.front().getTotalJiffies(), 0);
    EXPECT_GE(data.front().getActiveJiffies(), 0);
}

// Test GetActiveJiffies() system-wide
TEST_F(CpuParserTest, GetActiveJiffies_ReturnsPositiveValue) {
    long activeJiffies = cpuParser.GetActiveJiffies();
    EXPECT_GE(activeJiffies, 0);
}

// Test GetActiveJiffies(int pid) with a known process
TEST_F(CpuParserTest, GetActiveJiffiesForPID_ReturnsPositiveValue) {
    int pid = getpid();  // Get current process ID
    long activeJiffies = cpuParser.GetActiveJiffies(pid);
    EXPECT_GE(activeJiffies, 0);
}

// Test GetProcessorUtilization(int pid)
TEST_F(CpuParserTest, GetProcessorUtilization_ReturnsValidStruct) {
    int pid = getpid();  // Get current process ID
    pid_state_t procStats = cpuParser.GetProcessorUtilization(pid);
    ASSERT_EQ(procStats.pid, pid) << "PID mismatch";
    EXPECT_GE(procStats.utime, 0)<< "utime is greater than 0";
    EXPECT_GE(procStats.stime, 0)<< "stime is greater than 0";
}

// Test GetActiveJiffies()
TEST_F(CpuParserTest, GetJiffies_ReturnsPositiveValue) {
    long activeJiffies = cpuParser.GetActiveJiffies();
    EXPECT_GE(activeJiffies, 0);
}

// Test GetActiveJiffies()
TEST_F(CpuParserTest, GetJiffies_pid_ReturnsPositiveValue) {
    int pid = getpid();  // Get current process ID
    long activeJiffies = cpuParser.GetActiveJiffies(pid);
    EXPECT_GE(activeJiffies, 0);
}
// Test GetIdleJiffies()
TEST_F(CpuParserTest, GetIdleJiffies_ReturnsPositiveValue) {
    long idleJiffies = cpuParser.GetIdleJiffies();
    EXPECT_GE(idleJiffies, 0)<< "idleJiffies is greater than 0";
}