#include "display.h"
#include "tree.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static SDL_Window*   window     = NULL;
static SDL_Renderer* renderer   = NULL;
static int           paused     = 0;
static Uint32        start_time = 0;

//signal pour la touche RELANCER : 1 quand l'utilisateur clique dessus
static int relance_demandee = 0;

static int    scroll_x      = 0;
static int    scroll_y      = 0;
static int    drag          = 0;
static int    drag_start_x  = 0;
static int    drag_start_y  = 0;
static int    drag_scroll_x = 0;
static int    drag_scroll_y = 0;

//facteur de zoom (1.0 = taille normale)
static double zoom = 1.0;
#define ZOOM_MIN   0.1
#define ZOOM_MAX   5.0
#define ZOOM_STEP  1.2

extern int LONGEUR_MAXIMALE;

static void draw_full_tree_dyn(Noeud* node, int x, int y,
                                int x_min, int x_max, int level_h);
void close_display(void);

//Police bitmap 5x7
static const unsigned char font5x7[][7] = {
    {0x0E,0x11,0x11,0x1F,0x11,0x11,0x11},/*A*/
    {0x1E,0x11,0x11,0x1E,0x11,0x11,0x1E},/*B*/
    {0x0E,0x11,0x10,0x10,0x10,0x11,0x0E},/*C*/
    {0x1E,0x11,0x11,0x11,0x11,0x11,0x1E},/*D*/
    {0x1F,0x10,0x10,0x1E,0x10,0x10,0x1F},/*E*/
    {0x1F,0x10,0x10,0x1E,0x10,0x10,0x10},/*F*/
    {0x0E,0x11,0x10,0x17,0x11,0x11,0x0F},/*G*/
    {0x11,0x11,0x11,0x1F,0x11,0x11,0x11},/*H*/
    {0x0E,0x04,0x04,0x04,0x04,0x04,0x0E},/*I*/
    {0x07,0x02,0x02,0x02,0x02,0x12,0x0C},/*J*/
    {0x11,0x12,0x14,0x18,0x14,0x12,0x11},/*K*/
    {0x10,0x10,0x10,0x10,0x10,0x10,0x1F},/*L*/
    {0x11,0x1B,0x15,0x11,0x11,0x11,0x11},/*M*/
    {0x11,0x19,0x15,0x13,0x11,0x11,0x11},/*N*/
    {0x0E,0x11,0x11,0x11,0x11,0x11,0x0E},/*O*/
    {0x1E,0x11,0x11,0x1E,0x10,0x10,0x10},/*P*/
    {0x0E,0x11,0x11,0x11,0x15,0x12,0x0D},/*Q*/
    {0x1E,0x11,0x11,0x1E,0x14,0x12,0x11},/*R*/
    {0x0F,0x10,0x10,0x0E,0x01,0x01,0x1E},/*S*/
    {0x1F,0x04,0x04,0x04,0x04,0x04,0x04},/*T*/
    {0x11,0x11,0x11,0x11,0x11,0x11,0x0E},/*U*/
    {0x11,0x11,0x11,0x11,0x11,0x0A,0x04},/*V*/
    {0x11,0x11,0x11,0x15,0x15,0x1B,0x11},/*W*/
    {0x11,0x11,0x0A,0x04,0x0A,0x11,0x11},/*X*/
    {0x11,0x11,0x0A,0x04,0x04,0x04,0x04},/*Y*/
    {0x1F,0x01,0x02,0x04,0x08,0x10,0x1F},/*Z*/
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00},/*spc*/
    {0x00,0x00,0x00,0x00,0x00,0x00,0x06},/*.*/
    {0x00,0x00,0x00,0x0E,0x00,0x00,0x00},/*-*/
    {0x04,0x04,0x04,0x04,0x04,0x00,0x04},/*!*/
    {0x0E,0x11,0x13,0x15,0x19,0x11,0x0E},/*0*/
    {0x04,0x0C,0x04,0x04,0x04,0x04,0x0E},/*1*/
    {0x0E,0x11,0x01,0x06,0x08,0x10,0x1F},/*2*/
    {0x1F,0x01,0x02,0x06,0x01,0x11,0x0E},/*3*/
    {0x02,0x06,0x0A,0x12,0x1F,0x02,0x02},/*4*/
    {0x1F,0x10,0x1E,0x01,0x01,0x11,0x0E},/*5*/
    {0x06,0x08,0x10,0x1E,0x11,0x11,0x0E},/*6*/
    {0x1F,0x01,0x02,0x04,0x08,0x08,0x08},/*7*/
    {0x0E,0x11,0x11,0x0E,0x11,0x11,0x0E},/*8*/
    {0x0E,0x11,0x11,0x0F,0x01,0x02,0x0C} /*9*/
};

