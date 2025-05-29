#include <stdlib.h>
#include <time.h>
#include "combat.h"
#include "game.h"

void carrier_attack(GameState* game, bool is_player) {
    Ship* fleet = is_player ? game->player_fleet : game->enemy_fleet;
    Ship* target_fleet = is_player ? game->enemy_fleet : game->player_fleet;

    for (int i = 0; i < MAX_SHIPS; ++i) {
        if (fleet[i].type == SHIP_CARRIER && fleet[i].health > 0) {
            // 航母每回合自動攻擊，無需冷卻
            
            // 找到所有未被攻擊的格子
            int available_targets[GRID_SIZE * GRID_SIZE][2];
            int target_count = 0;
            int target_area_start = is_player ? ENEMY_AREA_START_X : PLAYER_AREA_START_X;
            int target_area_end = is_player ? ENEMY_AREA_END_X : PLAYER_AREA_END_X;
            
            for (int x = 0; x < GRID_SIZE; x++) {
                for (int y = target_area_start; y < target_area_end; y++) {
                    if (game->attack_history[x][y] == ATTACK_NONE) {
                        available_targets[target_count][0] = x;
                        available_targets[target_count][1] = y;
                        target_count++;
                    }
                }
            }
            
            if (target_count > 0) {
                // 隨機選擇一個目標
                int random_target = rand() % target_count;
                int target_x = available_targets[random_target][0];
                int target_y = available_targets[random_target][1];
                
                // 檢查是否在雷達範圍內
                if (game->radar_map[target_x][target_y]) {
                    // 雷達吸收攻擊，不顯示結果
                    handle_radar_damage(game, target_x, target_y);
                    return;
                }
                
                // 檢查是否擊中船隻
                bool hit = false;
                for (int j = 0; j < MAX_SHIPS; ++j) {
                    Ship* ship = &target_fleet[j];
                    if (ship->health > 0 && 
                        target_x >= ship->pos_x && 
                        target_x < ship->pos_x + ship->size && 
                        target_y == ship->pos_y) 
                    {
                        ship->health--;
                        hit = true;
                        
                        if (ship->health <= 0) {
                            game->attack_history[target_x][target_y] = ATTACK_SUNK;
                        } else {
                            game->attack_history[target_x][target_y] = ATTACK_HIT;
                        }
                        break;
                    }
                }
                
                if (!hit) {
                    game->attack_history[target_x][target_y] = ATTACK_MISS;
                }
            }
            break;
        }
    }
}


void light_cruiser_attack(GameState* game, int x, int y) {
    Ship* attacker_fleet = game->is_player_turn ? game->player_fleet : game->enemy_fleet;
    Ship* target_fleet = game->is_player_turn ? game->enemy_fleet : game->player_fleet;
    
    // 查看輕巡是否可攻擊
    bool can_attack = false;
    for (int i = 0; i < MAX_SHIPS; ++i) {
        if (attacker_fleet[i].type == SHIP_CRUISER_LIGHT && 
            attacker_fleet[i].health > 0 && 
            attacker_fleet[i].cooldown == 0) {
            can_attack = true;
            attacker_fleet[i].cooldown = 2; // 設置冷卻時間
            break;
        }
    }
    
    if (!can_attack) return;
    
    // 檢查是否已被攻擊過
    if (game->attack_history[x][y] != ATTACK_NONE) return;
    
    // 檢查是否在雷達範圍內
    if (game->radar_map[x][y]) {
        // 雷達吸收攻擊，不顯示結果
        handle_radar_damage(game, x, y);
        return;
    }
    
    // 檢查攻擊目標
    bool hit = false;
    for (int i = 0; i < MAX_SHIPS; ++i) {
        if (target_fleet[i].health > 0 && 
            x >= target_fleet[i].pos_x && 
            x < target_fleet[i].pos_x + target_fleet[i].size &&
            y == target_fleet[i].pos_y) 
        {
            target_fleet[i].health -= 2; //造成兩點傷害
            hit = true;
            
            if (target_fleet[i].health <= 0) {
                game->attack_history[x][y] = ATTACK_SUNK;
            } else {
                game->attack_history[x][y] = ATTACK_HIT;
            }
            break;
        }
    }
    
    if (!hit) {
        game->attack_history[x][y] = ATTACK_MISS;
    }
}

