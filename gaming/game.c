#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include "game.h"
#include "combat.h"
#include <stdio.h>
#include <stdbool.h>


void init_game(GameState* game) {
    // 初始化攻击历史和雷达地图
    for (int i = 0; i < GRID_WIDTH; i++) {
        for (int j = 0; j < GRID_HEIGHT; j++) {
            game->attack_history[i][j] = ATTACK_NONE;
            game->radar_map[i][j] = false;
        }
    }

    // 初始化玩家船艦位置（左半區）
    game->player_fleet[0] = create_ship(SHIP_FACTORY, 5, 5);
    game->player_fleet[1] = create_ship(SHIP_CARRIER, 3, 3);
    game->player_fleet[2] = create_ship(SHIP_CRUISER_LIGHT, 6, 4);
    game->player_fleet[3] = create_ship(SHIP_CRUISER_HEAVY, 10, 6);
    
    // 初始化其他玩家船只为无效状态
    for (int i = 4; i < MAX_SHIPS; i++) {
        game->player_fleet[i].health = 0;
        game->player_fleet[i].type = SHIP_FACTORY;
        game->player_fleet[i].pos_x = 0;
        game->player_fleet[i].pos_y = 0;
        game->player_fleet[i].size = 1;
        game->player_fleet[i].cooldown = 0;
        game->player_fleet[i].is_radar_active = false;
        game->player_fleet[i].can_move_in_radar = false;
        game->player_fleet[i].is_visible = true;
    }
    
    // 初始化敵方船艦位置（右半區）
    game->enemy_fleet[0] = create_ship(SHIP_FACTORY, 20, 5);
    game->enemy_fleet[1] = create_ship(SHIP_CARRIER, 18, 3);
    game->enemy_fleet[2] = create_ship(SHIP_CRUISER_LIGHT, 21, 4);
    game->enemy_fleet[3] = create_ship(SHIP_CRUISER_HEAVY, 25, 6);
    
    // 初始化其他敌方船只为无效状态
    for (int i = 4; i < MAX_SHIPS; i++) {
        game->enemy_fleet[i].health = 0;
        game->enemy_fleet[i].type = SHIP_FACTORY;
        game->enemy_fleet[i].pos_x = 0;
        game->enemy_fleet[i].pos_y = 0;
        game->enemy_fleet[i].size = 1;
        game->enemy_fleet[i].cooldown = 0;
        game->enemy_fleet[i].is_radar_active = false;
        game->enemy_fleet[i].can_move_in_radar = false;
        game->enemy_fleet[i].is_visible = true;
    }
    
    // 初始化游戏状态变量
    game->current_turn = 1;
    game->is_player_turn = true;
    game->selected_attack_type = 0;
    game->factory_cooldown = 0;
    game->game_over = false;
    game->winner = -1;
    game->light_cruiser_attacks = 0;
    game->heavy_cruiser_attacks = 0;
    // 设置UI偏移量
    game->ui_offset_x = GRID_WIDTH * CELL_SIZE;
    game->setup_phase = true;
    game->selected_ship_index = -1;
    game->setup_completed = false;
    game->light_cruiser_cooldown = 0;
    game->heavy_cruiser_cooldown = 0;

    game->selected_ship = NULL;
    game->dragging = false;

    game->confirm_button_x = 645;  // 讓按鈕寬度 150 水平置中 (1440 - 150) / 2
    game->confirm_button_y = 800; // 離底部 100 像素（你也可以調成更高或更低）900 - 100

}

void draw_grid(void) {
    for (int i = 0; i <= GRID_WIDTH; i++) {
        al_draw_line(i * CELL_SIZE, 0, i * CELL_SIZE, GRID_HEIGHT * CELL_SIZE,
                    al_map_rgb(100, 100, 100), 1);
    }
    for (int i = 0; i <= GRID_HEIGHT; i++) {
        al_draw_line(0, i * CELL_SIZE, GRID_WIDTH * CELL_SIZE, i * CELL_SIZE,
                    al_map_rgb(100, 100, 100), 1);
    }
    al_draw_line(15 * CELL_SIZE, 0, 15 * CELL_SIZE, GRID_HEIGHT * CELL_SIZE,
                al_map_rgb(200, 50, 50), 3);
}