static void draw_char(int x, int y, char c, int scale) {
    int idx;
    if      (c>='A'&&c<='Z') idx=c-'A';
    else if (c>='a'&&c<='z') idx=c-'a';
    else if (c==' ')          idx=26;
    else if (c=='.')          idx=27;
    else if (c=='-')          idx=28;
    else if (c=='!')          idx=29;
    else if (c>='0'&&c<='9') idx=30+(c-'0');
    else                      idx=26;
    for (int row=0; row<7; row++)
        for (int col=0; col<5; col++)
            if (font5x7[idx][row] & (1<<(4-col))) {
                SDL_Rect r = { x+col*scale, y+row*scale, scale, scale };
                SDL_RenderFillRect(renderer, &r);
            }
}

static void draw_text(int x, int y, const char* t,
                       int scale, int r, int g, int b) {
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    int cx = x;
    for (int i=0; t[i]; i++) { draw_char(cx, y, t[i], scale); cx += 6*scale; }
}

static void draw_button(int x, int y, int w, int h,
                         int br, int bg, int bb, const char* label) {
    SDL_SetRenderDrawColor(renderer, br, bg, bb, 255);
    SDL_Rect rect = {x, y, w, h};
    SDL_RenderFillRect(renderer, &rect);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &rect);
    int sc=2, tw=(int)strlen(label)*6*sc;
    draw_text(x+(w-tw)/2, y+(h-7*sc)/2, label, sc, 255,255,255);
}

static void draw_circle(int cx, int cy, int r) {
    for (int dy=-r; dy<=r; dy++) {
        int dx = (int)SDL_sqrt((double)(r*r-dy*dy));
        SDL_RenderDrawLine(renderer, cx-dx, cy+dy, cx+dx, cy+dy);
    }
}

static int tree_depth(Noeud* node) {
    if (!node) return 0;
    if (node->pere == NULL) {
        int l = tree_depth(node->filsG);
        int r = tree_depth(node->filsD);
        return l>r ? l : r;
    }
    int l = tree_depth(node->filsG);
    int r = tree_depth(node->filsD);
    return (l>r ? l : r) + 1;
}

//Compte le nombre de feuilles d'un sous-arbre
static int count_leaves(Noeud* node) {
    if (!node) return 0;
    if (!node->filsG && !node->filsD) return 1;
    return count_leaves(node->filsG) + count_leaves(node->filsD);
}

//Applique le zoom a une coordonnee logique
//Les coordonnees x,y sont en espace "arbre" et on les transforme
//en coordonnees ecran en appliquant zoom + scroll autour du centre horizontal.
static int apply_zoom_x(int x) {
    int cx = WIN_W / 2;
    return (int)((x - cx) * zoom) + cx - scroll_x;
}
static int apply_zoom_y(int y) {
    return (int)(y * zoom) - scroll_y;
}

//Test : un point (mx,my) est-il dans la zone d'un bouton ?
static int in_btn(int mx, int my, int bx, int by, int bw, int bh) {
    return (mx>=bx && mx<=bx+bw && my>=by && my<=by+bh);
}

