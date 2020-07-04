#include "firefly.h"


firefly::firefly(std::string path, TEST_HARNESS* dut, uint64_t* counter, float threshold)
{
  this->tile = dut;
  this->trace_count = counter;
  this->interm_actions = 0;
  this->lost_ints = 0;
  this->is_mscratch_init = false;
  this->trace_count = counter;
  this->current_voltage;
  this->step_cycles = 120;
  this->last_count = 0;
  this->threshold = threshold;
  this->fire_interrupt = false;
  this->restore_context = false;
  this->read_voltage_trace(path);

  if (*this->trace_count == 0)
  {
    if (this->voltages[0] > this->threshold)
    {
      state = __FIREFLY_ON;
    }
    else
    {
      state = __FIREFLY_OFF;
    }
    push_log(state);
  }
}

firefly::~firefly()
{
  // Delete log
  // Delete mem_log
}

void firefly::update()
{
  update_voltage();
  update_mscratch();
  update_interrupts();
  update_mem_trace();
}

void firefly::print_report()
{
  std::cout << "Cycles: " << *trace_count << "\n"
               "Step: " << step_cycles << " cycles\n"
               "Threshold: " << threshold << " V\n"
               "Intermittent actions: " << interm_actions << "\n"
               "Lost interrupts: " << lost_ints << std::endl;
  print_log();
}

void firefly::save_report(std::string path)
{
  std::ofstream file(path);

  file << "<cycles>" << *trace_count << "</cycles>\n" <<
          "<step>" << step_cycles << "</step>\n" <<
          "<threshold>" << threshold << "</threshold>\n" <<
          "<ints>" << interm_actions << "</ints>\n" <<
          "<lost_ints>" << lost_ints << "</lost_insts>" << std::endl;

  for (auto entry : log)
  {
    file << "<entry>\n";
      file << "<time>" << entry->time << "</time>\n";
      file << "<mode>" << entry->mode << "</mode>\n";
    file << "</entry>" << :: std::endl;
  }
}

void firefly::save_mem_trace(std::string path)
{
  std::ofstream file(path);

  for (auto entry : mem_log)
  {
    std::bitset<128> data(entry->data);

    file << std::dec << entry->cycle
         << " "
         << entry->op
         << " "
         << std::setfill('0') << std::setw(8) << std::hex << entry->address
         << " "
         << data
         << " 0" << std::endl;
  }
}


void firefly::set_threshold(float val)
{
  if (threshold > 0)
  {
    threshold = val;
  }
  else
  {
    std::cout << "FF: New threshold <= 0(" << val << "). Assuming threshold equal to "
    << threshold << "." <<std::endl;
  }
}

void firefly::init_mscratch(uint64_t mscratch)
{
  static bool is_first_run = true;
  static uint64_t prev_mscratch;
  if (is_first_run)
  {
    prev_mscratch = mscratch;
    is_first_run = false;
  }
  else
  {
    // Check whether a transiction to 0 happened.
    if (prev_mscratch != 0 && mscratch == 0)
    {
      is_mscratch_init = true;
    }
  }
  prev_mscratch = mscratch;
}

void firefly::set_mscratch(uint64_t val)
{
  tile->TestHarness__DOT__ldut__DOT__tile__DOT__core__DOT__csr__DOT__reg_mscratch = val;
}

void firefly::update_mscratch()
{
  uint64_t mscratch = tile->TestHarness__DOT__ldut__DOT__tile__DOT__core__DOT__csr__DOT__reg_mscratch;

  if (is_mscratch_init)
  {
    if (mscratch == __FIREFLY_SAVE_CONTEXT)
    {
      state = __FIREFLY_OFF;
      set_mscratch(__FIREFLY_OFF);
      push_log(state);
    }
    if (restore_context)
    {
      restore_context = false;
      set_mscratch(__FIREFLY_ON);
    }
    /*
    if (mscratch == __FIREFLY_SAVE_CONTEXT)
    {
      set_mscratch(__FIREFLY_OFF);
      state = __FIREFLY_OFF;
      push_log(state);
      std::cout << "Context succesfully saved! Trace: " << *trace_count << "\n";
    }
    // If we have charge to restore context, then release cpu.
    if (restore_context)
    {
      restore_context = false;
      set_mscratch(__FIREFLY_ON);
      std::cout << "State: " << state << " restoring context... " << "Trace: " << *trace_count << "\n";
    }
    */
  }
  else
  {
    init_mscratch(mscratch);
  }
}

