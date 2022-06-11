//milosz zalubski-gabis
//katarzyna zoledowska
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_audio.h> 
#include <allegro5/allegro_acodec.h>
#pragma warning(disable:4996)
#define _CRT_SECURE_NO_WARNINGS

#define FPS 60
#define DISP_W 960 //szerokosc ekranu
#define DISP_H 720 //wysokosc ekranu
//poczatkowe kolory tla
int DISP_R = 0; 
int DISP_G = 0;
int DISP_B = 0;

bool ai = false; //czy gra jest prowadzona z ai
bool main_screen = false; //czy wyswietlany jest ekran rozgrywki lub startowy
bool start = false; //czy gra sie rozpoczela
bool end = false; //czy gra sie zakonczyla
bool restart = false; //czy gracz chce restartu
int winner = 0; //przechowuje zwyciezce
//odpowiadaja za kolory graczy i tla
int p1 = 0; 
int p2 = 0;
int p3 = 0;
//do wczytywania ustawien
int set_arr[7];
int max_score = 3;

//sprawdza czy udalo sie zaladowac biblioteke
void must_init(bool test, const char* desc) {
	if (test) return;
	printf("couldn't initialize %s\n", desc);
	exit(1);
}

void all_init() {
    must_init(al_init(), "allegro");
    must_init(al_install_keyboard(), "keyboard");
    must_init(al_init_primitives_addon(), "primitives");
    must_init(al_init_ttf_addon(), "ttf");
    must_init(al_install_audio(), "audio");
    must_init(al_init_acodec_addon(), "audio codecs");
    must_init(al_reserve_samples(16), "reserve samples");
}

//laduje dzwieki do gry
ALLEGRO_SAMPLE* game_sound[3];
//0 - paddle
//1 - wall
//2 - score

void audio() {
    game_sound[0] = al_load_sample("sounds/paddle.wav");
    must_init(game_sound[0], "paddle");
    game_sound[1] = al_load_sample("sounds/wall.wav");
    must_init(game_sound[1], "wall");
    game_sound[2] = al_load_sample("sounds/score.wav");
    must_init(game_sound[2], "score");
}

void audio_deinit() {
    al_destroy_sample(game_sound[0]);
    al_destroy_sample(game_sound[1]);
    al_destroy_sample(game_sound[2]);
}
FILE* scores;
void open_file() {
    scores = fopen("files/scores.txt", "a+");
}

void close_file() {
    fclose(scores);
}

//laduje czcionke
ALLEGRO_FONT* font;
ALLEGRO_FONT* font_big;
ALLEGRO_FONT* font_small;
void font_init() {
    font = al_load_ttf_font("font/font.ttf", 64, 0);
    font_big = al_load_ttf_font("font/font.ttf", 128, 0);
    font_small = al_load_ttf_font("font/font.ttf", 32, 0);
    must_init(font, "font");
}

void font_deinit() {
    al_destroy_font(font);
}
//odpowiada za odczyt wciskanych klawiszy tak, by gra czytala kilka wcisnietych na raz
//sposob z wiki allegro
#define KEY_SEEN     1
#define KEY_RELEASED 2
unsigned char key[ALLEGRO_KEY_MAX];

void keyboard_init() {
    memset(key, 0, sizeof(key));
}

void keyboard_update(ALLEGRO_EVENT* event) {
    switch (event->type) {
    case ALLEGRO_EVENT_TIMER:
        for (int i = 0; i < ALLEGRO_KEY_MAX; i++)
            key[i] &= KEY_SEEN;
        break;

    case ALLEGRO_EVENT_KEY_DOWN:
        key[event->keyboard.keycode] = KEY_SEEN | KEY_RELEASED;
        break;
    case ALLEGRO_EVENT_KEY_UP:
        key[event->keyboard.keycode] &= KEY_RELEASED;
        break;
    }
}

//player
typedef struct PLAYER {
    int x, y;
    int speed;
    int score;
    int colorR, colorG, colorB;
} PLAYER;
PLAYER player;
PLAYER player2;

