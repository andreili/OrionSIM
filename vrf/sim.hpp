#pragma once

#include "gui_output.hpp"
#include "tb.h"
#include CONCAT5(V,TOP_NAME,_,TOP_NAME,.h)
#include CONCAT5(V,TOP_NAME,_,orion_pro_top,.h)

double sc_time_stamp() { return 0; }

#define TICK_TIME 2
#define TICK_PERIOD (TICK_TIME / 2)
#define SIM_TIME_MAX (1000*10)
#define SIM_TIME_MAX_TICK (TICK_TIME * SIM_TIME_MAX)

class SIM
{
public:
    SIM()
    {
        p_instance = this;
    }
    void init(GUIOutput* p_gui, KBD* p_kbd, int argc, char** argv)
    {
        this->p_gui = p_gui;
        this->p_kbd = p_kbd;
        TB* tb = new TB(TOP_NAME_STR, argc, argv);
        tb->init(on_step_cb);
        TOP_CLASS* top = tb->get_top();
        p_gui->set_memory_pointer((uint8_t*)top->TOP_NAME->u_orion_core->ram.m_storage,
                                  (uint8_t*)&top->TOP_NAME->u_orion_core->video_mode,
                                  (uint8_t*)&top->TOP_NAME->u_orion_core->screen_mode,
                                  (uint8_t*)&top->TOP_NAME->u_orion_core->colors_pseudo);
        p_kbd->set_memory_pointer((uint8_t*)&top->TOP_NAME->u_orion_core->kbd_input,
                                  (uint8_t*)&top->TOP_NAME->u_orion_core->kbd_output);
        p_thr = new std::thread(this->main, tb);
    }
    void run()
    {
        p_thr->join();
    }
private:
    static SIM*     p_instance;
    GUIOutput*      p_gui;
    KBD*            p_kbd;
    std::thread*    p_thr;

    static int on_step_cb(uint64_t time, TOP_CLASS* p_top)
    {
        static uint16_t prev_rows = 0;
        {
            // keyboard check
            uint32_t rows = p_top->TOP_NAME->u_orion_core->kbd_output;
            if (rows != prev_rows)
            {
                p_instance->p_kbd->proc();
            }
            prev_rows = rows;
        }
        if ((time % TICK_PERIOD) == 0)
        {
            p_top->i_clk = !p_top->i_clk;
        }
        return 0;
    }

    static void main(TB* p_tb)
    {
        TOP_CLASS* top = p_tb->get_top();

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
                p_instance->p_gui->draw(0.f);
            }
        }
        else
        {
            const uint32_t tick_speed = (10 * 1000 * 1000); // 10MHZ
            const uint32_t cycle_len = 100; // 1/100 sec
            const uint32_t cycles_count = tick_speed / cycle_len;
            float sim_time;
            uint64_t cycle = 0;
            time_t time_prev = time(0);
            while (!p_instance->p_gui->is_closed())
            {
                p_tb->run_steps(cycles_count * TICK_TIME);
                sim_time = (cycle * 1.f) / cycle_len;

                // check screen widht
                int width;
                if ((top->TOP_NAME->u_orion_core->screen_mode & (1 << 7)))
                {
                    width = 512;
                }
                else
                {
                    width = 384;
                }
                if (p_instance->p_gui->get_width() != width)
                {
                    printf("Set width %d->%d\n", p_instance->p_gui->get_width(), width);
                    p_instance->p_gui->set_width(width);
                }

                p_instance->p_gui->draw(sim_time);
                ++cycle;
                if ((cycle % cycle_len) == 0)
                {
                    time_t time_new = time(0);
                    time_t delta = time_new - time_prev;
                    time_prev = time_new;
                    printf("Sim time for 1 second: %ld\n", delta);
                }
            }
        }

        p_tb->finish();
        top->final();
    }
};

SIM* SIM::p_instance;
