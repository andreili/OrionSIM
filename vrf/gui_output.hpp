#pragma once

#include <QLabel>
#include <QPainter>
#include <QEvent>
#include <QCloseEvent>
#include <QImage>
#include "kbd.hpp"

typedef union
{
    uint8_t b;
    struct
    {
        uint8_t mode : 5;
        uint8_t : 3;
    } bt;
} port_f8_u;

typedef union
{
    uint8_t b;
    struct
    {
        uint8_t scr_no : 2;
        uint8_t : 4;
        uint8_t rfsh_off : 1;
        uint8_t scr_wide : 1;
    } bt;
} port_fa_u;

typedef union
{
    uint8_t b;
    struct
    {
        uint8_t pen : 4;
        uint8_t back: 4;
    } bt;
} port_fc_u;

class GUIOutput : public QLabel
{
public:
    void init(KBD* p_kbd)
    {
        this->p_kbd = p_kbd;
        this->close = false;
        //this->input_bits = 0;
        this->width = 512;
        this->height = 256;
        setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
        this->setFixedSize(this->width, this->height);
        this->p_image = new QImage(this->width, this->height, QImage::Format_RGB32);
    }

    int get_width() { return this->width; }
    void set_width(int width)
    {
        this->width = width;
        this->setFixedSize(this->width, this->height);
        //this->p_image->resize(this->width, this->height);
    }

    void set_memory_pointer(uint8_t* p_mem, uint8_t* p_f8, uint8_t* p_fa, uint8_t* p_fc)
    {
        this->p_memory = p_mem;
        this->p_f8 = (port_f8_u*)p_f8;
        this->p_fa = (port_fa_u*)p_fa;
        this->p_fc = (port_fc_u*)p_fc;
    }

    #define RGB(b,g,r) (((r) << 16) | ((g) << 8) | (b))

