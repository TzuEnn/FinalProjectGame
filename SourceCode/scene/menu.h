#ifndef MENU_H_INCLUDED                 // 如果還沒定義 MENU_H_INCLUDED，就定義它（避免重複 include）
#define MENU_H_INCLUDED

#include <allegro5/allegro_audio.h>        // 引入 Allegro 的音效模組
#include <allegro5/allegro_acodec.h>       // 引入音效格式支援（例如 ogg / wav）
#include <allegro5/allegro_font.h>         // 引入字體模組（用來顯示文字）
#include "scene.h"                         // 引入 Scene 結構與函式（選單是屬於一種 Scene）

/*
   [Menu object] 用來描述選單畫面的資料結構
*/
typedef struct _Menu {
    ALLEGRO_FONT *title_font;              // 標題用的字體（較大）
    ALLEGRO_FONT *font;                    // 選單項目用的字體（較小）
    ALLEGRO_SAMPLE *song;                  // 背景音樂（未播放的原始音效物件）
    ALLEGRO_SAMPLE_INSTANCE *sample_instance; // 背景音樂的播放實例，可控制音量、循環等
    ALLEGRO_BITMAP *background_img;        // 背景圖片，用來美化選單畫面
    ALLEGRO_SAMPLE* select_sound;          // 移動選單時的音效（上下鍵）
    ALLEGRO_SAMPLE* enter_sound;           // 按下確認時的音效（Enter 鍵）

    int title_x, title_y;                  // （預留用）標題的位置座標，可用於動畫或自訂位置
} Menu;

// 建立新的 Menu 場景
Scene *New_Menu(int label);

// 每幀更新選單邏輯（按鍵控制、狀態轉換等）
void menu_update(Scene *self);

// 負責畫出整個選單畫面（背景圖、選單文字等）
void menu_draw(Scene *self);

// 畫面結束時釋放 Menu 使用的所有資源
void menu_destroy(Scene *self);

#endif // MENU_H_INCLUDED