//player.width = 10;
//player.height = 100;

//tworzy gracza o domyslnych wartosciach
void player_init() {
    
    player.x = DISP_W-50;
    player.y = (DISP_H / 2) - (100 / 2);
    player.speed = 15;
    player.score = 0;

    player2.x = 50;
    player2.y = (DISP_H / 2) - (100 / 2);
    player2.speed = 15;
    player2.score = 0;
}

//odpowiada za ruchy gracza 1
void player_update() {
    if (key[ALLEGRO_KEY_LEFT])
        player.y -= (player.speed);
    if (key[ALLEGRO_KEY_RIGHT])
        player.y += (player.speed);
    if (player.y < 15)
        player.y = 15;
    if (player.y > 705 - 100)
        player.y = 705 - 100;
}

//odpowiada za ruchy gracza 2 
void player2_update() {
    if (key[ALLEGRO_KEY_A])
        player2.y -= (player2.speed);
    if (key[ALLEGRO_KEY_D])
        player2.y += (player2.speed);
    if (player2.y < 15)
        player2.y = 15;
    if (player2.y > 705 - 100)
        player2.y = 705 - 100;
}

//zmiana koloru gracza na poczatku gry i ekran startowy
void player_color() {

        al_draw_textf(font_big, al_map_rgb(255, 255, 255), DISP_W / 2, DISP_H - 680, ALLEGRO_ALIGN_CENTER, "PONG");
        al_draw_textf(font_small, al_map_rgb(255, 255, 255), DISP_W / 8, DISP_H - 460, 0, "MAX SCORE: %d (3, 6 or 9)", max_score);
        al_draw_textf(font_small, al_map_rgb(255, 255, 255), DISP_W / 8, DISP_H - 420, 0, "Left/Right to change Player 1 color");
        al_draw_textf(font_small, al_map_rgb(255, 255, 255), DISP_W / 8, DISP_H - 380, 0, "A/D to change Player 2 color");
        al_draw_textf(font_small, al_map_rgb(255, 255, 255), DISP_W / 8, DISP_H - 340, 0, "Optional: Q/E for background");
        al_draw_textf(font, al_map_rgb(255, 255, 255), DISP_W / 8, DISP_H - 100, 0, "PRESS ENTER TO START");

        al_draw_filled_rectangle(DISP_W * 0.75+10, DISP_H - 410, DISP_W * 0.75 + 25, DISP_H - 410 + 15, al_map_rgb(player.colorR, player.colorG, player.colorB));
        al_draw_filled_rectangle(DISP_W * 0.75+10, DISP_H - 370, DISP_W * 0.75 + 25, DISP_H - 370 + 15, al_map_rgb(player2.colorR, player2.colorG, player2.colorB));

        //jezeli nie wycztalo kolorow
        if ((player.colorR == 0 && player.colorG == 0 && player.colorB == 0) || (player2.colorR == 0 && player2.colorG == 0 && player2.colorB == 0)) {
            p1 = 1;
            p2 = 1;
        }
        if (max_score == 0) {
            max_score = 3;
        }


        //player
        if (p1 == 1) {
            player.colorR = 255; player.colorG = 255; player.colorB = 255;
        }
        else if (p1 == 2) {
            player.colorR = 255; player.colorG = 102; player.colorB = 99;
        }
        else if (p1 == 3) {
            player.colorR = 254; player.colorG = 177; player.colorB = 68;
        }
        else if (p1 == 4) {
            player.colorR = 158; player.colorG = 224; player.colorB = 158;
        }
        else if (p1 == 5) {
            player.colorR = 158; player.colorG = 193; player.colorB = 207;
        }
        else if (p1 == 6) {
            player.colorR = 204; player.colorG = 153; player.colorB = 201;
        }
        else if (p1 > 6) {
            p1 = 1;
        }
        else if (p1 < 1) {
            p1 = 6;
        }
        //player2
        if (p2 == 1) {
            player2.colorR = 255; player2.colorG = 255; player2.colorB = 255;
        }
        else if (p2 == 2) {
            player2.colorR = 255; player2.colorG = 102; player2.colorB = 99;
        }
        else if (p2 == 3) {
            player2.colorR = 254; player2.colorG = 177; player2.colorB = 68;
        }
        else if (p2 == 4) {
            player2.colorR = 158; player2.colorG = 224; player2.colorB = 158;
        }
        else if (p2 == 5) {
            player2.colorR = 158; player2.colorG = 193; player2.colorB = 207;
        }
        else if (p2 == 6) {
            player2.colorR = 204; player2.colorG = 153;  player2.colorB = 201;
        }
        else if (p2 > 6) {
            p2 = 1;
        }
        else if (p2 < 1) {
            p2 = 6;
        }
        //tlo
        if (p3 == 0) {
            DISP_R = 0;
            DISP_G = 0;
            DISP_B = 0;
        }
        else if (p3 == 1) {
            DISP_R = 38;
            DISP_G = 35;
            DISP_B = 34;
        }
        else if (p3 == 2) {
            DISP_R = 83;
            DISP_G = 89;
            DISP_B = 154;
        }
        else if (p3 == 3) {
            DISP_R = 6;
            DISP_G = 141;
            DISP_B = 157;
        }
        else if (p3 > 3) {
            p3 = 0;
        }
        else if (p3 < 0) {
            p3 = 3;
        }
}

