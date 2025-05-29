#include "ship.h"
#include "game.h"
#include <stdlib.h>
#include <time.h>

Ship create_ship(ShipType type, int x, int y) {
    Ship ship;//船的資訊
    ship.type = type;
    ship.pos_x = x;
    ship.pos_y = y;
    ship.cooldown = 0;
    ship.is_radar_active = false;
    ship.can_move_in_radar = false;
    ship.is_visible = true; // 預設可見
    ship.is_horizontal = true; // 新增：預設水平方向

    switch(type) {
        case SHIP_FACTORY:
            ship.health = 4;
            ship.size = 2;
            break;
        case SHIP_CARRIER:
            ship.health = 5;
            ship.size = 5;
            break;
        case SHIP_CRUISER_LIGHT:
            ship.health = 3;
            ship.size = 3;
            break;
        case SHIP_CRUISER_HEAVY:
            ship.health = 3;
            ship.size = 3;
            break;
        case SHIP_SUBMARINE:
            ship.health = 2;
            ship.size = 2;
            break;
        case SHIP_RADAR:
            ship.health = 1;
            ship.size = 1;
            break;
        case SHIP_SUICIDE_BOAT:
            ship.health = 1;
            ship.size = 1;
            ship.is_visible = false; // 自殺快艇預設不可見
            break;
        default:
            ship.health = 1;
            ship.size = 1;
    }
    return ship;
}

//計算技能冷卻時間
void update_cooldowns(Ship* fleet) {
    for (int i = 0; i < MAX_SHIPS; ++i) {
        if (fleet[i].health > 0 && fleet[i].cooldown > 0) {
            fleet[i].cooldown--;
        }
    }
}

// 檢查位置是否可放置船隻
bool is_position_valid_for_ship(GameState* game, int x, int y, int size, bool is_player, ShipType type) {
    if (type == SHIP_FACTORY) { // 2x2兵工廠
            Ship* ship = &game->player_fleet[game->selected_ship_index];
    int size = ship->size;
    bool is_horizontal = ship->is_horizontal;
    
    // 根據方向計算所需空間
    int req_width = is_horizontal ? size : 1;
    int req_height = is_horizontal ? 1 : size;
    
    // 檢查邊界
    if (x < 0 || y < 0 || 
        x + req_width > GRID_WIDTH || 
        y + req_height > GRID_HEIGHT) {
        return false;
    }
    if (x < 0 || y < 0 || x + size > GRID_WIDTH || y >= GRID_HEIGHT) {
            return false;
        }
    } 
    else { // 其他船艦
            if (x < 0 || y < 0 || x + size > GRID_WIDTH || y >= GRID_HEIGHT) {
                return false;
            }
    }
    // 檢查邊界
        if (x < 0 || y < 0 || x + size > GRID_WIDTH || y >= GRID_HEIGHT) {
            return false;
        }
    
    // 檢查是否在正確的區域
    if (is_player) {
        // 玩家區域 (x: 0-14)
        if (x < PLAYER_AREA_START_X || x + size > PLAYER_AREA_END_X) {
            return false;
        }
    } else {
        // 敵方區域 (x: 15-29)
        if (x < ENEMY_AREA_START_X || x + size > ENEMY_AREA_END_X) {
            return false;
        }
    }
    
    // 檢查是否已被攻擊過
    for (int i = 0; i < size; i++) {
        if (game->attack_history[x + i][y] != ATTACK_NONE) {
            return false;
        }
    }
    
    // 檢查是否與其他船隻重疊
    Ship* player_fleet = game->player_fleet;
    Ship* enemy_fleet = game->enemy_fleet;
    
    for (int fleet_idx = 0; fleet_idx < 2; fleet_idx++) {
        Ship* fleet = (fleet_idx == 0) ? player_fleet : enemy_fleet;
        
        for (int i = 0; i < MAX_SHIPS; i++) {
            if (fleet[i].health > 0) {
                // 檢查重疊
                if (y == fleet[i].pos_y) {
                    int ship_start = fleet[i].pos_x;
                    int ship_end = fleet[i].pos_x + fleet[i].size;
                    int new_start = x;
                    int new_end = x + size;
                    
                    if (!(new_end <= ship_start || new_start >= ship_end)) {
                        return false; // 有重疊
                    }
                }
            }
        }
    }
    
    return true;
}