void firefly::update_interrupts()
{
  if (fire_interrupt)
  {
    tile->io_sim_int = 1;
    fire_interrupt = false;
    interm_actions++;
  }
  else
  {
    tile->io_sim_int = 0;
  }
}

void firefly::update_voltage()
{
  if (*this->trace_count >= this->last_count)
  {
    this->last_count = *this->trace_count + this->step_cycles;
    inc_voltages_index(1);
  }

  if (voltages[current_voltage] > threshold)
  {
    if (state == __FIREFLY_OFF)
    {
      restore_context = true;
      state = __FIREFLY_ON;
      push_log(state);

    }
    if (state == __FIREFLY_SAVE_CONTEXT)
    {
      state = __FIREFLY_ON;
      push_log(state);
      lost_ints++;
    }

    state = __FIREFLY_ON;
  }
  else if (state == __FIREFLY_ON)
  {
    fire_interrupt = true;
    state = __FIREFLY_SAVE_CONTEXT;
    push_log(state);
  }
  else if (state == __FIREFLY_OFF)
  {
    this->skip_off();
  }
}

uint64_t firefly::get_next_step(uint64_t cycle)
{
  uint64_t rem = cycle % this->step_cycles;
  return cycle + (this->step_cycles - rem);

}

void firefly::inc_voltages_index(uint32_t steps)
{
  for (uint32_t i = 0; i < steps; ++i)
  {
    this->current_voltage++;
    if (this->current_voltage >= this->voltages.size())
    {
      this->current_voltage = 0;
    }
  }
}

void firefly::dec_voltages_index(uint32_t steps)
{
  for (uint32_t i = 0; i < steps; ++i)
  {
    this->current_voltage--;
    if (this->current_voltage == 0)
    {
      this->current_voltage = this->voltages.size();
    }
  }
}

void firefly::skip_off()
{
  uint64_t cycle = *this->trace_count;

  while (this->voltages[this->current_voltage] <= this->threshold)
  {
    cycle = this->get_next_step(cycle);
    this->inc_voltages_index(1);
  }
  *this->trace_count = cycle - 1;
  this->dec_voltages_index(1);
}

void firefly::read_voltage_trace(std::string path)
{
  std::ifstream trace(path);
  std::string line;

  while(getline(trace, line))
  {
    float v;
    std::istringstream in(line);

    in >> v;
    voltages.push_back(v);
  }

  trace.close();
  last_count = *trace_count;
  current_voltage = -1;
}

void firefly::update_mem_trace()
{
  if (mem_trace == true)
  {
    if(is_read_en()) {
      push_mem_log(*trace_count, 'R', get_mem_addr(), get_mem_data());
    }
    if(is_write_en()) {
      push_mem_log(*trace_count, 'W', get_mem_addr(), get_mem_data());
    }
  }
}

void firefly::push_mem_log(uint64_t cycle, uint32_t op, uint64_t address, uint64_t data)
{
  ff_mem_t* entry = new ff_mem_t;
  entry->cycle = cycle;
  entry-> op = op;
  entry->address = address;
  entry->data = data;
  mem_log.push_back(entry);
}

bool firefly::is_read_en()
{
  return tile->TestHarness__DOT__mem__DOT__srams__DOT__mem__DOT__mem_ext__DOT__reg_R0_ren;
}

bool firefly::is_write_en()
{
  return tile->TestHarness__DOT__mem__DOT__srams__DOT__w_full && tile->TestHarness__DOT__mem__DOT__srams__DOT__w_sel1;
}

uint64_t firefly::get_mem_data()
{
  return tile->TestHarness__DOT__mem__DOT__srams__DOT__mem__DOT__mem_ext_R0_data;
}

uint64_t firefly::get_mem_addr()
{
  return tile->TestHarness__DOT__mem__DOT__srams__DOT__mem__DOT__mem_ext__DOT__reg_R0_addr;
}

void firefly::push_log(uint32_t mode)
{
  ff_log_t* entry = new ff_log_t;
  entry->mode = mode;
  entry->time = *trace_count;
  log.push_back(entry);
}

void firefly::print_log()
{
  for (auto entry : log)
  {
    std::cout << "Time: " << entry->time << " Mode: " << entry->mode << "\n";
  }
}
