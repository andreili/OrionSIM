#pragma once

#include <QLabel>
#include <QPainter>
#include <QEvent>
#include <QCloseEvent>
#include <QKeyEvent>
#include <QImage>

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
    void init()
    {
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

    void draw_mem(std::function<void(int,int,uint8_t,uint8_t,uint8_t,uint8_t)> set_cb)
    {
        uint32_t scr_no;
        if (p_f8->bt.mode & (1 << 4))
        {
            scr_no = p_fa->bt.scr_no | 1;
        }
        else
        {
            scr_no = p_fa->bt.scr_no;
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
        for (int x=0 ; x<this->width ; x+=8)
        {
            uintptr_t line_addr = 0xc000 + ((x >> 3) << 8);
            for (int y=0 ; y<this->height ; ++y)
            {
                uintptr_t col_addr = line_addr | y;
                uint8_t pixels = this->p_memory[col_addr];
                for (int i=0 ; i<8 ; ++i)
                {
                    if (pixels & (1 << 7))
                    {
                        set_cb(x+i, y, 0, 0xff, 0, 0);
                    }
                    else
                    {
                        set_cb(x+i, y, 0, 0, 0, 0);
                    }
                    pixels <<= 1;
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
        draw_mem([this](int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t i)
            {
                uint32_t pxl = (r << 16) | (g << 8) | b;
                this->p_image->setPixel(x, y, pxl);
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
        //
    }

    void keyReleaseEvent(QKeyEvent *evt)
    {
        //
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
};