    void draw_mem(std::function<void(int,int,uint32_t)> set_cb)
    {
        if ((this->p_f8->bt.mode == 2) || (this->p_f8->bt.mode == 3))
        {
            return;
        }
        uint32_t scr_no;
        if (this->p_f8->bt.mode & (1 << 4))
        {
            scr_no = this->p_fa->bt.scr_no | 1;
        }
        else
        {
            scr_no = this->p_fa->bt.scr_no;
        }
        uintptr_t scr_start_addr;
        switch (scr_no)
        {
        case 0: scr_start_addr = 0x0c000;
                break;
        case 1: scr_start_addr = 0x08000;
                break;
        case 2: scr_start_addr = 0x04000;
                break;
        case 3: scr_start_addr = 0x00000;
                break;
        }
        uintptr_t scr_plane_0, scr_plane_1, scr_plane_2, scr_plane_3;
        scr_plane_0 = scr_start_addr;
        scr_plane_1 = scr_start_addr + 0x04000;
        scr_plane_2 = scr_start_addr + 0x10000;
        scr_plane_3 = scr_start_addr + 0x14000;
        uint32_t c0, c1, c2, c3;
        switch (this->p_f8->bt.mode)
        {
        case 0: c0 = RGB(0x00, 0x00, 0x00); c1 = RGB(0x00, 0xff, 0x00);
                break;
        case 1: c0 = RGB(0xc8, 0xb4, 0x28); c1 = RGB(0x32, 0xfa, 0xfa);
                break;
        case 4: c0 = RGB(0x00, 0x00, 0x00); c1 = RGB(0x00, 0x00, 0xc0); c2 = RGB(0x00, 0xc0, 0x00); c3 = RGB(0xc0, 0x00, 0x00);
                break;
        case 5: c0 = RGB(0xc0, 0xc0, 0xc0); c1 = RGB(0x00, 0x00, 0xc0); c2 = RGB(0x00, 0xc0, 0x00); c3 = RGB(0xc0, 0x00, 0x00);
                break;
        }
        for (int x=0 ; x<this->width ; x+=8)
        {
            for (int y=0 ; y<this->height ; ++y)
            {
                uint8_t pixels0 = this->p_memory[(scr_plane_0 + ((x >> 3) << 8)) | y];
                uint8_t pixels1 = this->p_memory[(scr_plane_1 + ((x >> 3) << 8)) | y];
                uint8_t pixels2 = this->p_memory[(scr_plane_2 + ((x >> 3) << 8)) | y];
                uint8_t pixels3 = this->p_memory[(scr_plane_3 + ((x >> 3) << 8)) | y];
                if ((this->p_f8->bt.mode == 14) || (this->p_f8->bt.mode == 15))
                {
                    pixels2 = this->p_fc->b;
                }
                if ((this->p_f8->bt.mode == 6) || (this->p_f8->bt.mode == 7) ||
                    (this->p_f8->bt.mode == 14) || (this->p_f8->bt.mode == 15))
                {
                    uint32_t ci, cr, cg, cb;
                    ci = (pixels2 & (1 << 7)) >> 1;
                    cr = (pixels2 & (1 << 6)) ? (ci | 0xbf) : 0x00;
                    cg = (pixels2 & (1 << 5)) ? (ci | 0xbf) : 0x00;
                    cb = (pixels2 & (1 << 4)) ? (ci | 0xbf) : 0x00;
                    c0 = RGB(cb, cg, cr);
                    ci = (pixels2 & (1 << 3)) << 3;
                    cr = (pixels2 & (1 << 2)) ? (ci | 0xbf) : 0x00;
                    cg = (pixels2 & (1 << 1)) ? (ci | 0xbf) : 0x00;
                    cb = (pixels2 & (1 << 0)) ? (ci | 0xbf) : 0x00;
                    c1 = RGB(cb, cg, cr);
                }
                for (int i=0 ; i<8 ; ++i)
                {
                    uint8_t pxl0 = (pixels0 >> 7);
                    uint8_t pxl1 = (pixels1 >> 7);
                    uint8_t pxl2 = (pixels2 >> 7);
                    uint8_t pxl3 = (pixels3 >> 7);
                    switch (this->p_f8->bt.mode)
                    {
                    case 0:
                    case 1:
                    case 6:
                    case 7:
                    case 14:
                    case 15:
                        if (pxl0)
                        {
                            set_cb(x+i, y, c1);
                        }
                        else
                        {
                            set_cb(x+i, y, c0);
                        }
                        break;
                    case 4:
                    case 5:
                        switch ((pxl0 << 1) | pxl2)
                        {
                        case 0: set_cb(x+i, y, c0);
                                break;
                        case 1: set_cb(x+i, y, c1);
                                break;
                        case 2: set_cb(x+i, y, c2);
                                break;
                        default:set_cb(x+i, y, c3);
                                break;
                        }
                        break;
                    }
                    switch (this->p_f8->bt.mode & 0x14)
                    {
                    case 0x10:
                        c0 = 0;
                        if (pxl0) { c0 |= RGB(0x00, 0x00, 0xbf); }
                        if (pxl1) { c0 |= RGB(0x00, 0xbf, 0x00); }
                        if (pxl2) { c0 |= RGB(0xbf, 0x00, 0x00); }
                        set_cb(x+i, y, c0);
                        break;
                    case 0x14:
                        c0 = 0;
                        if (pxl0) { c0 |= RGB(0x00, 0x00, 0xbf); }
                        if (pxl1) { c0 |= RGB(0x00, 0xbf, 0x00); }
                        if (pxl2) { c0 |= RGB(0xbf, 0x00, 0x00); }
                        if (pxl3) { c0 |= RGB(0x40, 0x40, 0x40); }
                        set_cb(x+i, y, c0);
                        break;
                    }
                    pixels0 <<= 1;
                    pixels1 <<= 1;
                    pixels2 <<= 1;
                    pixels3 <<= 1;
                }
            }
        }
    }

    void draw(float time)
    {
        QString str_time;
        str_time.asprintf("Sim time: %.2fs", time);
        printf("Sim time: %.2fs\n", time);

        QImage background_image(this->width, this->height, QImage::Format_RGB32);
        background_image.fill(0xffffffff);

        QPainter painter(&background_image);
        draw_mem([this](int x, int y, uint32_t argb)
            {
                this->p_image->setPixel(x, y, argb);
            });
        painter.drawImage(0, 0, *this->p_image);

        painter.setPen(QColorConstants::Red);
        painter.drawText(0, 16, str_time);
        painter.end();

        this->setPixmap(QPixmap::fromImage(background_image));
        this->show();
    }

    void keyPressEvent(QKeyEvent *evt)
    {
        this->p_kbd->add_event(evt);
    }

    void keyReleaseEvent(QKeyEvent *evt)
    {
        this->p_kbd->add_event(evt);
    }
    
    void closeEvent(QCloseEvent *evt)
    {
        this->close = true;
        evt->accept();
    }

    bool is_closed() { return close; }
private:
    bool close;
    int width;
    int height;
    uint8_t* p_memory;
    port_f8_u* p_f8;
    port_fa_u* p_fa;
    port_fc_u* p_fc;
    QImage* p_image;
    KBD* p_kbd;
};
