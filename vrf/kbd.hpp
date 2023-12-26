#pragma once

#include <QKeyEvent>
#include <QEvent>
#include <deque>
#include <set>

int key_matrix[8][11] = {
/*r D0*/  { '*',                      Qt::Key_Escape,           '+',                      Qt::Key_F1,               Qt::Key_F2,     Qt::Key_F3,        '4',           Qt::Key_F4,    Qt::Key_F5, '7',          '8' },
/*e D1*/  { Qt::Key_Minus,            Qt::Key_Tab,              'J' ,                     '1',                      '2',            '3',               'E',           '5',           '6',        '[',          ']' },
/*s D2*/  { 0,                        Qt::Key_CapsLock,         'F' ,                     'C',                      'U',            'K',               'P',           'N',           'G',        'L',          'D' },
/*u D3*/  { 0,                        0,                        'Q' ,                     'Y',                      'W',            'A',               'I',           'R',           'O',        'B',          0   },
/*l D4*/  { Qt::Key_Shift,            Qt::Key_Control,          0,                        0xDE,                     'S',            'M',               ' ',           'T',           'X',        Qt::Key_Left, '<' },
/*t D5*/  { '7' | Qt::KeypadModifier, '0' | Qt::KeypadModifier, '1' | Qt::KeypadModifier, '4' | Qt::KeypadModifier, Qt::Key_Plus,   Qt::Key_Backspace, Qt::Key_Right,  Qt::Key_Down, '>',        '\\',         'V' },
/*  D6*/  { '8' | Qt::KeypadModifier, '.',                      '2' | Qt::KeypadModifier, '5' | Qt::KeypadModifier, Qt::Key_F6,     Qt::Key_Home,      Qt::Key_Return, Qt::Key_Up,   '/',        'H',          'Z' },
/*  D7*/  { '9' | Qt::KeypadModifier, Qt::Key_Return,           '3' | Qt::KeypadModifier, '6' | Qt::KeypadModifier, Qt::Key_Insert, Qt::Key_End,       ':',            '?',          '-',        '0',          '9' }};
/*scancode  D0                        D1                        D2                        D3                        D4              D5                 D6              D7            CD0         CD1           CD2 */

typedef union
{
    uint32_t dw;
    struct
    {
        uint8_t PA;
        uint8_t PB;
        uint8_t PC;
        uint8_t res;
    } bt;
} kbd_port_u;

class KBD
{
public:
    void set_memory_pointer(uint8_t* p_input, uint8_t* p_output)
    {
        this->p_input  = (kbd_port_u*)p_input;
        this->p_output = (kbd_port_u*)p_output;
        this->p_input->dw = 0xffffffff;
    }
    void add_event(QKeyEvent *event)
    {
        if (event->type() == QEvent::Type::KeyPress)
        {
            this->pressed.insert(event->key());
        }
        if (event->type() == QEvent::Type::KeyRelease)
        {
            this->pressed.erase(event->key());
        }
        printf("Key event: 0x%x\n", event->key());
    }
    void proc()
    {
        uint32_t scancode_msk = ((this->p_output->bt.PC & 0x7) << 8) | this->p_output->bt.PB;
        uint32_t result = 0xff;
        for (int j=0 ; j<10 ; ++j)
        {
            if ((scancode_msk & (1 << j)) == 0)
            {
                for (int i=0 ; i<7 ; ++i)
                {
                    int key = key_matrix[i][j];
                    if (this->pressed.count(key) != 0)
                    {
                        result &= ~(1 << i);
                    }
                }
            }
        }
        this->p_input->bt.PA = result;
    }
private:
    kbd_port_u* p_input;
    kbd_port_u* p_output;
    std::set<uint32_t> pressed;
};