//Dessin recursif de l'arbre avec layout proportionnel aux feuilles
//Chaque noeud recoit un intervalle horizontal [x_min, x_max] dans lequel
//il doit tenir. Cet intervalle est reparti entre ses deux fils en
//proportion du nombre de feuilles de chaque sous-arbre.
//
//Couleurs :
//  vert  (0,210,80)   : solution == 1  (feuille valide)
//  rouge (180,30,30)  : dead == 1      (feuille morte)
//  bleu  (30,100,220) : noeud interne  (ni solution ni mort)
//  gris  (120,120,120): racine fictive
static void draw_full_tree_dyn(Noeud* node, int x, int y,
                                int x_min, int x_max, int level_h) {
    if (!node) return;

    int sx      = apply_zoom_x(x);
    int sy      = apply_zoom_y(y);
    int child_y = y + level_h;
    int csy     = apply_zoom_y(child_y);

    //repartition horizontale proportionnelle au nombre de feuilles
    int leaves_G = count_leaves(node->filsG);
    int leaves_D = count_leaves(node->filsD);
    int total    = leaves_G + leaves_D;
    if (total == 0) total = 1;

    int split = x_min + (x_max - x_min) * leaves_G / total;
    int x_G   = (x_min + split) / 2;
    int x_D   = (split + x_max) / 2;

    //traits vers les fils
    if (node->filsG) {
        int is_root = (node->pere == NULL);
        SDL_SetRenderDrawColor(renderer,
            is_root ? 150 : (node->filsG->dead ? 180 : 150),
            is_root ? 150 : (node->filsG->dead ? 30  : 150),
            is_root ? 150 : (node->filsG->dead ? 30  : 150), 255);
        SDL_RenderDrawLine(renderer, sx, sy, apply_zoom_x(x_G), csy);
    }
    if (node->filsD) {
        int is_root = (node->pere == NULL);
        SDL_SetRenderDrawColor(renderer,
            is_root ? 150 : (node->filsD->dead ? 180 : 150),
            is_root ? 150 : (node->filsD->dead ? 30  : 150),
            is_root ? 150 : (node->filsD->dead ? 30  : 150), 255);
        SDL_RenderDrawLine(renderer, sx, sy, apply_zoom_x(x_D), csy);
    }

    //dessin du noeud courant
    int visible = (sx > -200 && sx < WIN_W+200 && sy > -200 && sy < WIN_H+200);
    if (visible) {
        if (node->pere == NULL) {
            //racine fictive
            SDL_SetRenderDrawColor(renderer, 120,120,120,255);
            int r = (int)(NODE_R * zoom);
            if (r < 2) r = 2;
            draw_circle(sx, sy, r);
            if (r >= 6) draw_text(sx-3, sy-3, "R", 1, 255,255,255);
        } else {
            //priorite des couleurs :
            //1. solution == 1 -> vert  (feuille valide, rang final atteint)
            //2. dead == 1     -> rouge (aucun fils valide trouve)
            //3. sinon          bleu  (noeud interne en cours de construction)
            if (node->solution)
                SDL_SetRenderDrawColor(renderer, 0,210,80,255);
            else if (node->dead)
                SDL_SetRenderDrawColor(renderer, 180,30,30,255);
            else
                SDL_SetRenderDrawColor(renderer, 30,100,220,255);

            //taille du cercle : combine l'espace horizontal disponible et le zoom
            int slot = (int)((x_max - x_min) * zoom);
            int r;
            if      (slot < 20) r = 4;
            else if (slot < 40) r = 7;
            else if (slot < 80) r = 10;
            else                r = NODE_R;
            //appliquer le zoom sur le rayon de base
            r = (int)(r * zoom);
            if (r < 2) r = 2;
            draw_circle(sx, sy, r);

            if (r >= 8) {
                char buf[16];
                int v=node->valeur, neg=(v<0), av=neg?-v:v, bi=0, tl=0;
                char tmp[10];
                if (av==0) tmp[tl++]='0';
                else while (av>0) { tmp[tl++]='0'+(av%10); av/=10; }
                if (neg) buf[bi++]='-';
                for (int i=tl-1; i>=0; i--) buf[bi++]=tmp[i];
                buf[bi]='\0';
                //echelle du texte : 1 par defaut, 2 si le zoom est fort
                int txt_scale = (zoom >= 1.8) ? 2 : 1;
                draw_text(sx-(bi*6*txt_scale)/2, sy-3*txt_scale, buf,
                          txt_scale, 255,255,255);
            }
        }
    }

    //recursion avec les sous-intervalles
    if (node->filsG)
        draw_full_tree_dyn(node->filsG, x_G, child_y, x_min, split, level_h);
    if (node->filsD)
        draw_full_tree_dyn(node->filsD, x_D, child_y, split, x_max, level_h);
}

