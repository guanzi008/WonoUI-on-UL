#include "window.h"
#include "ui_state.h"
#include "animation.h"
#include "display.h"
#include "eeprom_manager.h"
#include "pages.h"
#include "knob.h"

/************************************* 弹窗相关 *************************************/

void window_value_init(const char title[], uint8_t select, uint8_t *value, uint8_t max, uint8_t min, uint8_t step, Menu *bg, uint8_t index) {
    strcpy(win.title, title);
    win.select = select;
    win.value = value;
    win.max = max;
    win.min = min;
    win.step = step;
    win.bg = bg;
    win.index = index;
    ui.index = M_WINDOW;
    if (ui.param[WIN_BOK]) {
        ui.state = S_FADE;  // Win Bokeh Bg打开时，先进入 fade 动画
        ui.fade = 1;
    } else {
        // Win Bokeh Bg关闭时，直接初始化窗口并显示
        window_param_init();
    }
}

void window_param_init() {
    win.bar = 0;
    win.y = WIN_Y;
    win.y_trg = 50;
    win.l = 13;
    ui.state = S_NONE;
}

void window_show() {
    list_show(win.bg, win.index);
    if (ui.param[WIN_BOK]) {
        if (ui.param[DARK_MODE]) {
            // 第 1 步：横竖同时处理，形成棋盘格网格（暗色模式用 &）
            for (uint16_t y = 0; y < 128; ++y) {
                for (uint16_t x = 0; x < 16; ++x) {
                    if (y % 2 == 0) {
                        buf_ptr[y * 16 + x] = buf_ptr[y * 16 + x] & 0xAA;
                    } else {
                        buf_ptr[y * 16 + x] = buf_ptr[y * 16 + x] & 0x55;
                    }
                }
            }
        } else {
            // 第 1 步：横竖同时处理，形成棋盘格网格（亮色模式用 |）
            for (uint16_t y = 0; y < 128; ++y) {
                for (uint16_t x = 0; x < 16; ++x) {
                    if (y % 2 == 0) {
                        buf_ptr[y * 16 + x] = buf_ptr[y * 16 + x] | 0xAA;
                    } else {
                        buf_ptr[y * 16 + x] = buf_ptr[y * 16 + x] | 0x55;
                    }
                }
            }
        }
    }
    
    u8g2.setFont(WIN_FONT);
    win.bar_trg = (float)(*win.value - win.min) / (float)(win.max - win.min) * (WIN_BAR_W - 4);
    
    animation(&win.bar, &win.bar_trg, WIN_ANI);
    animation(&win.y, &win.y_trg, WIN_ANI);
    
    u8g2.setDrawColor(0);
    u8g2.drawRBox(win.l, (int16_t)win.y, WIN_W, WIN_H, 2);
    u8g2.setDrawColor(1);
    u8g2.drawRFrame(win.l, (int16_t)win.y, WIN_W, WIN_H, 2);
    u8g2.drawRFrame(win.l + 5, (int16_t)win.y + 20, WIN_BAR_W, WIN_BAR_H, 1);
    u8g2.drawBox(win.l + 7, (int16_t)win.y + 22, win.bar, WIN_BAR_H - 4);
    u8g2.setCursor(win.l + 5, (int16_t)win.y + 14);
    u8g2.print(win.title);
    u8g2.setCursor(win.l + 78, (int16_t)win.y + 14);
    u8g2.print(*win.value);
    
    if (!strcmp(win.title, "Disp Bri")) {
        u8g2.setContrast(ui.param[DISP_BRI]);
    }
    
    u8g2.setDrawColor(2);
    if (!ui.param[DARK_MODE]) {
        u8g2.drawBox(0, 0, DISP_W, DISP_H);
    }
}

void window_proc() {
    window_show();
    if (win.y == WIN_Y_TRG) {
        // 窗口已经退场，直接返回目标页面，不做 fade 动画
        ui.index = win.index;
        ui.state = S_NONE;
    }
    if (btn.pressed && win.y == win.y_trg && win.y != WIN_Y_TRG) {
        btn.pressed = false;
        switch (btn.id) {
            case BTN_ID_CW:
                if (*win.value < win.max) {
                    *win.value += win.step;
                    eeprom.change = true;
                }
                break;
            case BTN_ID_CC:
                if (*win.value > win.min) {
                    *win.value -= win.step;
                    eeprom.change = true;
                }
                break;
            case BTN_ID_SP:
            case BTN_ID_LP:
                win.y_trg = WIN_Y_TRG;
                buzzer_exit_sound();
                break;
        }
    }
}