// 繪製每個船艦
void draw_ship(Ship* ship, GameSprites* sprites, int offset_y) {
    if (ship->type == SHIP_SUICIDE_BOAT && !ship->is_visible && !ship->is_radar_active) {
        return; // 隱藏未偵測的自殺快艇
    }

    ALLEGRO_BITMAP* img = NULL;
    switch(ship->type) {
        case SHIP_CARRIER:        img = sprites->carrier; break;
        case SHIP_CRUISER_LIGHT:  img = sprites->cruiser_light; break;
        case SHIP_CRUISER_HEAVY:  img = sprites->cruiser_heavy; break;
        case SHIP_SUBMARINE:      img = sprites->submarine; break;
        case SHIP_FACTORY:        img = sprites->factory; break;
        case SHIP_RADAR:          img = sprites->radar; break;
        case SHIP_SUICIDE_BOAT:   img = sprites->suicide_boat; break;
    }

    if (!img) return;

    int draw_x = ship->pos_x * CELL_SIZE;
    int draw_y = ship->pos_y * CELL_SIZE + offset_y;

    if (ship->type == SHIP_FACTORY) {
        // 工廠圖為 3x2 格的 L 型，這裡縮放畫圖
        al_draw_scaled_bitmap(img, 0, 0,
            al_get_bitmap_width(img),
            al_get_bitmap_height(img),
            draw_x, draw_y,
            CELL_SIZE * 3,
            CELL_SIZE * 2, 0);
    } else {
        if (ship->is_horizontal) {
            al_draw_scaled_bitmap(img, 0, 0,
                al_get_bitmap_width(img),
                al_get_bitmap_height(img),
                draw_x, draw_y,
                CELL_SIZE * ship->size,
                CELL_SIZE, 0);
        } else {
            al_draw_rotated_bitmap(img,
                CELL_SIZE / 2, CELL_SIZE / 2,
                draw_x + CELL_SIZE / 2,
                draw_y + CELL_SIZE / 2,
                ALLEGRO_PI / 2, 0);
        }
    }

    // 顯示血量文字
    if (ship->health > 0) {
        al_draw_textf(al_create_builtin_font(), al_map_rgb(255, 255, 255),
            draw_x + 2, draw_y + 2, 0, "%d", ship->health);
    }
}


void draw_attack_effects(GameState* game, GameSprites* sprites) {
    for (int x = 0; x < GRID_WIDTH; x++) {
        for (int y = 0; y < GRID_HEIGHT; y++) {
            ALLEGRO_BITMAP* effect = NULL;
            switch(game->attack_history[x][y]) {
                case ATTACK_MISS:
                    effect = sprites->attack_miss;
                    break;
                case ATTACK_HIT:
                    effect = sprites->attack_hit;
                    break;
                case ATTACK_SUNK:
                    effect = sprites->attack_sunk;
                    break;
                default:
                    continue;
            }
            
            if (effect) {
                al_draw_scaled_bitmap(effect, 0, 0,
                    al_get_bitmap_width(effect), al_get_bitmap_height(effect),
                    x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE,
                    0);
            }
        }
    }
}

//檢測雷達覆蓋範圍
void draw_radar_overlay(GameState* game) {
    for (int x = 0; x < GRID_WIDTH; x++) {
        for (int y = 0; y < GRID_HEIGHT; y++) {
            if (game->radar_map[x][y]) {
                al_draw_filled_rectangle(
                    x * CELL_SIZE, y * CELL_SIZE,
                    (x + 1) * CELL_SIZE, (y + 1) * CELL_SIZE,
                    al_map_rgba(255, 255, 0, 50)
                );
            }
        }
    }
}