//Dessin des trois boutons (PAUSE/REPRENDRE, RELANCER, QUITTER)
static void draw_all_buttons(int is_final) {
    //bouton PAUSE / REPRENDRE : meme bouton, label dynamique
    //orange quand on peut mettre en pause, vert quand on peut reprendre
    //masque pendant l'affichage final puisqu'il n'y a plus rien a mettre en pause
    if (!is_final) {
        if (paused) {
            draw_button(BTN_PAUSE_X, BTN_Y, BTN_W, BTN_H,
                        0, 180, 60, "REPRENDRE");
        } else {
            draw_button(BTN_PAUSE_X, BTN_Y, BTN_W, BTN_H,
                        220, 120, 0, "PAUSE");
        }
    }
    //bouton RELANCER (bleu)
    draw_button(BTN_RELANCE_X, BTN_Y, BTN_W, BTN_H,
                30, 100, 200, "RELANCER");
    //bouton QUITTER (rouge)
    draw_button(BTN_QUITTER_X, BTN_Y, BTN_W, BTN_H,
                180, 30, 30, "QUITTER");
}

//Rendu complet
static void render(Noeud* root, int rank, int total,
                    int is_final, int nb_sol) {
    SDL_SetRenderDrawColor(renderer, 0,0,30,255);
    SDL_RenderClear(renderer);

    if (is_final) {
        draw_text(20, 22, "RESULTAT FINAL", 2, 255,200,50);
        char msg[32];
        msg[0]='S'; msg[1]='O'; msg[2]='L'; msg[3]=':'; msg[4]=' ';
        int mi=5, mv=nb_sol; char mt[8]; int mtl=0;
        if (mv==0) { mt[mtl++]='0'; }
        else while (mv>0) { mt[mtl++]='0'+(mv%10); mv/=10; }
        for (int i=mtl-1; i>=0; i--) msg[mi++]=mt[i];
        msg[mi]='\0';
        draw_text(220, 22, msg, 2, (nb_sol==1)?0:230, (nb_sol==1)?210:120, 0);
    } else {
        char buf[32];
        buf[0]='R'; buf[1]='A'; buf[2]='N'; buf[3]='G'; buf[4]=' ';
        int bi=5, rv=rank; char tmp[10]; int tl=0;
        if (rv==0) { tmp[tl++]='0'; }
        else while (rv>0) { tmp[tl++]='0'+(rv%10); rv/=10; }
        for (int i=tl-1; i>=0; i--) buf[bi++]=tmp[i];
        buf[bi]='/'; bi++;
        int tv=total; tl=0;
        if (tv==0) { tmp[tl++]='0'; }
        else while (tv>0) { tmp[tl++]='0'+(tv%10); tv/=10; }
        for (int i=tl-1; i>=0; i--) buf[bi++]=tmp[i];
        buf[bi]='\0';
        draw_text(20, 22, buf, 2, 255,200,50);
    }

    //les trois boutons (PAUSE/REPRENDRE est masque pendant l'affichage final)
    draw_all_buttons(is_final);

    //legende
    SDL_SetRenderDrawColor(renderer,0,210,80,255);
    draw_circle(WIN_W-180,30,8);
    draw_text(WIN_W-165,24,"SOLUTION",1,200,200,200);
    SDL_SetRenderDrawColor(renderer,30,100,220,255);
    draw_circle(WIN_W-180,48,8);
    draw_text(WIN_W-165,42,"NOEUD",1,200,200,200);
    SDL_SetRenderDrawColor(renderer,180,30,30,255);
    draw_circle(WIN_W-180,66,8);
    draw_text(WIN_W-165,60,"MORT",1,200,200,200);

    //affichage du niveau de zoom
    char zbuf[16];
    int zpct = (int)(zoom * 100);
    int zi = 0;
    zbuf[zi++]='Z'; zbuf[zi++]='O'; zbuf[zi++]='O'; zbuf[zi++]='M'; zbuf[zi++]=' ';
    char ztmp[8]; int ztl=0; int zv = zpct;
    if (zv==0) ztmp[ztl++]='0';
    else while (zv>0) { ztmp[ztl++]='0'+(zv%10); zv/=10; }
    for (int i=ztl-1; i>=0; i--) zbuf[zi++]=ztmp[i];
    zbuf[zi]='\0';
    draw_text(WIN_W-180, 84, zbuf, 1, 200,200,200);

    draw_text(20,WIN_H-44,"CLIC DROIT+GLISSER ou FLECHES : naviguer",1,120,120,120);
    draw_text(20,WIN_H-30,"MOLETTE : scroll  |  CTRL+MOLETTE ou .-./- : zoom",1,120,120,120);
    draw_text(20,WIN_H-16,"R : reset vue  |  0 : reset zoom",1,120,120,120);

    if (root) {
        int depth = tree_depth(root);
        if (depth<1) depth=1;
        int lh=(WIN_H-180)/(depth+1);
        if (lh>LEVEL_H) lh=LEVEL_H;
        if (lh<15)      lh=15;

        //calcul de la largeur totale necessaire :
        //on garantit au moins 40 pixels par feuille pour eviter les chevauchements
        int nb_feuilles = count_leaves(root);
        if (nb_feuilles < 1) nb_feuilles = 1;
        int largeur_totale = nb_feuilles * 40;
        if (largeur_totale < WIN_W - 40) largeur_totale = WIN_W - 40;

        int x_center = WIN_W / 2;
        int x_min = x_center - largeur_totale / 2;
        int x_max = x_center + largeur_totale / 2;
        //l'arbre commence sous la barre de boutons
        draw_full_tree_dyn(root, x_center, 130, x_min, x_max, lh);
    }

    SDL_RenderPresent(renderer);
}

