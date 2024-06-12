#include <stdio.h>
#include <allegro.h>
#include <stdlib.h>
#include <time.h>

#define SCREEN_W 800
#define SCREEN_H 600
#define CARD_SIZE 80
#define MAX_NUM_CARDS 42  // Máximo de cartas para 21 pares

#define INITIAL_MENU 0
#define GAME 1
#define PAUSE_MENU 2
#define END_GAME 3
#define EXIT 4

typedef struct {
    int x, y;
    int number;
    int flipped;
    int matched;
    int owner;
} Card;

void init();
void deinit();
void shuffle_cards(Card *cards, int num_cards);
void draw_cards(Card *cards, BITMAP *buffer, int current_player, int num_cards, int *scores, int rows, int cols, int round);
Card* get_card_at_position(Card *cards, int x, int y, int num_cards);
void draw_initial_menu(BITMAP *buffer, int option);
void draw_pause_menu(BITMAP *buffer, int option);
void draw_end_game_menu(BITMAP *buffer, int option, int winner);
int process_initial_menu_input(int *option, int *state, int *num_players);
int process_pause_menu_input(int *option, int *state);
int process_end_game_menu_input(int *option, int *state);
int contains(int arr[], int size, int num);

int main() {
    Card cards[MAX_NUM_CARDS];
    int pairs_found = 0;
    int current_level = 1;
    int max_pairs = current_level * 2; // Inicialmente 2 pares
    int rows = 2, cols = 2;  // Inicialmente 2x2
    int num_cards = max_pairs * 2;
    Card *first_card = NULL;
    Card *second_card = NULL;
    int menu_option = 0;
    int state = INITIAL_MENU;
    int num_players = 2;  // Default para dois jogadores
    int current_player = 0;
    int scores[2] = {0, 0};
    int round = 1; // Contador de rodadas
    BITMAP *buffer;

    init();
    buffer = create_bitmap(SCREEN_W, SCREEN_H);
    srand(time(NULL));
    shuffle_cards(cards, num_cards);

    for (int i = 0; i < num_cards; i++) {
        cards[i].x = (i % cols) * (CARD_SIZE + 10) + 50;
        cards[i].y = (i / cols) * (CARD_SIZE + 10) + 100;
        cards[i].flipped = 0;
        cards[i].matched = 0;
        cards[i].owner = -1;  // Nenhum dono inicialmente
    }

    while (state != EXIT) {
        if (state == INITIAL_MENU) {
            draw_initial_menu(buffer, menu_option);
            state = process_initial_menu_input(&menu_option, &state, &num_players);
        } else if (state == GAME) {
            if (pairs_found < max_pairs) {
                if (key[KEY_ESC]) {
                    state = PAUSE_MENU;
                    rest(150);
                }

                if (mouse_b & 1) {
                    Card *clicked_card = get_card_at_position(cards, mouse_x, mouse_y, num_cards);

                    if (clicked_card && !clicked_card->flipped && !clicked_card->matched) {
                        clicked_card->flipped = 1;

                        if (!first_card) {
                            first_card = clicked_card;
                        } else if (!second_card) {
                            second_card = clicked_card;

                            // Mostra a segunda carta mesmo se não corresponder
                            draw_cards(cards, buffer, current_player, num_cards, scores, rows, cols, round);
                            blit(buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
                            rest(500);  // Aguarda um tempo mais curto para mostrar a segunda carta

                            if (first_card->number == second_card->number) {
                                first_card->matched = 1;
                                second_card->matched = 1;
                                first_card->owner = current_player;
                                second_card->owner = current_player;
                                pairs_found++;
                                scores[current_player]++;
                            } else {
                                first_card->flipped = 0;
                                second_card->flipped = 0;
                                current_player = (current_player + 1) % num_players;  // Alterna a vez do jogador
                            }
                            first_card = NULL;
                            second_card = NULL;
                        }
                    }
                }

                clear_to_color(buffer, makecol(0, 0, 0));  // Fundo preto
                draw_cards(cards, buffer, current_player, num_cards, scores, rows, cols, round);
                rest(10);
            } else {
                // Final de uma rodada
                round++;
                if (round > 6) {
                    // Após 6 rodadas, determine o vencedor
                    state = END_GAME;
                    int winner = (scores[0] > scores[1]) ? 0 : 1;
                    menu_option = 0;
                } else {
                    // Reset para próxima rodada
                    pairs_found = 0;
                    if (round % 2 == 1) {
                        rows++;
                        cols++;
                    }
                    max_pairs = (rows * cols) / 2;
                    num_cards = max_pairs * 2;
                    shuffle_cards(cards, num_cards);
                    for (int i = 0; i < num_cards; i++) {
                        cards[i].flipped = 0;
                        cards[i].matched = 0;
                        cards[i].owner = -1;
                        cards[i].x = (i % cols) * (CARD_SIZE + 10) + 50;
                        cards[i].y = (i / cols) * (CARD_SIZE + 10) + 100;
                    }
                }
            }
        } else if (state == END_GAME) {
            int winner = (scores[0] > scores[1]) ? 0 : 1;
            draw_end_game_menu(buffer, menu_option, winner);
            state = process_end_game_menu_input(&menu_option, &state);
            if (state == GAME) {
                // Resetar o jogo
                pairs_found = 0;
                current_level = 1;
                rows = 2;
                cols = 2;
                max_pairs = (rows * cols) / 2;
                num_cards = max_pairs * 2;
                first_card = NULL;
                second_card = NULL;
                current_player = 0;
                scores[0] = 0;
                scores[1] = 0;
                round = 1; // Resetar rodadas
                shuffle_cards(cards, num_cards);
                for (int i = 0; i < num_cards; i++) {
                    cards[i].flipped = 0;
                    cards[i].matched = 0;
                    cards[i].owner = -1;
                    cards[i].x = (i % cols) * (CARD_SIZE + 10) + 50;
                    cards[i].y = (i / cols) * (CARD_SIZE + 10) + 100;
                }
            }
        } else if (state == PAUSE_MENU) {
            draw_pause_menu(buffer, menu_option);
            state = process_pause_menu_input(&menu_option, &state);
            if (state == GAME) {
                // Retomar o jogo
            } else if (state == INITIAL_MENU) {
                // Voltar ao menu inicial
                pairs_found = 0;
                current_level = 1;
                rows = 2;
                cols = 2;
                max_pairs = (rows * cols) / 2;
                num_cards = max_pairs * 2;
                first_card = NULL;
                second_card = NULL;
                current_player = 0;
                scores[0] = 0;
                scores[1] = 0;
                round = 1; // Resetar rodadas
                shuffle_cards(cards, num_cards);
                for (int i = 0; i < num_cards; i++) {
                    cards[i].flipped = 0;
                    cards[i].matched = 0;
                    cards[i].owner = -1;
                    cards[i].x = (i % cols) * (CARD_SIZE + 10) + 50;
                    cards[i].y = (i / cols) * (CARD_SIZE + 10) + 100;
                }
            }
        }
        show_mouse(screen);  // Exibe o mouse na tela
    }

    destroy_bitmap(buffer);
    deinit();
    return 0;
}
END_OF_MAIN();

void init() {
    allegro_init();
    install_keyboard();
    install_mouse();
    set_color_depth(32);
    set_gfx_mode(GFX_AUTODETECT_WINDOWED, SCREEN_W, SCREEN_H, 0, 0);
    install_sound(DIGI_AUTODETECT, MIDI_AUTODETECT, NULL);
    set_window_title("Jogo da Memoria");
}

void deinit() {
    clear_keybuf();
}

void shuffle_cards(Card *cards, int num_cards) {
    for (int i = 0; i < num_cards; i++) {
        cards[i].number = i / 2;
        cards[i].flipped = 0;
        cards[i].matched = 0;
        cards[i].owner = -1;  // Nenhum dono inicialmente
    }
    for (int i = 0; i < num_cards; i++) {
        int r = rand() % num_cards;
        Card temp = cards[i];
        cards[i] = cards[r];
        cards[r] = temp;
    }
}

void draw_cards(Card *cards, BITMAP *buffer, int current_player, int num_cards, int *scores, int rows, int cols, int round) {
    clear_to_color(buffer, makecol(0, 0, 0));  // Fundo preto
    for (int i = 0; i < num_cards; i++) {
        rectfill(buffer, cards[i].x, cards[i].y, cards[i].x + CARD_SIZE, cards[i].y + CARD_SIZE, makecol(255, 255, 255));
        if (cards[i].flipped || cards[i].matched) {
            textprintf_centre_ex(buffer, font, cards[i].x + CARD_SIZE / 2, cards[i].y + CARD_SIZE / 2, makecol(0, 0, 0), -1, "%d", cards[i].number);
        }
    }
    textprintf_ex(buffer, font, 10, 10, makecol(255, 255, 255), -1, "Jogador 1: %d pontos", scores[0]);
    textprintf_ex(buffer, font, 10, 30, makecol(255, 255, 255), -1, "Jogador 2: %d pontos", scores[1]);
    textprintf_ex(buffer, font, SCREEN_W - 200, 10, makecol(255, 255, 255), -1, "Jogador Atual: %d", current_player + 1);
    textprintf_ex(buffer, font, SCREEN_W - 200, 30, makecol(255, 255, 255), -1, "Rodada: %d", round);
    blit(buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
}

Card* get_card_at_position(Card *cards, int x, int y, int num_cards) {
    for (int i = 0; i < num_cards; i++) {
        if (x >= cards[i].x && x <= cards[i].x + CARD_SIZE && y >= cards[i].y && y <= cards[i].y + CARD_SIZE) {
            return &cards[i];
        }
    }
    return NULL;
}

void draw_initial_menu(BITMAP *buffer, int option) {
    clear_to_color(buffer, makecol(0, 0, 0));  // Fundo preto
    textout_centre_ex(buffer, font, "Menu Inicial", SCREEN_W / 2, SCREEN_H / 2 - 40, makecol(255, 255, 255), -1);
    textout_centre_ex(buffer, font, "1 Jogador", SCREEN_W / 2, SCREEN_H / 2, makecol(255, 255, 255), (option == 0) ? makecol(255, 0, 0) : -1);
    textout_centre_ex(buffer, font, "2 Jogadores", SCREEN_W / 2, SCREEN_H / 2 + 20, makecol(255, 255, 255), (option == 1) ? makecol(255, 0, 0) : -1);
    textout_centre_ex(buffer, font, "Sair", SCREEN_W / 2, SCREEN_H / 2 + 40, makecol(255, 255, 255), (option == 2) ? makecol(255, 0, 0) : -1);
    blit(buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
}

void draw_pause_menu(BITMAP *buffer, int option) {
    clear_to_color(buffer, makecol(0, 0, 0));  // Fundo preto
    textout_centre_ex(buffer, font, "Jogo Pausado", SCREEN_W / 2, SCREEN_H / 2 - 40, makecol(255, 255, 255), -1);
    textout_centre_ex(buffer, font, "Continuar", SCREEN_W / 2, SCREEN_H / 2, makecol(255, 255, 255), (option == 0) ? makecol(255, 0, 0) : -1);
    textout_centre_ex(buffer, font, "Voltar ao Menu Inicial", SCREEN_W / 2, SCREEN_H / 2 + 20, makecol(255, 255, 255), (option == 1) ? makecol(255, 0, 0) : -1);
    textout_centre_ex(buffer, font, "Sair", SCREEN_W / 2, SCREEN_H / 2 + 40, makecol(255, 255, 255), (option == 2) ? makecol(255, 0, 0) : -1);
    blit(buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
}

void draw_end_game_menu(BITMAP *buffer, int option, int winner) {
    clear_to_color(buffer, makecol(0, 0, 0));  // Fundo preto
    char winner_text[50];
    sprintf(winner_text, "Jogador %d Venceu!", winner + 1);
    textout_centre_ex(buffer, font, winner_text, SCREEN_W / 2, SCREEN_H / 2 - 40, makecol(255, 255, 255), -1);
    textout_centre_ex(buffer, font, "Jogar Novamente", SCREEN_W / 2, SCREEN_H / 2, makecol(255, 255, 255), (option == 0) ? makecol(255, 0, 0) : -1);
    textout_centre_ex(buffer, font, "Voltar ao Menu Inicial", SCREEN_W / 2, SCREEN_H / 2 + 20, makecol(255, 255, 255), (option == 1) ? makecol(255, 0, 0) : -1);
    textout_centre_ex(buffer, font, "Sair", SCREEN_W / 2, SCREEN_H / 2 + 40, makecol(255, 255, 255), (option == 2) ? makecol(255, 0, 0) : -1);
    blit(buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
}

int process_initial_menu_input(int *option, int *state, int *num_players) {
    if (key[KEY_DOWN]) {
        (*option)++;
        if (*option > 2) *option = 0;
        rest(150);
    } else if (key[KEY_UP]) {
        (*option)--;
        if (*option < 0) *option = 2;
        rest(150);
    } else if (key[KEY_ENTER]) {
        if (*option == 0) {
            *num_players = 1;
            *state = GAME;
        } else if (*option == 1) {
            *num_players = 2;
            *state = GAME;
        } else if (*option == 2) {
            *state = EXIT;
        }
        rest(150);
    }
    return *state;
}

int process_pause_menu_input(int *option, int *state) {
    if (key[KEY_DOWN]) {
        (*option)++;
        if (*option > 2) *option = 0;
        rest(150);
    } else if (key[KEY_UP]) {
        (*option)--;
        if (*option < 0) *option = 2;
        rest(150);
    } else if (key[KEY_ENTER]) {
        if (*option == 0) {
            *state = GAME;
        } else if (*option == 1) {
            *state = INITIAL_MENU;
        } else if (*option == 2) {
            *state = EXIT;
        }
        rest(150);
    }
    return *state;
}

int process_end_game_menu_input(int *option, int *state) {
    if (key[KEY_DOWN]) {
        (*option)++;
        if (*option > 2) *option = 0;
        rest(150);
    } else if (key[KEY_UP]) {
        (*option)--;
        if (*option < 0) *option = 2;
        rest(150);
    } else if (key[KEY_ENTER]) {
        if (*option == 0) {
            *state = GAME;
        } else if (*option == 1) {
            *state = INITIAL_MENU;
        } else if (*option == 2) {
            *state = EXIT;
        }
        rest(150);
    }
    return *state;
}