//rysuje na planszy gracza
void draw_player() {
    al_draw_filled_rectangle(player.x, player.y, player.x + 15, player.y + 100, al_map_rgb(player.colorR, player.colorG, player.colorB));
    al_draw_filled_rectangle(player2.x, player2.y, player2.x - 15, player2.y + 100, al_map_rgb(player2.colorR, player2.colorG, player2.colorB));
}

//ball
typedef struct BALL {
    int x, y;
    int width;
    int height;
    int radius;
    int direction;
    int speedX;
    int speedY;
} BALL;
BALL ball;

//tworzy pilke o domyslnych wartosciach
int direction = 1; //w ktorym kierunku na poczatku leci pilka
void ball_init() {
    ball.radius = 16;
    ball.x = DISP_W / 2;
    ball.y = DISP_H / 2;
    ball.speedX = 5 * direction;
    ball.speedY = 0;
}

//rysuje pilke
void draw_ball() {
    al_draw_filled_circle(ball.x, ball.y, ball.radius, al_map_rgb(255,255,255));
}
//ball.y >= player2.y - ball.radius && ball.y <= player2.y + 100 + ball.radius
//odpowiada za ruchy komputera
void computer_player() {
    if (ball.y > player2.y + 50) {
        player2.y += 4;
    }
    if (ball.y <= player2.y + 50) {
        player2.y -= 4;
    }           
        
    if (player2.y < 15)
        player2.y = 15;
    if (player2.y > 705 - 100)
        player2.y = 705 - 100;

}