void draw_game(GameState* game, GameSprites* sprites, ALLEGRO_FONT* font) {
    al_clear_to_color(al_map_rgb(30, 30, 60));
    draw_grid();
    al_draw_line(0, 15 * CELL_SIZE, GRID_SIZE * CELL_SIZE, 15 * CELL_SIZE,
                al_map_rgb(200, 50, 50), 3);
    draw_radar_overlay(game);

    // 绘制舰队
    for (int i = 0; i < MAX_SHIPS; ++i) {
        if (game->player_fleet[i].health > 0) {
            draw_ship(&game->player_fleet[i], sprites, 0);
        }
        if (game->enemy_fleet[i].health > 0) {
            draw_ship(&game->enemy_fleet[i], sprites, 0);
        }
    }

    draw_attack_effects(game, sprites);
    draw_ui(game, font);
    al_flip_display();
}

void draw_ui(GameState* game, ALLEGRO_FONT* font) {
    if (!font) {
        printf("[錯誤] draw_ui 中 font 是 NULL，跳過繪製。\n");
        return;
    }

    int ui_x = GRID_WIDTH * CELL_SIZE + 20; // UI在網格右側
    int ui_y = 20;
    
    // 繪製UI背景
    al_draw_filled_rectangle(ui_x - 10, ui_y - 10, 
                            UI_WIDTH - 20, UI_HEIGHT - 20,
                            al_map_rgba(0, 0, 50, 200));
    
    // 顯示當前回合
    al_draw_textf(font, al_map_rgb(255, 255, 255),
                 ui_x, ui_y, 0, "Turn: %d", game->current_turn);
    
    // 顯示當前選擇的攻擊類型
    const char* attack_names[] = {"Normal/Carrier", "Light Cruiser", "Heavy Cruiser"};
    al_draw_textf(font, al_map_rgb(255, 255, 255),
                 ui_x, ui_y + 20, 0, "Attack: %s", attack_names[game->selected_attack_type]);
    
    // 顯示回合歸屬
    al_draw_textf(font, al_map_rgb(255, 255, 255),
                 ui_x, ui_y + 40, 0, "%s Turn", game->is_player_turn ? "Player" : "Enemy");
    
    // 顯示兵工廠冷卻
    al_draw_textf(font, al_map_rgb(255, 255, 255),
                 ui_x, ui_y + 60, 0, "Factory Cooldown: %d", game->factory_cooldown);
    
    // 顯示輕巡冷卻
    al_draw_textf(font, al_map_rgb(255, 255, 255),
                 ui_x, ui_y + 80, 0, "Light Cruiser CD: %d", game->light_cruiser_cooldown);
    
    // 顯示重巡冷卻
    al_draw_textf(font, al_map_rgb(255, 255, 255),
                 ui_x, ui_y + 100, 0, "Heavy Cruiser CD: %d", game->heavy_cruiser_cooldown);
    
    // 顯示累積攻擊次數
    al_draw_textf(font, al_map_rgb(255, 255, 255),
                 ui_x, ui_y + 120, 0, "Light Attacks: %d/2", game->light_cruiser_attacks);
    al_draw_textf(font, al_map_rgb(255, 255, 255),
                 ui_x, ui_y + 140, 0, "Heavy Attacks: %d/2", game->heavy_cruiser_attacks);
             
    // 繪製確認按鈕
    if (game->setup_phase && !game->setup_completed) {
        al_draw_filled_rectangle(
            game->confirm_button_x, game->confirm_button_y,
            game->confirm_button_x + 150, game->confirm_button_y + 50,
            al_map_rgb(0, 200, 0)
        );
        al_draw_text(font, al_map_rgb(0, 0, 0),
                     game->confirm_button_x + 75, game->confirm_button_y + 25,
                     ALLEGRO_ALIGN_CENTER, "CONFIRM");
    }
    // 顯示遊戲結束信息
    if (game->game_over) {
        const char* winner_text = (game->winner == 0) ? "Player Wins!" : "Enemy Wins!";
        al_draw_textf(font, al_map_rgb(255, 0, 0),
                     GRID_SIZE*CELL_SIZE/2 - 50, GRID_SIZE*CELL_SIZE/2, 0, 
                     "%s\nPress R to restart", winner_text);
    }
}
// 確認是否所有船隻都已經被部署（判斷 x, y 是否有設定）
bool check_all_ships_placed(GameState* game) {
    for (int i = 0; i < MAX_SHIPS; i++) {
        Ship* ship = &game->player_fleet[i];
        if (ship->health > 0 && (ship->pos_x < 0 || ship->pos_y < 0)) {
            return false;
        }
    }
    return true;
}