//Applique un zoom autour du point (mx, my) de l'ecran
//On veut que le point sous le curseur reste au meme endroit apres le zoom,
//ce qui est plus naturel qu'un zoom autour du centre.
static void apply_zoom_at(int mx, int my, double factor) {
    double new_zoom = zoom * factor;
    if (new_zoom < ZOOM_MIN) new_zoom = ZOOM_MIN;
    if (new_zoom > ZOOM_MAX) new_zoom = ZOOM_MAX;
    if (new_zoom == zoom) return;

    //coordonnees logiques (espace arbre) sous le curseur avant zoom
    int cx = WIN_W / 2;
    double world_x = (mx + scroll_x - cx) / zoom + cx;
    double world_y = (my + scroll_y) / zoom;

    zoom = new_zoom;

    //ajuster scroll pour que (mx,my) reste fixe a l'ecran apres zoom
    scroll_x = (int)((world_x - cx) * zoom + cx - mx);
    scroll_y = (int)(world_y * zoom - my);
}

//Gere un clic gauche sur un des boutons.
//is_final : 1 si on est en affichage final (pas de bouton PAUSE).
//Retourne 1 si on doit sortir de la boucle d'evenements (clic RELANCER en final).
static int handle_button_click(int mx, int my, int is_final) {
    //bouton PAUSE/REPRENDRE (uniquement pendant la construction)
    if (!is_final && in_btn(mx, my, BTN_PAUSE_X, BTN_Y, BTN_W, BTN_H)) {
        paused = !paused;
        fprintf(stderr, "%s\n", paused ? "PAUSE" : "REPRISE");
        fflush(stderr);
        return 0;
    }
    //bouton RELANCER
    if (in_btn(mx, my, BTN_RELANCE_X, BTN_Y, BTN_W, BTN_H)) {
        relance_demandee = 1;
        fprintf(stderr, "RELANCER demande\n"); fflush(stderr);
        return 1;
    }
    //bouton QUITTER
    if (in_btn(mx, my, BTN_QUITTER_X, BTN_Y, BTN_W, BTN_H)) {
        fprintf(stderr, "QUITTER\n"); fflush(stderr);
        close_display();
        exit(0);
    }
    return 0;
}