//dzialanie pi³ki
void ball_update() {

    //kolizja z graczem po prawej
    ball.x += ball.speedX;
    ball.y += ball.speedY;
    if (ball.x >= player.x - ball.radius && ball.x <= player.x + 10) {
        if (ball.y >= player.y - ball.radius && ball.y <= player.y + 100 + ball.radius) {
            al_play_sample(game_sound[0], 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
            if (ball.y >= player.y - ball.radius && ball.y < player.y + 5 + ball.radius) {
                ball.speedY = 8;

            }
            if (ball.y >= player.y + 5 + ball.radius && ball.y < player.y + 45 + ball.radius) {
                ball.speedY = 4;

            }
            if (ball.y >= player.y + 45 + ball.radius && ball.y < player.y + 55 + ball.radius) {
                ball.speedY = 0;

            }
            if (ball.y >= player.y + 55 + ball.radius && ball.y < player.y + 95 + ball.radius) {
                ball.speedY = -4;

            }
            if (ball.y >= player.y + 95 + ball.radius && ball.y <= player.y + 100 + ball.radius) {
                ball.speedY = -8;
                
            }
            ball.speedX = -ball.speedX - 1;
            ball.x = player.x - ball.radius;
            
        }
    }

    //kolizja z graczem po lewej
    if (ball.x <= player2.x + ball.radius && ball.x >= player2.x - 10) {
        if (ball.y >= player2.y - ball.radius && ball.y <= player2.y + 100 + ball.radius) {
            al_play_sample(game_sound[0], 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
            if (ball.y >= player2.y - ball.radius && ball.y < player2.y + 5 + ball.radius) {
                ball.speedY = 8;

            }
            if (ball.y >= player2.y + 5 + ball.radius && ball.y < player2.y + 45 + ball.radius) {
                ball.speedY = 4;

            }
            if (ball.y >= player2.y + 45 + ball.radius && ball.y < player2.y + 55 + ball.radius) {
                ball.speedY = 0;

            }
            if (ball.y >= player2.y + 55 + ball.radius && ball.y < player2.y + 95 + ball.radius) {
                ball.speedY = -4;

            }
            if (ball.y >= player2.y + 95 + ball.radius && ball.y <= player2.y + 100 + ball.radius) {
                ball.speedY = -8;

            }
            ball.speedX = -ball.speedX + 1;
            ball.x = player2.x + ball.radius;
        }
     
    }
    //jesli znajdzie sie za graczem to zalicza punkt

    if (ball.x > DISP_W - 30 + ball.radius) {
        al_play_sample(game_sound[2], 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
        direction = -1;
        player2.score++;
        ball_init();
    }

    if (ball.x < 30 - ball.radius) {
        al_play_sample(game_sound[2], 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
        direction = 1;
        player.score++;
        ball_init();
    }

    //kolizja ze sciana
    if (ball.y >= DISP_H - 15 - ball.radius) {
        al_play_sample(game_sound[1], 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
        ball.speedY = -ball.speedY;
        ball.y = DISP_H - 15 - ball.radius;
    }
    if (ball.y <= 15 + ball.radius) {
        al_play_sample(game_sound[1], 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
        ball.speedY = -ball.speedY;
        ball.y = 15 + ball.radius;
    }

}

//wypisuje wynik oraz sprawdza warunki zwyciestwa
void draw_score() {
    al_draw_textf(font, al_map_rgb(255, 255, 255), DISP_W * 0.75, 15, 0, "%d", player.score);
    al_draw_textf(font, al_map_rgb(255, 255, 255), DISP_W * 0.25, 15, 0, "%d", player2.score);
    if (player.score == max_score) {
        end = true;
        winner = 1;
    }
    if (player2.score == max_score) {
        end = true;
        winner = 2;
    }
}

//rysuje plansze
void draw_level() {
    al_draw_line(DISP_W / 2, 0, DISP_W / 2, DISP_H / 2 - 12, al_map_rgb(255, 255, 255), 2);
    al_draw_circle(DISP_W / 2, DISP_H / 2, 12, al_map_rgb(255, 255, 255), 4);
    al_draw_line(DISP_W / 2, DISP_H / 2 + 12, DISP_W / 2, DISP_H, al_map_rgb(255, 255, 255), 2);

    //gorna i dolna granica
    al_draw_filled_rectangle(0, 0, DISP_W, 15, al_map_rgb(242, 229, 215));
    al_draw_filled_rectangle(0, DISP_H, DISP_W, DISP_H - 15, al_map_rgb(242, 229, 215));
}



FILE* settings;

//wczytuje z pliku kolory
//nie sprawdza czy udalo sie wczytac bo i tak scrashuje jezeli brakuje plikow
int load_settings() {
    settings = fopen("files/settings.txt", "r");
    for(int i = 0; i < 7; i++) {
        fscanf(settings, "%d,", &set_arr[i]);
    }
    player.colorR = set_arr[0]; //r
    player.colorG = set_arr[1]; //g
    player.colorB = set_arr[2]; //b
    player2.colorR = set_arr[3]; //r
    player2.colorG = set_arr[4]; //g
    player2.colorB = set_arr[5]; //b

    //po wczytaniu sprawdza .colorB i ustawia zmienna na dany kolor
    if (player.colorB == 255) {
        p1 = 1;
    }
    else if (player.colorB == 99) {
        p1 = 2;
    }
    else if (player.colorB == 68) {
        p1 = 3;
    }
    else if (player.colorB == 158) {
        p1 = 4;
    }
    else if (player.colorB == 207) {
        p1 = 5;
    }
    else if (player.colorB == 201) {
        p1 = 6;
    }

    if (player2.colorB == 255) {
        p2 = 1;
    }
    else if (player2.colorB == 99) {
        p2 = 2;
    }
    else if (player2.colorB == 68) {
        p2 = 3;
    }
    else if (player2.colorB == 158) {
        p2 = 4;
    }
    else if (player2.colorB == 207) {
        p2 = 5;
    }
    else if (player2.colorB == 201) {
        p2 = 6;
    }

    max_score = set_arr[6];
    fclose(settings);
}

//zapisuje kolory do pliku
void save_settings() {
    settings = fopen("files/settings.txt", "w");
    fprintf(settings, "%d %d %d %d %d %d %d", player.colorR, player.colorG, player.colorB, player2.colorR, player2.colorG, player2.colorB, max_score);
    fclose(settings);
}

//przywraca domyslne wartosci po restarcie
void restart_game() {
    time_t t;
    time(&t);
    //podczas restartu gry zapisuje wyniki do scores.txt jezeli byl jakis zwyciezca i gra zostala rozegrana do konca
    if (end == true && winner == 1) {
        fprintf(scores, "\n%s --- Player 1 wins! --- Player 1 score: %d Player 2 score: %d \n", ctime(&t), player.score, player2.score);
    }
    if (end == true && winner == 2) {
        fprintf(scores, "\n%s --- Player 2 wins! --- Player 1 score: %d Player 2 score: %d \n", ctime(&t), player.score, player2.score);
    }
    
    player_init();
    ball_init();
    winner = 0;
    start = false;
    end = false;
    restart = false;
}


int main() {
    all_init();
    load_settings();
    ALLEGRO_TIMER* timer = al_create_timer(1.0 / FPS);
    must_init(timer, "timer");

    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
    must_init(queue, "queue");

    ALLEGRO_DISPLAY* disp = al_create_display(DISP_W, DISP_H);
    must_init(disp, "display");

    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_display_event_source(disp));
    al_register_event_source(queue, al_get_timer_event_source(timer));
     
    open_file();
    player_init();
    ball_init();
    keyboard_init();
    font_init();
    audio();

    bool done = false;
    bool redraw = true;
    

    ALLEGRO_EVENT event;

    al_start_timer(timer);
    while (true) {
        al_wait_for_event(queue, &event);

        switch (event.type) {
        case ALLEGRO_EVENT_TIMER:
            //wylacza gre po wcisnieciu ESC
            //if (key[ALLEGRO_KEY_ESCAPE])
            //    done = true;

            if (main_screen == true) {
                //jezeli gra trwa, odczytuje input gracza
                if (end == false) {
                    player_update();
                }
                if (end == false && ai == false) {
                    player2_update();
                }

                //rozpoczyna gre i zatrzymuje pilke na koniec (end)
                if (key[ALLEGRO_KEY_LEFT] || key[ALLEGRO_KEY_RIGHT] || key[ALLEGRO_KEY_A] || key[ALLEGRO_KEY_D]) {
                    start = true;
                }
                if (start == true && end == false) {
                    ball_update();
                }

                //restart gry
                if (restart == true) {
                    restart_game();

                }
                if (ai == true) {
                    computer_player();
                }
            }

            for (int i = 0; i < ALLEGRO_KEY_MAX; i++)
                key[i] &= KEY_SEEN;

            redraw = true;
            break;

        case ALLEGRO_EVENT_KEY_DOWN:
            key[event.keyboard.keycode] = KEY_SEEN | KEY_RELEASED;
            //wraca do menu, z menu wylacza program
            if (key[ALLEGRO_KEY_ESCAPE]) {
                if (main_screen == false) {
                    done = true;
                }
                else {
                    restart_game();
                    ai = false;
                    main_screen = false;
                }
            }
            //przelacznik gry z komputerem
            if (key[ALLEGRO_KEY_SPACE]) {
                if (main_screen == true) {
                    if (ai == false) {
                        ai = true;
                    }
                    else ai = false;
                }
            }
            //jezeli gra sie nie rozpoczela i uzytkownik jest wciaz na ekranie startowym
            if (start == false && main_screen == false) {
                //ustawianie max wyniku gry
                if (key[ALLEGRO_KEY_3]) {
                    max_score = 3;
                }
                if (key[ALLEGRO_KEY_6]) {
                    max_score = 6;
                }
                if (key[ALLEGRO_KEY_9]) {
                    max_score = 9;
                }
                //kolor tla
                if (key[ALLEGRO_KEY_E]) {
                    p3++;
                }
                if (key[ALLEGRO_KEY_Q]) {
                    p3--;
                }
                //zmiany kolorkow player
                if (key[ALLEGRO_KEY_RIGHT]) {
                    p1++;
                }
                if (key[ALLEGRO_KEY_LEFT]) {
                    p1--;
                }
                //zmiany kolorkow player2
                if (key[ALLEGRO_KEY_D]) {
                    p2++;
                }
                if (key[ALLEGRO_KEY_A]) {
                    p2--;
                }
            }
            //przelacza na glowny ekran gry po wyborze koloru
            if (key[ALLEGRO_KEY_ENTER]) {
                save_settings();
                main_screen = true;
            }

            //restart gry
            if (key[ALLEGRO_KEY_R]) {
                restart = true;
            }

            break;
        case ALLEGRO_EVENT_KEY_UP:
            key[event.keyboard.keycode] &= KEY_RELEASED;
            break;

        case ALLEGRO_EVENT_DISPLAY_CLOSE:
            done = true;
            break;
        }
        if (done) {
            break;
        }

        if (redraw && al_is_event_queue_empty(queue)) {
            al_clear_to_color(al_map_rgb(DISP_R, DISP_G, DISP_B));
            if (main_screen == true) {

                draw_player();
                draw_ball();
                draw_level();
                draw_score();

                //po prostu wyswietla napisy
                if (ai == false) {
                    al_draw_textf(font_small, al_map_rgb(255, 255, 255), DISP_W / 8, DISP_H - 100, 0, "SPACE - PLAY WITH PC");
                } else al_draw_textf(font_small, al_map_rgb(255, 255, 255), DISP_W / 8, DISP_H - 100, 0, "SPACE - PLAYER 2");
                if (start == false) {
                    al_draw_textf(font_small, al_map_rgb(255, 255, 255), DISP_W / 8, DISP_H - 75, 0, "R - RESTART");
                }
                
            }

            //wyswietla ekran startowy
            if (ai == false && start == false && main_screen == false) {
                player_color();
               
            }

            //jesli mecz sie zakonczyl i jest zwyciezca
            if (end == true) {
                if (winner == 1) {
                    al_draw_textf(font, al_map_rgb(255, 255, 255), DISP_W * 0.15 + 45, DISP_H - 300, 0, "Player 1 wins!");
                }
                else {
                    al_draw_textf(font, al_map_rgb(255, 255, 255), DISP_W * 0.15 + 45, DISP_H - 300, 0, "Player 2 wins!");  
                }
                al_draw_textf(font, al_map_rgb(255, 255, 255), DISP_W * 0.15 + 45, DISP_H - 250, 0, "Press R to reset");
            }

            al_flip_display();

            redraw = false;
        }

    }

    close_file();
    font_deinit();
    audio_deinit();
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);
    al_destroy_display(disp);
    al_destroy_font(font);
    al_destroy_font(font_big);
    al_destroy_font(font_small);
    al_destroy_sample(game_sound[0]);
    al_destroy_sample(game_sound[1]);
    al_destroy_sample(game_sound[2]);
    return 0;
}