void heavy_cruiser_attack(GameState* game, int x, int y, bool horizontal) {
    Ship* attacker_fleet = game->is_player_turn ? game->player_fleet : game->enemy_fleet;
    Ship* target_fleet = game->is_player_turn ? game->enemy_fleet : game->player_fleet;
    
    // 檢查重巡是否可攻擊
    bool can_attack = false;
    for (int i = 0; i < MAX_SHIPS; ++i) {
        if (attacker_fleet[i].type == SHIP_CRUISER_HEAVY && 
            attacker_fleet[i].health > 0 && 
            attacker_fleet[i].cooldown == 0) {
            can_attack = true;
            attacker_fleet[i].cooldown = 3; // 冷卻時間
            break;
        }
    }
    
    if (!can_attack) return;
    
    bool radar_hit = false;
    
    // 先檢查是否有任何格子在雷達範圍內
    for (int dx = 0; dx < 3; ++dx) {
        int tx = horizontal ? x + dx : x;
        int ty = horizontal ? y : y + dx;
        
        if (tx >= GRID_SIZE || ty >= GRID_SIZE || tx < 0 || ty < 0) continue;
        if (game->attack_history[tx][ty] != ATTACK_NONE) continue;
        
        if (game->radar_map[tx][ty]) {
            radar_hit = true;
            handle_radar_damage(game, tx, ty);
            break;
        }
    }
    
    // 如果有格子在雷達範圍內，只處理雷達被擊毀，不顯示其他格子的結果
    if (radar_hit) return;
    
    // 攻擊所有三格
    for (int dx = 0; dx < 3; ++dx) {
        int tx = horizontal ? x + dx : x;
        int ty = horizontal ? y : y + dx;
        
        if (tx >= GRID_SIZE || ty >= GRID_SIZE || tx < 0 || ty < 0) continue;
        if (game->attack_history[tx][ty] != ATTACK_NONE) continue;

        bool found = false;
        for (int i = 0; i < MAX_SHIPS; ++i) {
            if (target_fleet[i].health > 0 && 
                tx >= target_fleet[i].pos_x && 
                tx < target_fleet[i].pos_x + target_fleet[i].size &&
                ty == target_fleet[i].pos_y) 
            {
                target_fleet[i].health--;
                found = true;
                
                if (target_fleet[i].health <= 0) {
                    game->attack_history[tx][ty] = ATTACK_SUNK;
                } else {
                    game->attack_history[tx][ty] = ATTACK_HIT;
                }
                break;
            }
        }
        
        if (!found) {
            game->attack_history[tx][ty] = ATTACK_MISS;
        }
    }
}

// 自殺快艇攻擊 - 移動並造成2點傷害
void suicide_boat_attack(GameState* game, int target_x, int target_y, int boat_index) {
    Ship* fleet = game->is_player_turn ? game->player_fleet : game->enemy_fleet;
    Ship* target_fleet = game->is_player_turn ? game->enemy_fleet : game->player_fleet;
    
    if (boat_index >= MAX_SHIPS || fleet[boat_index].type != SHIP_SUICIDE_BOAT || 
        fleet[boat_index].health <= 0) return;
    
    Ship* boat = &fleet[boat_index];
    
    // 檢查移動距離（最多2格）
    int dx = abs(target_x - boat->pos_x);
    int dy = abs(target_y - boat->pos_y);
    if (dx + dy > 2 || dx + dy == 0) return;
    
    // 移動路徑上攻擊敵方船隻
    int step_x = (target_x > boat->pos_x) ? 1 : (target_x < boat->pos_x) ? -1 : 0;
    int step_y = (target_y > boat->pos_y) ? 1 : (target_y < boat->pos_y) ? -1 : 0;
    
    int current_x = boat->pos_x;
    int current_y = boat->pos_y;
    
    while (current_x != target_x || current_y != target_y) {
        current_x += step_x;
        current_y += step_y;
        
        // 檢查雷達效果 - 自殺快艇遇到雷達會被顯形
        if (game->radar_map[current_x][current_y]) {
            boat->is_radar_active = true; // 被雷達發現
        }
        
        // 檢查是否撞到敵方船隻
        for (int i = 0; i < MAX_SHIPS; ++i) {
            if (target_fleet[i].health > 0 && 
                current_x >= target_fleet[i].pos_x && 
                current_x < target_fleet[i].pos_x + target_fleet[i].size &&
                current_y == target_fleet[i].pos_y) 
            {
                target_fleet[i].health -= 2; // 造成2點傷害
                break;
            }
        }
    }
    
    // 更新快艇位置
    boat->pos_x = target_x;
    boat->pos_y = target_y;
}

