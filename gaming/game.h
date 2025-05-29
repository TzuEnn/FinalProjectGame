#pragma once
#include "ship.h"
#include "resource.h"

#define CELL_SIZE 48  // 放大格子尺寸
#define GRID_WIDTH 30 // 水平格子数
#define GRID_HEIGHT 15 // 垂直格子数
#define UI_WIDTH 1440
#define UI_HEIGHT 900
#define MAX_SHIPS 20
#define PLAYER_AREA_START_X 0
#define PLAYER_AREA_END_X 14
#define ENEMY_AREA_START_X 15
#define ENEMY_AREA_END_X 29
#define AREA_HEIGHT 30  // 整個區域的高度

// 按钮相关常量
#define BUTTON_WIDTH 100
#define BUTTON_HEIGHT 30
#define CONFIRM_BUTTON_X 50
#define CONFIRM_BUTTON_Y 500

typedef enum {
    ATTACK_NONE,
    ATTACK_MISS,
    ATTACK_HIT,
    ATTACK_SUNK
} AttackResult;

typedef struct GameState{
    Ship player_fleet[MAX_SHIPS];//max_ships為20，暫定
    Ship enemy_fleet[MAX_SHIPS];
    int current_turn;
    bool is_player_turn;
    int selected_attack_type;  // 0=普通 1=輕巡 2=重巡
    bool radar_map[GRID_WIDTH][GRID_HEIGHT]; // 新增雷達覆蓋圖
    int factory_cooldown; // 兵工廠生產冷卻
    AttackResult attack_history[GRID_WIDTH][GRID_HEIGHT];//用來儲存下面三行的壯派，miss，hitt和sunk
    bool game_over; // 遊戲是否結束
    int winner; // 獲勝者
    // 新增：存儲各船隻的攻擊次數限制
    int light_cruiser_attacks; // 輕巡洋艦累積攻擊次數
    int heavy_cruiser_attacks; // 重巡洋艦累積攻擊次數
    bool setup_phase;       // 是否在佈置階段
    int selected_ship_index; // 當前選中的船艦索引
    bool setup_completed;   // 佈置是否完成
    bool show_confirm_button; // 新增：顯示確認按鈕標誌
    int light_cruiser_cooldown; // 輕巡洋艦冷卻時間
    int heavy_cruiser_cooldown; // 重巡洋艦冷卻時間
    int ui_offset_x; // UI區域偏移量
    Ship* selected_ship;
    bool dragging;
} GameState;


void init_game(GameState* game);
void draw_game(GameState* game, GameSprites* sprites);
void handle_input(GameState* game, int mouse_x, int mouse_y);
void handle_game_input(GameState* game, int grid_x, int grid_y);
void handle_key_input(GameState* game, int keycode);
bool check_victory(GameState* game);
void draw_ui(GameState* game);
void move_all_suicide_boats(GameState* game, bool is_player);
void execute_enemy_turn(GameState* game);    // 敵方回合
void rotate_ship(GameState* game);          // 船艦旋轉
Ship* get_selected_ship(GameState* game);   // 獲取當前選中船隻

Ship* select_ship_at(GameState* game, int grid_x, int grid_y);
void move_selected_ship(GameState* game, int new_x, int new_y);


// 新增需要的函數聲明
void setup_enemy_fleet(GameState* game);           // 設置敵方艦隊
void draw_grid(void);                              // 繪製網格
void draw_ship(Ship* ship, GameSprites* sprites, int offset_y); // 繪製船艦
void draw_attack_effects(GameState* game, GameSprites* sprites); // 繪製攻擊效果
void draw_radar_overlay(GameState* game);          // 繪製雷達覆蓋
void update_radar_effects(GameState* game);        // 更新雷達效果
void update_turn(GameState* game);                 // 更新回合
void handle_setup_input(GameState* game, int grid_x, int grid_y); // 處理佈置階段輸入
void handle_game_input(GameState* game, int grid_x, int grid_y);   // 處理遊戲階段輸入
void select_ship(GameState* game, int mouse_x, int mouse_y);       // 選擇船艦
void place_ship(GameState* game, int mouse_x, int mouse_y);        // 放置船艦

// 敵方AI相關函數
void enemy_normal_attack(GameState* game);         // 敵方普通攻擊
void enemy_light_cruiser_attack(GameState* game);  // 敵方輕巡攻擊
void enemy_heavy_cruiser_attack(GameState* game);  // 敵方重巡攻擊