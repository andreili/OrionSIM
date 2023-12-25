#include <QApplication>
#include "gui_output.hpp"
#include "sim.hpp"

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    app.setApplicationName("Orion-PRO RTL simulation");
    GUIOutput gui_out;
    gui_out.init();
    SIM* p_sim = new SIM();
    p_sim->init(&gui_out, argc, argv);
    app.exec();
    p_sim->run();
    return 0;
}