//Gestion des evenements
static void handle_events(Noeud* root, int rank) {
	(void)root;   //non utilises pour le moment, conserves pour usages futurs
	(void)rank;
    SDL_Event e;
    Uint32 now = SDL_GetTicks();
    while (SDL_PollEvent(&e)) {
        if (now-start_time<300) continue;
        if (e.type==SDL_QUIT) { close_display(); exit(0); } // @suppress("Symbol is not resolved")
        if (e.type==SDL_KEYDOWN) { // @suppress("Symbol is not resolved")
            switch(e.key.keysym.sym) {
                case SDLK_ESCAPE: close_display(); exit(0); // @suppress("Symbol is not resolved")
                case SDLK_LEFT:   scroll_x-=50; break; // @suppress("Symbol is not resolved")
                case SDLK_RIGHT:  scroll_x+=50; break; // @suppress("Symbol is not resolved")
                case SDLK_UP:     scroll_y-=50; break; // @suppress("Symbol is not resolved")
                case SDLK_DOWN:   scroll_y+=50; break; // @suppress("Symbol is not resolved")
                case SDLK_r:      scroll_x=0; scroll_y=0; break; // @suppress("Symbol is not resolved")
                case SDLK_0:      zoom=1.0; scroll_x=0; scroll_y=0; break; // @suppress("Symbol is not resolved")
                case SDLK_PLUS: // @suppress("Symbol is not resolved")
                case SDLK_EQUALS: // @suppress("Symbol is not resolved")
                case SDLK_KP_PLUS: // @suppress("Symbol is not resolved")
                    apply_zoom_at(WIN_W/2, WIN_H/2, ZOOM_STEP);
                    break;
                case SDLK_MINUS: // @suppress("Symbol is not resolved")
                case SDLK_KP_MINUS: // @suppress("Symbol is not resolved")
                    apply_zoom_at(WIN_W/2, WIN_H/2, 1.0/ZOOM_STEP);
                    break;
                default: break;
            }
        }
        if (e.type==SDL_MOUSEWHEEL) { // @suppress("Symbol is not resolved")
            //si Ctrl est tenu : zoom, sinon : scroll
            const Uint8* kbd = SDL_GetKeyboardState(NULL);
            int ctrl_held = kbd[SDL_SCANCODE_LCTRL] || kbd[SDL_SCANCODE_RCTRL]; // @suppress("Symbol is not resolved")
            if (ctrl_held) {
                int mx, my;
                SDL_GetMouseState(&mx, &my);
                if (e.wheel.y > 0)      apply_zoom_at(mx, my, ZOOM_STEP);
                else if (e.wheel.y < 0) apply_zoom_at(mx, my, 1.0/ZOOM_STEP);
            } else {
                scroll_y-=e.wheel.y*40;
                scroll_x-=e.wheel.x*40;
            }
        }
        if (e.type==SDL_MOUSEBUTTONDOWN && e.button.button==SDL_BUTTON_RIGHT) { // @suppress("Symbol is not resolved")
            drag=1; drag_start_x=e.button.x; drag_start_y=e.button.y;
            drag_scroll_x=scroll_x; drag_scroll_y=scroll_y;
        }
        if (e.type==SDL_MOUSEBUTTONUP && e.button.button==SDL_BUTTON_RIGHT) drag=0; // @suppress("Symbol is not resolved")
        if (e.type==SDL_MOUSEMOTION && drag) { // @suppress("Symbol is not resolved")
            scroll_x=drag_scroll_x-(e.motion.x-drag_start_x);
            scroll_y=drag_scroll_y-(e.motion.y-drag_start_y);
        }
        if (e.type==SDL_MOUSEBUTTONDOWN && e.button.button==SDL_BUTTON_LEFT) { // @suppress("Symbol is not resolved")
            //pendant la construction : is_final = 0
            handle_button_click(e.button.x, e.button.y, 0);
        }
    }
}

//Initialisation SDL2
void init_display(void) {
    fprintf(stderr,"Initialisation SDL2...\n"); fflush(stderr);
    if (SDL_Init(SDL_INIT_VIDEO)!=0) {
        fprintf(stderr,"SDL_Init: %s\n",SDL_GetError()); return;
    }
    window=SDL_CreateWindow("Dequantification - Arbre Binaire",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIN_W, WIN_H, SDL_WINDOW_SHOWN); // @suppress("Symbol is not resolved")
    if (!window) { fprintf(stderr,"Window: %s\n",SDL_GetError()); SDL_Quit(); return; }
    renderer=SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED); // @suppress("Symbol is not resolved")
    if (!renderer) {
        fprintf(stderr,"Renderer: %s\n",SDL_GetError());
        SDL_DestroyWindow(window); SDL_Quit(); return;
    }
    start_time=SDL_GetTicks();
    //reset des flags a chaque init (utile en cas de relance)
    relance_demandee = 0;
    paused = 0;
    scroll_x = 0; scroll_y = 0; zoom = 1.0;
    fprintf(stderr,"SDL2 OK\n"); fflush(stderr);
}

//Mise a jour pendant la construction
void update_display(void* root_void, int current_rank) {
    if (!renderer) return;
    Noeud* root = (Noeud*)root_void;
    handle_events(root, current_rank);
    while (paused) {
        handle_events(root, current_rank);
        render(root, current_rank, LONGEUR_MAXIMALE, 0, 0);
        SDL_Delay(30);
    }
    render(root, current_rank, LONGEUR_MAXIMALE, 0, 0);
    SDL_Delay(DELAY_NODE_MS);
}

