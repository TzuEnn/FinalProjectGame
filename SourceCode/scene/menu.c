#include <allegro5/allegro_primitives.h>   // 引入 Allegro 畫圖模組，可以畫線、方形、圓等基本圖形
#include <allegro5/allegro_audio.h>        // 引入音效播放模組，負責播放音樂和音效
#include <allegro5/allegro_acodec.h>       // 引入音樂格式支援模組，支援 ogg、wav 格式
#include <allegro5/allegro_font.h>         // 引入字體模組，用來顯示文字
#include <allegro5/allegro_ttf.h>          // 引入 TTF 字體模組，可以載入 .ttf 檔案的字型
#include "menu.h"                          // 引入自定義的 menu 標頭檔，宣告 Menu 結構與函式
#include "../global.h"                     // 引入全域變數設定，如 WIDTH、HEIGHT、window 狀態等
#include <stdbool.h>                       // 引入布林值支援，可以用 true/false 表示真假值

#define MENU_ITEM_COUNT 3   // 定義選單項目的數量為 3

const char *menu_items[] = {              // 宣告一個包含選單文字的陣列
    "Start Game",                         // 第一個選項：開始遊戲
    "How to Play",                        // 第二個選項：玩法說明
    "Exit"                                // 第三個選項：離開遊戲
};

const int menu_item_count = MENU_ITEM_COUNT; // 設定實際使用的選單數量，來自前面的 define
int menu_index = 0;                           // 當前被選中的選單項目的索引值（從 0 開始）

Scene *New_Menu(int label)                   // 建立新的 Menu 場景，傳入場景標籤（label）
{
    Menu *pDerivedObj = (Menu *)malloc(sizeof(Menu)); // 配置一塊記憶體來存放 Menu 結構
    Scene *pObj = New_Scene(label);                   // 呼叫共通的場景建立函式，回傳一個 Scene 指標

    int title_font_size = WIDTH / 20;                 // 設定標題字體大小，依據畫面寬度調整
    int menu_font_size = WIDTH / 40;                  // 設定選單字體大小，依據畫面寬度調整
    pDerivedObj->title_font = al_load_ttf_font("assets/font/pixel.ttf", title_font_size, 0); // 載入標題字體
    pDerivedObj->font = al_load_ttf_font("assets/font/pixel.ttf", menu_font_size, 0);        // 載入選單字體

    pDerivedObj->background_img = al_load_bitmap("assets/image/title_bg.png");               // 載入背景圖片

    pDerivedObj->song = al_load_sample("assets/sound/battle_ship.ogg");                      // 載入背景音樂檔
    al_reserve_samples(20);                                                                  // 預留最多可播放的音效數量
    pDerivedObj->sample_instance = al_create_sample_instance(pDerivedObj->song);             // 建立音樂實例（可以控制播放方式）

    pDerivedObj->select_sound = al_load_sample("assets/sound/select.wav");                  // 載入選擇選單音效
    pDerivedObj->enter_sound = al_load_sample("assets/sound/enter.ogg");                    // 載入按下確認音效

    al_set_sample_instance_playmode(pDerivedObj->sample_instance, ALLEGRO_PLAYMODE_LOOP);    // 設定背景音樂為循環播放
    al_restore_default_mixer();                                                              // 重設音效混音器為預設
    al_attach_sample_instance_to_mixer(pDerivedObj->sample_instance, al_get_default_mixer()); // 將音樂實例掛到混音器上
    al_set_sample_instance_gain(pDerivedObj->sample_instance, 0.1);                          // 設定背景音樂音量（0.0 ~ 1.0）

    pDerivedObj->title_x = WIDTH / 2;          // 標題的 X 位置設為畫面中間
    pDerivedObj->title_y = -100;               // 標題的 Y 起始位置在畫面外上方（實作滑入效果）

    pObj->pDerivedObj = pDerivedObj;         // 把 Menu 資料物件掛到 Scene 裡面
    pObj->Update = menu_update;              // 設定這個場景的更新邏輯函式
    pObj->Draw = menu_draw;                  // 設定這個場景的畫圖邏輯函式
    pObj->Destroy = menu_destroy;            // 設定這個場景的銷毀邏輯函式

    return pObj;                             // 回傳場景物件
}

void menu_update(Scene *self)               // 選單畫面每幀更新時會執行的邏輯
{
    Menu *Obj = (Menu *)(self->pDerivedObj); // 取得這個場景裡的 Menu 資料

    // 標題滑入動畫：從畫面頂端滑到指定位置
    if (Obj->title_y < HEIGHT / 5) {
        Obj->title_y += 5; // 每幀往下移動一點（可以調整數字改變速度）
        if (Obj->title_y > HEIGHT / 5)
            Obj->title_y = HEIGHT / 5; // 超過就停住
    }

    if (key_state[ALLEGRO_KEY_DOWN]) {
        key_state[ALLEGRO_KEY_DOWN] = false;
        menu_index = (menu_index + 1) % menu_item_count;
        al_play_sample(Obj->select_sound, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
    }

    if (key_state[ALLEGRO_KEY_UP]) {
        key_state[ALLEGRO_KEY_UP] = false;
        menu_index = (menu_index - 1 + menu_item_count) % menu_item_count;
        al_play_sample(Obj->select_sound, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
    }

    if (key_state[ALLEGRO_KEY_ENTER]) {
        key_state[ALLEGRO_KEY_ENTER] = false;

        al_play_sample(Obj->enter_sound, 2.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
        al_rest(0.5);

        if (menu_index == 0) window = 1;
        else if (menu_index == 1) window = 2;
        else if (menu_index == 2) window = -1;

        self->scene_end = true;
    }
}

void menu_draw(Scene *self) {
    Menu *Obj = (Menu *)(self->pDerivedObj);
    al_clear_to_color(al_map_rgb(0, 0, 0));

    if (Obj->background_img) {
        al_draw_scaled_bitmap(Obj->background_img,
                              0, 0,
                              al_get_bitmap_width(Obj->background_img),
                              al_get_bitmap_height(Obj->background_img),
                              0, 0,
                              WIDTH, HEIGHT, 0);
    }

    const char *title = "Battle Ship++";
    al_draw_text(Obj->title_font, al_map_rgb(255, 255, 255),
                 Obj->title_x, Obj->title_y,
                 ALLEGRO_ALIGN_CENTRE, title);

    int line_height = al_get_font_line_height(Obj->font) + 10;

    for (int i = 0; i < menu_item_count; i++) {
        ALLEGRO_COLOR color = (i == menu_index) ?
                              al_map_rgb(255, 255, 0) :
                              al_map_rgb(255, 255, 255);
        al_draw_text(Obj->font, color,
                     WIDTH / 2, HEIGHT / 2 + i * line_height,
                     ALLEGRO_ALIGN_CENTRE, menu_items[i]);
    }

    al_play_sample_instance(Obj->sample_instance);
}

void menu_destroy(Scene *self)
{
    Menu *Obj = ((Menu *)(self->pDerivedObj));
    al_destroy_font(Obj->font);
    al_destroy_font(Obj->title_font);
    al_destroy_sample(Obj->song);
    al_destroy_sample_instance(Obj->sample_instance);

    al_destroy_sample(Obj->select_sound);
    al_destroy_sample(Obj->enter_sound);

    free(Obj);
    free(self);
}
