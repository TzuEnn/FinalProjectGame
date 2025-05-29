# =====================
# 一般設定
# =====================
TARGET = game
SRC = combat.c game.c main.c resource.c ship.c
OBJS = $(SRC:.c=.o)
CFLAGS = -Wall -std=c11 -O2 -g

# =====================
# 系統偵測與平台設定
# =====================
ifeq ($(OS),Windows_NT)  # Windows 系統
    TARGET := $(TARGET).exe
    # Windows Allegro 路徑 - 依照你的安裝調整
    ALLEGRO_INCLUDE = -IC:/allegro/include
    ALLEGRO_LIB = -LC:/allegro/lib \
        -lallegro_monolith \
        -lallegro_main -lallegro_image -lallegro_font -lallegro_ttf \
        -lallegro_primitives -lallegro_audio -lallegro_acodec
    RM = del /Q
    MKDIR = mkdir
else  # macOS 或 Linux
    ALLEGRO_INCLUDE = $(shell pkg-config --cflags allegro-5 allegro_main-5)
    ALLEGRO_LIB = $(shell pkg-config --libs allegro-5 allegro_main-5 allegro_image-5 allegro_font-5 allegro_ttf-5 allegro_primitives-5 allegro_audio-5 allegro_acodec-5)
    RM = rm -f
    MKDIR = mkdir -p
endif

# =====================
# 預設目標
# =====================
all: $(TARGET)

# =====================
# 編譯規則
# =====================
%.o: %.c
	$(CC) -c $(CFLAGS) $(ALLEGRO_INCLUDE) -o $@ $<

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(ALLEGRO_LIB)

# =====================
# 清除目標
# =====================
clean:
	$(RM) $(OBJS) $(TARGET)

.PHONY: all clean