//Affichage final
void display_final(void* root_void, int nb_solutions) {
    if (!renderer) return;
    Noeud* root = (Noeud*)root_void;
    SDL_Event e;
    while (SDL_PollEvent(&e)) {}
    SDL_Delay(200);
    while (SDL_PollEvent(&e)) {}
    render(root,1,1,1,nb_solutions);
    fprintf(stderr,"Ferme la fenetre ou clique RELANCER/QUITTER.\n"); fflush(stderr);
    int running=1;
    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type==SDL_QUIT) running=0; // @suppress("Symbol is not resolved")
            if (e.type==SDL_KEYDOWN && e.key.keysym.sym==SDLK_ESCAPE) running=0; // @suppress("Symbol is not resolved")
            if (e.type==SDL_KEYDOWN) {// @suppress("Symbol is not resolved")
                switch(e.key.keysym.sym) {
                    case SDLK_LEFT:  scroll_x-=50; break;// @suppress("Symbol is not resolved")
                    case SDLK_RIGHT: scroll_x+=50; break;// @suppress("Symbol is not resolved")
                    case SDLK_UP:    scroll_y-=50; break;// @suppress("Symbol is not resolved")
                    case SDLK_DOWN:  scroll_y+=50; break;// @suppress("Symbol is not resolved")
                    case SDLK_r:     scroll_x=0; scroll_y=0; break;// @suppress("Symbol is not resolved")
                    case SDLK_0:     zoom=1.0; scroll_x=0; scroll_y=0; break;// @suppress("Symbol is not resolved")
                    case SDLK_PLUS:// @suppress("Symbol is not resolved")
                    case SDLK_EQUALS:// @suppress("Symbol is not resolved")
                    case SDLK_KP_PLUS:// @suppress("Symbol is not resolved")
                        apply_zoom_at(WIN_W/2, WIN_H/2, ZOOM_STEP); break;
                    case SDLK_MINUS:// @suppress("Symbol is not resolved")
                    case SDLK_KP_MINUS:// @suppress("Symbol is not resolved")
                        apply_zoom_at(WIN_W/2, WIN_H/2, 1.0/ZOOM_STEP); break;
                    default: break;
                }
            }
            if (e.type==SDL_MOUSEWHEEL) {// @suppress("Symbol is not resolved")
                const Uint8* kbd = SDL_GetKeyboardState(NULL);
                int ctrl_held = kbd[SDL_SCANCODE_LCTRL] || kbd[SDL_SCANCODE_RCTRL];// @suppress("Symbol is not resolved")
                if (ctrl_held) {
                    int mx, my;
                    SDL_GetMouseState(&mx, &my);
                    if (e.wheel.y > 0)      apply_zoom_at(mx, my, ZOOM_STEP);
                    else if (e.wheel.y < 0) apply_zoom_at(mx, my, 1.0/ZOOM_STEP);
                } else {
                    scroll_y-=e.wheel.y*40;
                    scroll_x-=e.wheel.x*40;
                }
            }
            if (e.type==SDL_MOUSEBUTTONDOWN && e.button.button==SDL_BUTTON_RIGHT) {// @suppress("Symbol is not resolved")
                drag=1; drag_start_x=e.button.x; drag_start_y=e.button.y;
                drag_scroll_x=scroll_x; drag_scroll_y=scroll_y;
            }
            if (e.type==SDL_MOUSEBUTTONUP && e.button.button==SDL_BUTTON_RIGHT) drag=0;// @suppress("Symbol is not resolved")
            if (e.type==SDL_MOUSEMOTION && drag) {// @suppress("Symbol is not resolved")
                scroll_x=drag_scroll_x-(e.motion.x-drag_start_x);
                scroll_y=drag_scroll_y-(e.motion.y-drag_start_y);
            }
            if (e.type==SDL_MOUSEBUTTONDOWN && e.button.button==SDL_BUTTON_LEFT) {// @suppress("Symbol is not resolved")
                //pendant l'affichage final : is_final = 1
                if (handle_button_click(e.button.x, e.button.y, 1)) {
                    //RELANCER a ete clique : on sort de la boucle
                    running = 0;
                }
            }
        }
        render(root,1,1,1,nb_solutions);
        SDL_Delay(30);
    }
}

//Permet au main de savoir si l'utilisateur a demande une relance
int display_relance_demandee(void) {
    int r = relance_demandee;
    relance_demandee = 0;  //consomme le flag
    return r;
}

//Liberation SDL2
void close_display(void) {
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window)   SDL_DestroyWindow(window);
    SDL_Quit();
    renderer=NULL; window=NULL;
}
