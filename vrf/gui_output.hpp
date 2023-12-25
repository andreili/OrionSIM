#pragma once

#include <QLabel>
#include <QPainter>
#include <QEvent>
#include <QCloseEvent>
#include <QKeyEvent>
#include <QImage>

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

    void set_memory_pointer(uint8_t* p_mem)
    {
        this->p_memory = p_mem;
    }

    void draw(float time)
    {
        QString str_time;
        str_time.asprintf("Sim time: %.2fs", time);
        printf("Sim time: %.2fs\n", time);

        QImage background_image(this->width, this->height, QImage::Format_RGB32);
        background_image.fill(0xffffffff);

        QPainter painter(&background_image);

        for (int x=0 ; x<this->width ; x+=8)
        {
            uintptr_t line_addr = 0xc000 + ((x >> 3) << 8);
            for (int y=0 ; y<this->height ; ++y)
            {
                uintptr_t col_addr = line_addr | y;
                uint8_t pixels = p_memory[col_addr];
                for (int i=0 ; i<8 ; ++i)
                {
                    if (pixels & 1)
                    {
                        this->p_image->setPixel(x+i, y, 0x0000ff00);
                    }
                    else
                    {
                        this->p_image->setPixel(x+i, y, 0);
                    }
                    pixels >>= 1;
                }
            }
        }
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
    QImage* p_image;
};