// 兵工廠生產邏輯
void factory_production(GameState* game, bool is_player, int production_choice) {
    Ship* fleet = is_player ? game->player_fleet : game->enemy_fleet;
    Ship* factory = NULL;
    
    // 找到活著的兵工廠
    for (int i = 0; i < MAX_SHIPS; ++i) {
        if (fleet[i].type == SHIP_FACTORY && fleet[i].health > 0) {
            factory = &fleet[i];
            break;
        }
    }
    
    if (!factory) return; // 沒有兵工廠
    
    int empty_slot = find_empty_ship_slot(fleet);
    if (empty_slot == -1) return; // 沒有空位
    
    Ship new_ship;
    bool placed = false;
    
    switch(production_choice) {
        case 0: // 自殺快艇 - 只能放在兵工廠旁邊
            {
                // 嘗試兵工廠周圍的位置
                int directions[4][2] = {{-1,0}, {1,0}, {0,-1}, {0,1}};
                for (int d = 0; d < 4; d++) {
                    int new_x = factory->pos_x + directions[d][0];
                    int new_y = factory->pos_y + directions[d][1];
                    
                    if (is_position_valid_for_ship(game, new_x, new_y, 1, is_player, SHIP_SUICIDE_BOAT)) {
                        new_ship = create_ship(SHIP_SUICIDE_BOAT, new_x, new_y);
                        fleet[empty_slot] = new_ship;
                        placed = true;
                        break;
                    }
                }
            }
            break;
            
        case 1: // 雷達船
            {
                int area_start = is_player ? 0 : GRID_SIZE/2 + 1;
                int area_end = is_player ? GRID_SIZE/2 : GRID_SIZE;
                
                for (int attempts = 0; attempts < 100; attempts++) {
                    int rand_x = rand() % GRID_SIZE;
                    int rand_y = area_start + (rand() % (area_end - area_start));
                    
                    if (is_position_valid_for_ship(game, rand_x, rand_y, 1, is_player, SHIP_RADAR)) {
                        new_ship = create_ship(SHIP_RADAR, rand_x, rand_y);
                        fleet[empty_slot] = new_ship;
                        placed = true;
                        break;
                    }
                }
            }
            break;
            
        case 2: // 潛艇
            {
                int area_start = is_player ? 0 : GRID_SIZE/2 + 1;
                int area_end = is_player ? GRID_SIZE/2 : GRID_SIZE;
                
                for (int attempts = 0; attempts < 100; attempts++) {
                    int rand_x = rand() % (GRID_SIZE - 1); // 確保有空間放置2格船隻
                    int rand_y = area_start + (rand() % (area_end - area_start));
                    
                    if (is_position_valid_for_ship(game, rand_x, rand_y, 2, is_player, SHIP_SUBMARINE)) {
                        new_ship = create_ship(SHIP_SUBMARINE, rand_x, rand_y);
                        fleet[empty_slot] = new_ship;
                        placed = true;
                        break;
                    }
                }
            }
            break;
    }
    
    if (placed) {
        // 重置兵工廠冷卻時間
        game->factory_cooldown = 4;
    }
}

// 檢查兵工廠是否可以生產
bool can_factory_produce(GameState* game, bool is_player) {
    if (game->factory_cooldown > 0) return false;
    
    Ship* fleet = is_player ? game->player_fleet : game->enemy_fleet;
    
    // 檢查是否有活著的兵工廠
    for (int i = 0; i < MAX_SHIPS; ++i) {
        if (fleet[i].type == SHIP_FACTORY && fleet[i].health > 0) {
            return true;
        }
    }
    return false;
}

int find_empty_ship_slot(Ship* fleet) {
    for (int i = 0; i < MAX_SHIPS; i++) {
        if (fleet[i].health <= 0) {
            return i;
        }
    }
    return -1; // 沒有空位
}