// 處理部署階段中的滑鼠點擊，包括按下確認鍵
void handle_setup_input(GameState* game, int mouse_x, int mouse_y) {
    // 先檢查是否點了確認鍵
    if (!game->dragging) {
        if (mouse_x >= game->confirm_button_x &&
            mouse_x <= game->confirm_button_x + 150 &&
            mouse_y >= game->confirm_button_y &&
            mouse_y <= game->confirm_button_y + 50) {
            
            if (check_all_ships_placed(game)) {
                game->setup_completed = true;
                printf("[提示] 所有船艦已部署完畢，遊戲開始！\n");
            } else {
                printf("[警告] 尚有船艦未部署完畢，請完成後再確認。\n");
            }
            return;
        }
    }

    // 玩家正在選擇船艦
    if (game->selected_ship_index == -1) {
        if (select_ship(game, mouse_x, mouse_y)) {
            // 成功選到船，等下一次點擊再決定放置
            printf("[提示] 選擇了船艦 %d\n", game->selected_ship_index);
        }
    } else {
        // 第二次點擊：放置船
        place_ship(game, mouse_x, mouse_y);
        game->selected_ship_index = -1;  // 放好後取消選擇
    }
}




void update_radar_effects(GameState* game) {
    // 重置雷達地圖
    for (int x = 0; x < GRID_WIDTH; x++) {
        for (int y = 0; y < GRID_HEIGHT; y++) {
            game->radar_map[x][y] = false;
        }
    }
    
    // 檢查所有活著的雷達船並應用效果
    Ship* all_fleets[2] = {game->player_fleet, game->enemy_fleet};
    for (int fleet_idx = 0; fleet_idx < 2; fleet_idx++) {
        Ship* fleet = all_fleets[fleet_idx];
        for (int i = 0; i < MAX_SHIPS; i++) {
            if (fleet[i].type == SHIP_RADAR && fleet[i].health > 0) {
                radar_ship_effect(game, &fleet[i]);
            }
        }
    }
}

// 在回合切換時調用
void update_turn(GameState* game) {
    // 潜艇被動攻擊
    submarine_attack(game, game->is_player_turn);
    
    // 按陣營觸發
    if(game->is_player_turn) {
        radar_ship_effect(game, game->player_fleet);
    } else {
        radar_ship_effect(game, game->enemy_fleet);
    }
    
    update_cooldowns(game->player_fleet);
    update_cooldowns(game->enemy_fleet);
    
    // 更新冷卻時間
    if (game->light_cruiser_cooldown > 0) {
        game->light_cruiser_cooldown--;
    }
    if (game->heavy_cruiser_cooldown > 0) {
        game->heavy_cruiser_cooldown--;
    }
    
    // 移動所有自殺快艇（被動技能）
    move_all_suicide_boats(game, game->is_player_turn);
    
    // 潛艇自動攻擊
    submarine_attack(game, game->is_player_turn);
    
    update_radar_effects(game);
}

