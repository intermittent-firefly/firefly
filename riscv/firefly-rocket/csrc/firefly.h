#ifndef FIREFLY_H
#define FIREFLY_H

#include "verilated.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <string>
#include <sstream>
#include <iomanip>
#include <bitset>

#include <stdio.h>
#include <stdlib.h>

//----- Firefly functions -----//
#define __FIREFLY_ON 0
#define __FIREFLY_SAVE_CONTEXT 1
#define __FIREFLY_OFF 2

typedef struct firefly_log 
{
  uint64_t time;
  uint32_t mode;
} ff_log_t;

typedef struct firefly_mem_trace
{
  uint64_t cycle;
  char     op;
  uint64_t address;
  uint64_t data;
} ff_mem_t;

class firefly
{
  TEST_HARNESS* tile;
  uint32_t state;
  bool is_mscratch_init = false;
  uint64_t* trace_count = NULL;

  std::vector<float> voltages;
  std::list<ff_log_t*> log;
  std::list<ff_mem_t*> mem_log;

  uint64_t current_voltage;
  uint64_t step_cycles;
  uint64_t last_count = 0;
  float threshold = 0.07f;
  bool fire_interrupt = false;
  bool restore_context = false;
  bool mem_trace = true;

  uint32_t interm_actions;
  uint32_t lost_ints;

public:
  firefly(std::string path, TEST_HARNESS* tile, uint64_t* counter, float threshold);
  ~firefly();
  void update();
  void print_report();
  void save_report(std::string path);
  void save_mem_trace(std::string path);
  void set_threshold(float val);

private:
  // Comunication with mscratch CSR
  void init_mscratch(uint64_t mscratch);
  void set_mscratch(uint64_t val);
  void update_mscratch();
  // Interrupts
  void update_interrupts();
  // Voltage
  void update_voltage();
  uint64_t get_next_step(uint64_t cycle);
  void inc_voltages_index(uint32_t steps);
  void dec_voltages_index(uint32_t steps);
  void skip_off();
  void read_voltage_trace(std::string path);
  // Memory acesses monitor
  void update_mem_trace();
  bool is_read_en();
  bool is_write_en();
  uint64_t get_mem_data();
  uint64_t get_mem_addr();
  // Log related functions
  void push_log(uint32_t mode);
  void print_log();
  void push_mem_log(uint64_t cycle, uint32_t op, uint64_t address, uint64_t data);
};


#endif