// 潛艇攻擊 - 每3回合發射魚雷
void submarine_attack(GameState* game, bool is_player) {
    Ship* fleet = is_player ? game->player_fleet : game->enemy_fleet;
    Ship* target_fleet = is_player ? game->enemy_fleet : game->player_fleet;
    
    for (int i = 0; i < MAX_SHIPS; ++i) {
        if (fleet[i].type == SHIP_SUBMARINE && fleet[i].health > 0) {
            if (fleet[i].cooldown == 0) {
                // 發射魚雷，攻擊潛艇所在行
                int torpedo_row = fleet[i].pos_y;
                bool torpedo_active = true;
                
                // 根據陣營決定魚雷方向
                int start_x = is_player ? ENEMY_AREA_START_X : PLAYER_AREA_START_X;
                int end_x = is_player ? ENEMY_AREA_END_X : PLAYER_AREA_END_X;
                int step = is_player ? 1 : -1;
                
                for (int x = start_x; x != end_x && torpedo_active; x += step) {
                    // 檢查雷達阻擋 - 魚雷遇到雷達會停在雷達前一格
                    if (game->radar_map[x][torpedo_row]) {
                        // 魚雷被雷達阻擋，停在雷達前一格
                        int stop_x = x - step;
                        if (stop_x >= 0 && stop_x < GRID_SIZE) {
                            game->attack_history[stop_x][torpedo_row] = ATTACK_MISS;
                        }
                        torpedo_active = false;
                        break;
                    }
                    
                    // 檢查是否擊中船隻
                    bool hit = false;
                    for (int j = 0; j < MAX_SHIPS; ++j) {
                        if (target_fleet[j].health > 0 && 
                            x >= target_fleet[j].pos_x && 
                            x < target_fleet[j].pos_x + target_fleet[j].size &&
                            torpedo_row == target_fleet[j].pos_y) 
                        {
                            target_fleet[j].health--;
                            hit = true;
                            torpedo_active = false;
                            
                            if (target_fleet[j].health <= 0) {
                                game->attack_history[x][torpedo_row] = ATTACK_SUNK;
                            } else {
                                game->attack_history[x][torpedo_row] = ATTACK_HIT;
                            }
                            break;
                        }
                    }
                    
                    if (!hit) {
                        game->attack_history[x][torpedo_row] = ATTACK_MISS;
                    }
                }
                
                fleet[i].cooldown = 3; // 重置冷卻
            }
        }
    }
}

// 雷達船效果 - 生成5x5雷達範圍
void radar_ship_effect(GameState* game, Ship* radar_ship) {
    if (radar_ship->type != SHIP_RADAR || radar_ship->health <= 0) return;
    
    int center_x = radar_ship->pos_x;
    int center_y = radar_ship->pos_y;
    
    // 清除舊的雷達範圍
    for (int x = 0; x < GRID_WIDTH; x++) {
        for (int y = 0; y < GRID_HEIGHT; y++) {
            game->radar_map[x][y] = false;
        }
    }
    
    // 生成5x5雷達範圍
    for (int dx = -2; dx <= 2; ++dx) {
        for (int dy = -2; dy <= 2; ++dy) {
            int x = center_x + dx;
            int y = center_y + dy;
            
            if (x < 0 || x >= GRID_SIZE || y < 0 || y >= GRID_SIZE) continue;
            
            // 不包含對方海域
            bool is_player_radar = (center_y < GRID_SIZE/2);
            if (is_player_radar && y >= GRID_SIZE/2 + 1) continue;
            if (!is_player_radar && y < GRID_SIZE/2) continue;
            
            game->radar_map[x][y] = true;
        }
    }
    
    radar_ship->is_radar_active = true;
}

