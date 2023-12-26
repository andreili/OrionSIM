#include <QApplication>
#include "sim.hpp"

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    app.setApplicationName("Orion-PRO RTL simulation");

    GUIOutput* p_gui_out = new GUIOutput();
    KBD* p_kbd = new KBD();
    SIM* p_sim = new SIM();
    p_gui_out->init(p_kbd);
    p_sim->init(p_gui_out, p_kbd, argc, argv);

    app.exec();
    p_sim->run();
    return 0;
}
