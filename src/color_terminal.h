// Black        0;30     Dark Gray     1;30
// Red          0;31     Light Red     1;31
// Green        0;32     Light Green   1;32
// Brown/Orange 0;33     Yellow        1;33
// Blue         0;34     Light Blue    1;34
// Purple       0;35     Light Purple  1;35
// Cyan         0;36     Light Cyan    1;36
// Light Gray   0;37     White         1;37

#define COLOR_BLACK "\033[0;30m"
#define COLOR_RED "\033[0;31m"
#define COLOR_GREEN "\033[0;32m"
#define COLOR_BROWN "\033[0;33m"
#define COLOR_BLUE "\033[0;34m"
#define COLOR_PURPLE "\033[0;35m"
#define COLOR_CYAN "\033[0;36m"
#define COLOR_LIGHT_GRAY "\033[0;37m"

#define COLOR_DARK_GRAY "\033[1;30m"
#define COLOR_LIGT_RED "\033[1;31m"
#define COLOR_LIGT_GREEN "\033[1;32m"
#define COLOR_YELLOW "\033[1;33m"
#define COLOR_LIGT_BLUE "\033[1;34m"
#define COLOR_LIGT_PURPLE "\033[1;35m"
#define COLOR_LIGHT_CYAN "\033[1;36m"
#define COLOR_WHITE "\033[1;37m"

#define COLOR_NONE "\033[0m"

#define text_color(FG) \
    Serial.print(FG);

#define print_color(FG, S) \
    Serial.print(FG);      \
    Serial.print(S);       \
    Serial.print(COLOR_NONE);

#define println_color(FG, S) \
    Serial.print(FG);        \
    Serial.println(S);       \
    Serial.print(COLOR_NONE);

// #define printf_color(FG, SF, S)       \
//   Serial.print(FG); \
//   Serial.println(S);         \
//   Serial.print(COLOR_NONE);