// 自殺快艇移動函數（需要在combat.c中實現）
void move_all_suicide_boats(GameState* game, bool is_player) {
    Ship* fleet = is_player ? game->player_fleet : game->enemy_fleet;
    
    for (int i = 0; i < MAX_SHIPS; i++) {
        if (fleet[i].type == SHIP_SUICIDE_BOAT && fleet[i].health > 0) {
            // 這裡可以加入AI邏輯來決定移動方向
            // 目前先不移動，需要更複雜的邏輯
        }
    }
}

void handle_input(GameState* game, int mouse_x, int mouse_y) {
    if (game->game_over) return;
    
    int grid_x = mouse_x / CELL_SIZE;
    int grid_y = mouse_y / CELL_SIZE;
    
    // 检查坐标有效性
    if (grid_x < 0 || grid_x >= GRID_SIZE || grid_y < 0 || grid_y >= GRID_SIZE) {
        return;
    }
    
    // 佈置階段處理
    if (game->setup_phase) {
        if (game->selected_ship_index == -1) {
            // 選擇船艦
            select_ship(game, mouse_x, mouse_y);
        } else {
            // 放置船艦
            place_ship(game, mouse_x, mouse_y);
        }
        return;
    }
    
    if (game->is_player_turn) {
        // 检查是否只能攻擊敵方（x >= 15）
        if (grid_x < ENEMY_AREA_START_X) {
            return; // 不能攻擊己方
        }
        
        bool attack_executed = false;
        
        // 根据选择的攻击类型执行攻击
        switch(game->selected_attack_type) {
            case 0: // 普通攻击
                normal_attack(game, grid_x, grid_y);
                attack_executed = true;
                break;
            case 1: // 輕巡攻击（限制2次）
                if (game->light_cruiser_attacks < 2 && game->light_cruiser_cooldown == 0) {
                    light_cruiser_attack(game, grid_x, grid_y);
                    game->light_cruiser_attacks++;
                    game->light_cruiser_cooldown = 2; // 設置冷卻
                    attack_executed = true;
                }
                break;
            case 2: // 重巡攻击（限制2次）
                if (game->heavy_cruiser_attacks < 2 && game->heavy_cruiser_cooldown == 0) {
                    heavy_cruiser_attack(game, grid_x, grid_y, true);
                    game->heavy_cruiser_attacks++;
                    game->heavy_cruiser_cooldown = 3; // 設置冷卻
                    attack_executed = true;
                }
                break;
        }
        
        if (attack_executed) {
            // 玩家被动技能
            carrier_attack(game, true);
            move_all_suicide_boats(game, true);
    
            // 切换到敌方回合
            game->is_player_turn = false;
            update_turn(game);
    
            // 敌方完整回合
            if (!game->game_over) {
            // 敌方主动攻击
            execute_enemy_turn(game); // 新增函数封装
        
            // 敌方被动技能
            carrier_attack(game, false);
            move_all_suicide_boats(game, false);
        
            // 切换回玩家
            game->is_player_turn = true;
            game->current_turn++;
            update_turn(game);
            }
        }
    }
}

