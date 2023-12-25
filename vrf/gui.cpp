#include <QApplication>
#include "gui_output.hpp"
#include <thread>
#include "tb.h"
#include "Vgui_gui.h"
#include "Vgui_orion_pro_top.h"

double sc_time_stamp() { return 0; }

#define TICK_TIME 2
#define TICK_PERIOD (TICK_TIME / 2)
#define SIM_TIME_MAX (1000*10)
#define SIM_TIME_MAX_TICK (TICK_TIME * SIM_TIME_MAX)

int on_step_cb(uint64_t time, TOP_CLASS* p_top)
{
    if ((time % TICK_PERIOD) == 0)
    {
        p_top->i_clk = !p_top->i_clk;
    }
    return 0;
}

void sim_thread(TB* p_tb, GUIOutput* p_gui)
{
    p_tb->init(on_step_cb);
    TOP_CLASS* top = p_tb->get_top();

    p_gui->set_memory_pointer((uint8_t*)top->gui->u_orion_core->ram.m_storage);

    const char* cycles_str = p_tb->get_context()->commandArgsPlusMatch("cycles");
    int64_t cycles = -1;
    if (strlen(cycles_str) > 8)
    {
        cycles_str += 8;
        cycles = atoi(cycles_str);
        printf("Simulation limit: %" PRId64 " cycles\n", cycles);
    }

    // wait for reset
    top->i_reset_n = 0;
    p_tb->run_steps(20 * TICK_TIME);
    top->i_reset_n = 1;

    if (cycles > -1)
    {
        for (int i=0 ; i<cycles ; ++i)
        {
            p_tb->run_steps(TICK_TIME);
            p_gui->draw(0.f);
        }
    }
    else
    {
        const uint32_t tick_speed = (10 * 1000 * 1000); // 10MHZ
        const uint32_t cycle_len = 100; // 1/100 sec
        const uint32_t cycles_count = tick_speed / cycle_len;
        float sim_time;
        uint64_t cycle = 0;
        while (!p_gui->is_closed())
        {
            p_tb->run_steps(cycles_count * TICK_TIME);
            sim_time = (cycle * 1.f) / cycle_len;
            p_gui->draw(sim_time);
            ++cycle;
        }
    }

    p_tb->finish();
    top->final();
}

int main(int argc, char** argv, char** env)
{
    QApplication app(argc, argv);
    app.setApplicationName("Orion-PRO RTL simulation");
    GUIOutput gui_out;
    gui_out.init();
    TB* tb = new TB(TOP_NAME_STR, argc, argv);
    std::thread thr(sim_thread, tb, &gui_out);
    app.exec();
    thr.join();
    return 0;
}

#if 0
#include <memory>
#include <fstream>

uint32_t prev_marker;
bool initialized;

int main(int argc, char** argv, char** env)
{
    TB* tb = new TB(TOP_NAME_STR, argc, argv);
    tb->init(on_step_cb);
    TOP_CLASS* top = tb->get_top();
    initialized = false;

    const char* cycles_str = tb->get_context()->commandArgsPlusMatch("cycles");
    uint32_t cycles = (uint32_t)-1;
    if (strlen(cycles_str) > 8)
    {
        cycles_str += 8;
        cycles = atoi(cycles_str);
    }

    // wait for reset
    top->i_reset_n = 0;
    tb->run_steps(20 * TICK_TIME);
    top->i_reset_n = 1;
    initialized = true;

    int ret = -1;
    for (int i=0 ; i<cycles ; ++i)
    {
        ret = tb->run_steps(TICK_TIME);
        if (ret != 0)
        {
            break;
        }
    }
    if (cycles != (uint32_t)-1)
    {
        ret = 0;
    }

    // memory dump
    /*uint8_t* p_ram = (uint8_t*)top->gui->u_orion_core->ram.m_storage;
    for (int32_t addr=0xc000 ; addr<0xcfff ; addr++)
    {
        int32_t offset = addr << 1;
        uint8_t val = p_ram[offset];
        if ((addr % 16) == 0)
        {
            printf("\n0x%+4x: %02x", addr, val);
        }
        else
        {
            printf(" %02x", val);
        }
    }
    printf("\n");
    std::ofstream out_f("../dump.bin");
    out_f.write((char*)p_ram, 1024*1024);
    out_f.close();*/

    if (ret == 1)
    {
        ret = 0;
    }

    tb->finish();
    top->final();
#if VM_COVERAGE
    //tb->get_context()->coveragep()->write(COV_FN);
#endif
    return ret;
}
#endif