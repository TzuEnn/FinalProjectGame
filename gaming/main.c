#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include "game.h"
#include "resource.h"
#include <stdarg.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>


#define FPS 60

// 函数原型声明（统一使用 void）
void init_log(void);
void log_message(const char* format, ...);
void close_log(void);
void pause_before_exit(void);




// 全局变量
ALLEGRO_DISPLAY* display = NULL;
ALLEGRO_EVENT_QUEUE* event_queue = NULL;
ALLEGRO_FONT* font = NULL;  // 全域宣告但不呼叫函式
static FILE* log_file = NULL;

// 日誌功能实现
void init_log(void) {
    log_file = fopen("game_errors.log", "a");
    if (!log_file) {
        fprintf(stderr, "無法開啟日誌文件！\n");
        return;
    }
    time_t now = time(NULL);
    fprintf(log_file, "\n=== 遊戲啟動於: %s===\n", ctime(&now));
    fflush(log_file);
}

void log_message(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    if (log_file) {
        vfprintf(log_file, format, args);
        fflush(log_file);
    }
    va_end(args);
}

void close_log(void) {
    if (log_file) {
        fclose(log_file);
        log_file = NULL;
    }
}

void pause_before_exit(void) {
    #ifdef _WIN32
        system("pause");
    #else
        printf("Press Enter to exit...\n");
        getchar();
    #endif
}

int main() {
    // 初始化Allegro核心庫
    if (!al_init()) {
        log_message("[錯誤] 初始化 Allegro 失敗！\n");
        pause_before_exit();
        close_log();
        return -1;
    }

    al_init_font_addon(); // 初始化內建字型系統
    al_init_ttf_addon();  // 初始化 TTF 字型支援
    font = al_load_ttf_font("assets/font/pixel.ttf", 20, 0);
    if (!font) {
        fprintf(stderr, "無法載入字型：assets/font/pixel.ttf\n");
        return -1;
    }


    // 初始化插件和安裝設備
    al_install_keyboard();  // 先安裝鍵盤
    al_install_mouse();     // 安裝滑鼠
    if (!al_init_image_addon()) {  // 初始化圖像插件
        log_message("[錯誤] 初始化圖像插件失敗！\n");
        pause_before_exit();
        close_log();
        return -1;
    }
    al_init_primitives_addon();

    // 創建1440x900的窗口
    display = al_create_display(UI_WIDTH, UI_HEIGHT);
    if (!display) {
        log_message("[錯誤] 無法創建顯示器！\n");
        pause_before_exit();
        close_log();
        return -1;
    }

    // 創建事件隊列
    event_queue = al_create_event_queue();
    if (!event_queue) {
        log_message("[錯誤] 無法創建事件隊列！\n");
        al_destroy_display(display);  // 清理已創建的顯示器
        pause_before_exit();
        close_log();
        return -1;
    }

    // 註冊事件源
    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_mouse_event_source());
    al_register_event_source(event_queue, al_get_keyboard_event_source());

    GameSprites sprites;
    load_resources(&sprites);//加載那些戰艦
    
    GameState game;
    init_game(&game);

    bool redraw = true;
    ALLEGRO_TIMER* timer = al_create_timer(1.0 / FPS);//控制幀率
    al_register_event_source(event_queue, al_get_timer_event_source(timer));
    al_start_timer(timer);

   
    while (1) {
        ALLEGRO_EVENT ev;
        al_wait_for_event(event_queue, &ev);

        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
            break;

        if (ev.type == ALLEGRO_EVENT_TIMER) {
            redraw = true;
        }
        else if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
            int grid_x = ev.mouse.x / CELL_SIZE;
            int grid_y = ev.mouse.y / CELL_SIZE;

            if (game.setup_phase && !game.setup_completed) {
                // 點擊時，選取船艦並準備拖曳
                game.selected_ship = select_ship_at(&game, grid_x, grid_y); // 你要實作這個
                if (game.selected_ship) {
                    game.dragging = true;
                }
                handle_setup_input(&game, grid_x, grid_y);
                // ✅ 檢查是否全部船艦都已部署完畢，設為完成
                if (check_all_ships_placed(&game)) {
                    game.setup_completed = true;
                    printf("[提示] 所有船艦已部署完成，遊戲開始！\n");
                }
            } else {
                handle_game_input(&game, grid_x, grid_y);
            }
            redraw = true;
        }
        else if (ev.type == ALLEGRO_EVENT_MOUSE_AXES) {
            if (game.dragging && game.selected_ship != NULL) {
                int new_x = ev.mouse.x / CELL_SIZE;
                int new_y = ev.mouse.y / CELL_SIZE;
                move_selected_ship(&game, new_x, new_y);  // 實作這個函數
                redraw = true;
            }
        }
        else if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
            game.dragging = false;
            game.selected_ship = NULL;
        }
        else if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
            handle_key_input(&game, ev.keyboard.keycode);
            redraw = true;
        }

        if (redraw && al_is_event_queue_empty(event_queue)) {
            draw_game(&game, &sprites, font);
            redraw = false;
        }
    }

    // 資源釋放應在循環外執行
    destroy_resources(&sprites);
    al_destroy_timer(timer);
    al_destroy_display(display);
    al_destroy_event_queue(event_queue);
    pause_before_exit();
    al_destroy_font(font);
    return 0;
}