void handle_game_input(GameState* game, int grid_x, int grid_y) {
    // 检查坐标有效性
    if (grid_x < 0 || grid_x >= GRID_SIZE || grid_y < 0 || grid_y >= GRID_SIZE) {
        return;
    }
    
    if (game->is_player_turn) {
        // 检查是否只能攻击敌方区域
        if (grid_x < ENEMY_AREA_START_X) {
            return; // 不能攻击己方
        }
        
        bool attack_executed = false;
        
        // 根据选择的攻击类型执行攻击
        switch(game->selected_attack_type) {
            case 0: // 普通攻击
                normal_attack(game, grid_x, grid_y);
                attack_executed = true;
                break;
            case 1: // 轻巡攻击
                if (game->light_cruiser_attacks < 2 && game->light_cruiser_cooldown == 0) {
                    light_cruiser_attack(game, grid_x, grid_y);
                    game->light_cruiser_attacks++;
                    game->light_cruiser_cooldown = 2;
                    attack_executed = true;
                }
                break;
            case 2: // 重巡攻击
                if (game->heavy_cruiser_attacks < 2 && game->heavy_cruiser_cooldown == 0) {
                    heavy_cruiser_attack(game, grid_x, grid_y, true);
                    game->heavy_cruiser_attacks++;
                    game->heavy_cruiser_cooldown = 3;
                    attack_executed = true;
                }
                break;
        }
        
        if (attack_executed) {
            // 玩家被动技能
            carrier_attack(game, true);
            move_all_suicide_boats(game, true);
    
            // 切换到敌方回合
            game->is_player_turn = false;
            update_turn(game);
    
            // 敌方完整回合
            if (!game->game_over) {
                execute_enemy_turn(game);
                carrier_attack(game, false);
                move_all_suicide_boats(game, false);
            
                // 切换回玩家
                game->is_player_turn = true;
                game->current_turn++;
                update_turn(game);
            }
        }
    }
}

bool select_ship(GameState* game, int mouse_x, int mouse_y) {
    int grid_x = mouse_x / CELL_SIZE;
    int grid_y = mouse_y / CELL_SIZE;

    for (int i = 0; i < MAX_SHIPS; i++) {
        Ship* ship = &game->player_fleet[i];
        if (ship->health > 0) {
            int ship_end_x = ship->pos_x + (ship->is_horizontal ? ship->size : 1);
            int ship_end_y = ship->pos_y + (ship->is_horizontal ? 1 : ship->size);

            if (grid_x >= ship->pos_x && grid_x < ship_end_x &&
                grid_y >= ship->pos_y && grid_y < ship_end_y) {
                game->selected_ship_index = i;
                game->dragging = true; // 加上拖曳狀態
                return true;
            }
        }
    }

    return false;
}


void place_ship(GameState* game, int mouse_x, int mouse_y) {
    int grid_x = mouse_x / CELL_SIZE;
    int grid_y = mouse_y / CELL_SIZE;
    
    if (grid_x < PLAYER_AREA_START_X || grid_x > PLAYER_AREA_END_X) {
        return;
    }
    
    Ship* ship = &game->player_fleet[game->selected_ship_index];
    
    // Fixed: pass all required arguments
    if (is_position_valid_for_ship(game, grid_x, grid_y, 
                                  ship->size,
                                  true,   // is_player = true
                                  ship->type)) 
    {
        ship->pos_x = grid_x;
        ship->pos_y = grid_y;
    }
}

void execute_enemy_turn(GameState* game) {
    // 示例：敌方随机攻击+生产
    if (can_factory_produce(game, false)) {
        factory_production(game, false, rand()%3); 
    }
    
    // 随机选择攻击方式
    int attack_type = rand()%3;
    int x = rand()%GRID_SIZE;
    int y = rand()%(GRID_SIZE/2); // 攻击玩家区域
    
    switch(attack_type) {
        case 0: normal_attack(game, x, y); break;
        case 1: light_cruiser_attack(game, x, y); break;
        case 2: heavy_cruiser_attack(game, x, y, rand()%2); break;
    }
}

Ship* get_selected_ship(GameState* game) {
    // 实现你的船舰选择逻辑
    // 示例：返回第一个可操作的船舰
    for (int i = 0; i < MAX_SHIPS; i++) {
        if (game->player_fleet[i].health > 0) {
            return &game->player_fleet[i];
        }
    }
    return NULL;
}

void rotate_ship(GameState* game) {
    if (game->selected_ship_index != -1) {
        Ship* ship = &game->player_fleet[game->selected_ship_index];
        ship->is_horizontal = !ship->is_horizontal;
        
        // Fixed: pass all required arguments (size, is_player, type)
        if (!is_position_valid_for_ship(game, ship->pos_x, ship->pos_y, 
                                      ship->size, 
                                      true,   // is_player = true for player ships
                                      ship->type)) 
        {
            // Revert rotation if position invalid
            ship->is_horizontal = !ship->is_horizontal;
        }
    }
}