// 處理雷達被攻擊時的效果
void handle_radar_damage(GameState* game, int attack_x, int attack_y) {
    Ship* all_fleets[2] = {game->player_fleet, game->enemy_fleet};
    
    for (int fleet_idx = 0; fleet_idx < 2; fleet_idx++) {
        Ship* fleet = all_fleets[fleet_idx];
        
        for (int i = 0; i < MAX_SHIPS; i++) {
            if (fleet[i].type == SHIP_RADAR && fleet[i].health > 0 && 
                fleet[i].is_radar_active) {
                
                // 檢查攻擊是否在雷達範圍內
                if (game->radar_map[attack_x][attack_y]) {
                    // 雷達船承受傷害並消失
                    fleet[i].health = 0;
                    fleet[i].is_radar_active = false;
                    
                    // 標記雷達範圍內的船隻可以移動
                    mark_ships_for_radar_movement(game, fleet_idx == 0);
                    
                    // 清除雷達效果
                    for (int x = 0; x < GRID_WIDTH; x++) {
                        for (int y = 0; y < GRID_HEIGHT; y++) {
                            game->radar_map[x][y] = false;
                        }
                    }
                    
                    return;
                }
            }
        }
    }
}

void clear_radar_map(GameState* game) {
    for (int x = 0; x < GRID_WIDTH; x++) {
        for (int y = 0; y < GRID_HEIGHT; y++) {
            game->radar_map[x][y] = false;
        }
    }
}

// 標記雷達範圍內的船隻可以移動
void mark_ships_for_radar_movement(GameState* game, bool is_player_radar) {
    Ship* fleet = is_player_radar ? game->player_fleet : game->enemy_fleet;
    
    for (int i = 0; i < MAX_SHIPS; i++) {
        if (fleet[i].health > 0) {
            // 檢查船隻是否在雷達範圍內且該位置之前未被攻擊
            bool in_radar_area = false;
            for (int ship_x = fleet[i].pos_x; ship_x < fleet[i].pos_x + fleet[i].size; ship_x++) {
                if (game->radar_map[ship_x][fleet[i].pos_y] && 
                    game->attack_history[ship_x][fleet[i].pos_y] == ATTACK_NONE) {
                    in_radar_area = true;
                    break;
                }
            }
            
            if (in_radar_area) {
                fleet[i].can_move_in_radar = true;
            }
        }
    }
}

// 普通攻擊
void normal_attack(GameState* game, int x, int y) {
    Ship* target_fleet = game->is_player_turn ? game->enemy_fleet : game->player_fleet;
    
    // 檢查是否已被攻擊過
    if (game->attack_history[x][y] != ATTACK_NONE) return;
    
    // 檢查是否在雷達範圍內
    if (game->radar_map[x][y]) {
        // 雷達吸收攻擊，不顯示結果
        handle_radar_damage(game, x, y);
        return;
    }
    
    // 檢查是否擊中船隻
    bool hit = false;
    for (int i = 0; i < MAX_SHIPS; ++i) {
        if (target_fleet[i].health > 0 && 
            x >= target_fleet[i].pos_x && 
            x < target_fleet[i].pos_x + target_fleet[i].size &&
            y == target_fleet[i].pos_y) 
        {
            target_fleet[i].health--;
            hit = true;
        
        if (target_fleet[i].health <= 0) {
            // 标记该船所有格子为击沉
            for (int dx = 0; dx < target_fleet[i].size; dx++) {
                int tx = target_fleet[i].pos_x + dx;
                int ty = target_fleet[i].pos_y;
                if (tx < GRID_SIZE && ty < GRID_SIZE) {
                    game->attack_history[tx][ty] = ATTACK_SUNK;
                }
            }
        } else {
            // 仅标记当前格子为击中
            game->attack_history[x][y] = ATTACK_HIT;
        }
        break;
    }
}

if (!hit) {
    game->attack_history[x][y] = ATTACK_MISS;
}
}