void handle_key_input(GameState* game, int keycode) {
    if (game->game_over) {
        if (keycode == ALLEGRO_KEY_R) {
            init_game(game); // 重新遊戲
        }
        return;
    }
    
    switch(keycode) {
        case ALLEGRO_KEY_1:
            game->selected_attack_type = 0; // 普通攻击
            break;
        case ALLEGRO_KEY_2:
            game->selected_attack_type = 1; // 轻巡攻击
            break;
        case ALLEGRO_KEY_3:
            game->selected_attack_type = 2; // 重巡攻击
            break;
        case ALLEGRO_KEY_F:
            // 兵工廠生產（簡化版，可以擴展）
            if (can_factory_produce(game, true)) {
                factory_production(game, true, 0); // 生產自殺快艇
            }
            break;
            case ALLEGRO_KEY_H:
                rotate_ship(game); // 船艦旋轉方向
                break;
    }
}

// 新增勝利條件檢查
bool check_victory(GameState* game) {
    bool player_wins = true;
    bool enemy_wins = true;
    
    // 检查敌方关键设施是否全部被击沉
    for (int i = 0; i < MAX_SHIPS; ++i) {
        Ship* s = &game->enemy_fleet[i];
        if ((s->type == SHIP_FACTORY || s->type == SHIP_CARRIER) && s->health > 0) {
            player_wins = false;
            break;
        }
    }
    
    // 检查玩家关键设施是否全部被击沉
    for (int i = 0; i < MAX_SHIPS; ++i) {
        Ship* s = &game->player_fleet[i];
        if ((s->type == SHIP_FACTORY || s->type == SHIP_CARRIER) && s->health > 0) {
            enemy_wins = false;
            break;
        }
    }
    
    // 更新游戏状态
    if (player_wins || enemy_wins) {
        game->game_over = true;
        game->winner = player_wins ? 0 : 1;
        return true;
    }
    return false;
}

// 根據玩家點擊的格子座標，尋找那一艘船被點到
Ship* select_ship_at(GameState* game, int grid_x, int grid_y) {
    for (int i = 0; i < MAX_SHIPS; i++) {
        Ship* ship = &game->player_fleet[i];
        if (ship->health <= 0) continue;

        if (ship->type == SHIP_FACTORY) {
            // 工廠是 L 型（假設佔這些格子：pos_x,pos_y / +1,0 / +2,0 / 0,+1）
            int x = ship->pos_x;
            int y = ship->pos_y;

            if ((grid_x == x && grid_y == y) ||
                (grid_x == x + 1 && grid_y == y) ||
                (grid_x == x + 2 && grid_y == y) ||
                (grid_x == x && grid_y == y + 1)) {
                return ship;
            }
        } else {
            int end_x = ship->pos_x + (ship->is_horizontal ? ship->size : 1);
            int end_y = ship->pos_y + (ship->is_horizontal ? 1 : ship->size);

            if (grid_x >= ship->pos_x && grid_x < end_x &&
                grid_y >= ship->pos_y && grid_y < end_y) {
                return ship;
            }
        }
    }
    return NULL;
}


// 拖曳選中船艦到新位置
void move_selected_ship(GameState* game, int new_x, int new_y) {
    if (!game->selected_ship) return;

    // 檢查是否落在合法區域（只允許拖曳到玩家的左半邊）
    if (new_x < PLAYER_AREA_START_X || new_x > PLAYER_AREA_END_X ||
        new_y < 0 || new_y >= GRID_HEIGHT) return;

    // 假設我們不檢查是否重疊、也不阻止覆蓋（可自行加入更複雜的邏輯）
    game->selected_ship->pos_x = new_x;
    game->selected_ship->pos_y = new_y;
}
