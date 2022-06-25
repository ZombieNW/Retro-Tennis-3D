/*===========================================
                 Retro Tennis 3d
                   - ZombieNW -

                      GRRLIB
=============================================*/
#include <grrlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <wiiuse/wpad.h>
#include <time.h>
#include <asndlib.h>
#include <mp3player.h>

//Fonts
#include "LCDSolid_ttf.h"
#include "Rubik_ttf.h"
//Backgrounds
#include "options_scrolling_background_png.h"
#include "options_static_background_png.h"
#include "options_static_decor_png.h"
#include "titlebg_png.h"
#include "tbg1_png.h"
#include "tbg2_png.h"
#include "tbg3_png.h"
#include "tbg4_png.h"
#include "tbg5_png.h"
#include "credits_png.h"
//Pointers
#include "player1_point_png.h"
#include "player2_point_png.h"
#include "player1_openhand_png.h"
#include "player2_openhand_png.h"
//Buttons
#include "a_button_png.h"
#include "b_button_png.h"
#include "triangle_button_png.h"
#include "triangle_button_highlighted_png.h"
#include "left_triangle_button_png.h"
#include "left_triangle_button_highlighted_png.h"
//Decor
#include "logo_png.h"
#include "p1mote_png.h"
#include "p2mote_png.h"
#include "popup_png.h"
//Intro Images
#include "logo_intro_png.h"
#include "wii_warning_png.h"
//Music
#include "optionsmusic_mp3.h"
#include "titlemusic_mp3.h"
#include "bgm1_mp3.h"
#include "bgm2_mp3.h"
//Sounds
#include "win_mp3.h"
#include "score_pcm.h"
#include "bounce_pcm.h"
#include "click_pcm.h"
#include "one_button_pcm.h"
#include "start_pcm.h"

//Functions
float RandomFloat(float a, float b) { //a:min b:max
    float random = ((float) rand()) / (float) RAND_MAX;
    float diff = b - a;
    float r = random * diff;
    return a + r;
}

void ExitGame(){
    GRRLIB_Exit(); // Be a good boy, clear the memory allocated by GRRLIB
    SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);//Return to the system menu
}

//For power pressed callbacks
s8 HWButton = -1;
void WiiResetPressed()
{
	HWButton = SYS_RESTART;
}
void WiiPowerPressed()
{
	HWButton = SYS_POWEROFF;
}
void WiimotePowerPressed(s32 chan)
{
	HWButton = SYS_POWEROFF_IDLE;
}

int main() {
    // Initialise everything
    GRRLIB_Init();
    WPAD_Init();
    ASND_Init(INIT_RATE_48000);
	MP3Player_Init();
    srand((unsigned int)time(NULL));//Init rand seed
    WPAD_SetIdleTimeout(75);//Timeout Wiimotes after 60 seconds
    WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR);//Something?
    WPAD_SetDataFormat(WPAD_CHAN_1, WPAD_FMT_BTNS_ACC_IR);//Something?
    ir_t P1Mote; //Player 1 wiimote
    ir_t P2Mote; //Player 2 wiimote
    SYS_SetResetCallback(WiiResetPressed);
	SYS_SetPowerCallback(WiiPowerPressed);
	WPAD_SetPowerButtonCallback(WiimotePowerPressed);

    //Adjustable Vars
    int gsbgS = 1; //scrolling background speed
    float pspeed = 0.05;//players speed
    float cpuspeed = 0.03;//cpu speed, usually what controlls the difficulty as well as all speed
    float bspeed = 0.05;//ball max speed
    int players = 1;//Ammount of active players, does NOT start at 0
    int gamemode = 0;//Menus
    //0 = Safety And Info Screen
    //1 = Developer Logo
    //2 = Title Screen
    //3 = Options Menu
    //4 = Gameplay
    //5 = Pause Screen
    int pausestatus = 0;
    /*Pause Satuses
    0 = unpaused
    1 = normal pause
    2 = player 1 won
    3 = player 2 won
    4 = credits
    */

    //Vars
    u16 WinW = rmode->fbWidth; //Screen Width
    u16 WinH = rmode->efbHeight; //Screen Height
    int wiimote1state = 0;//if openhand or pointer icon
    int wiimote2state = 0;//if openhand or pointer icon
    int gsbgW = 0; //Scrolling background width
    float camrotx = 0;//camera x
    float camroty = 0;//camera y
    float camrotz = 5;//camera z
    float p1y = 0;//player 1 y pos
    float p2y = 0;//player 2 y pos
    float p1x = -15;//player 1 x pos
    float p2x = 15;//player 2 x pos
    float p1w = 0.16;//player width
    float p1h = 1.0;//player height
    float p1z = 1;//player base
    int p1s = 0;//player 1 score
    int p2s = 0;//player 2 score
    float ballX = 0;//ball x pos
    float ballY = 0;//ball y pos
    float ballW = 0.1;//ball width
    float ballspeedX = RandomFloat(0.03,bspeed); //Ball horitzontal speed. Changes when contacts borders or Bars.
    float ballspeedY = RandomFloat(0.01,bspeed); //Ball vertical speed. Changes when contacts borders or Bars.
    int cpuselectscreenchoice = 2;//menus
    int gameplayselector = 1;//menus
    int selectorcooldown = 0;//cool down for item selection with pointer
    int rumbletime1 = 0;//for creating rumbles player 1
    int rumbletime2 = 0;//for creating rumbles player 2
    int wiiwarningcooldown = 300;//intro warning/logo timer
    int bgmchoice = 0;//randomly choosing what bgm plays during gameplay
    int aupstate = 1;//0 = down, 1 = up
    int bupstate = 1;//0 = down, 1 = up
    int stopdamusic = 0;//if music on the title screen should be stopped
    int waitbeforeoptionmenu = -10;//waiting before changing gamemode on title screen, its -10 so it's not at 0 and wont set off the change

    //Textures
    GRRLIB_ttfFont *LCDSolidFont = GRRLIB_LoadTTF(LCDSolid_ttf, LCDSolid_ttf_size);//Init the lcdsolid font
    GRRLIB_ttfFont *RubikFont = GRRLIB_LoadTTF(Rubik_ttf, Rubik_ttf_size);//Init the rubik font
    GRRLIB_texImg *GFX_tbg1 = GRRLIB_LoadTexturePNG(tbg1_png);
    GRRLIB_texImg *GFX_tbg2 = GRRLIB_LoadTexturePNG(tbg2_png);
    GRRLIB_texImg *GFX_tbg3 = GRRLIB_LoadTexturePNG(tbg3_png);
    GRRLIB_texImg *GFX_tbg4 = GRRLIB_LoadTexturePNG(tbg4_png);
    GRRLIB_texImg *GFX_tbg5 = GRRLIB_LoadTexturePNG(tbg5_png);
    GRRLIB_texImg *GFX_logo = GRRLIB_LoadTexturePNG(logo_png);
    GRRLIB_texImg *GFX_titlebg = GRRLIB_LoadTexturePNG(titlebg_png);
    GRRLIB_texImg *GFX_options_scrolling_background = GRRLIB_LoadTexturePNG(options_scrolling_background_png);
    GRRLIB_texImg *GFX_options_static_background = GRRLIB_LoadTexturePNG(options_static_background_png);
    GRRLIB_texImg *GFX_options_static_decor = GRRLIB_LoadTexturePNG(options_static_decor_png);
    GRRLIB_texImg *GFX_wii_warning_texture = GRRLIB_LoadTexturePNG(wii_warning_png);
    GRRLIB_texImg *GFX_logo_intro_texture = GRRLIB_LoadTexturePNG(logo_intro_png);
    GRRLIB_texImg *GFX_player1_point = GRRLIB_LoadTexturePNG(player1_point_png);
    GRRLIB_texImg *GFX_player2_point = GRRLIB_LoadTexturePNG(player2_point_png);
    GRRLIB_texImg *GFX_p1mote = GRRLIB_LoadTexturePNG(p1mote_png);
    GRRLIB_texImg *GFX_p2mote = GRRLIB_LoadTexturePNG(p2mote_png);
    GRRLIB_texImg *GFX_popup = GRRLIB_LoadTexturePNG(popup_png);
    GRRLIB_texImg *GFX_credits = GRRLIB_LoadTexturePNG(credits_png);
    GRRLIB_texImg *GFX_a_button = GRRLIB_LoadTexturePNG(a_button_png);
    GRRLIB_texImg *GFX_b_button = GRRLIB_LoadTexturePNG(b_button_png);
    GRRLIB_texImg *GFX_player1_openhand = GRRLIB_LoadTexturePNG(player1_openhand_png);
    GRRLIB_texImg *GFX_player2_openhand = GRRLIB_LoadTexturePNG(player2_openhand_png);
    GRRLIB_texImg *GFX_triangle_button_texture = GRRLIB_LoadTexturePNG(triangle_button_png);
    GRRLIB_texImg *GFX_triangle_button_highlighted_texture = GRRLIB_LoadTexturePNG(triangle_button_highlighted_png);
    GRRLIB_texImg *GFX_left_triangle_button_texture = GRRLIB_LoadTexturePNG(left_triangle_button_png);
    GRRLIB_texImg *GFX_left_triangle_button_highlighted_texture = GRRLIB_LoadTexturePNG(left_triangle_button_highlighted_png);
    GRRLIB_SetMidHandle(GFX_player1_point, true);
    GRRLIB_SetMidHandle(GFX_player2_point, true);
    GRRLIB_SetMidHandle(GFX_player1_openhand, true);
    GRRLIB_SetMidHandle(GFX_player2_openhand, true);
    GRRLIB_SetMidHandle(GFX_logo, true);
    GRRLIB_SetMidHandle(GFX_popup, true);
    GRRLIB_SetMidHandle(GFX_logo_intro_texture, true);
    GRRLIB_SetMidHandle(GFX_wii_warning_texture, true);

    //Init grrlib bg and camera
    GRRLIB_SetBackgroundColour(0x00, 0x00, 0x00, 0xFF);//background color
    GRRLIB_Camera3dSettings(0.0f,0.0f,5.0f, 0,1,0, 0,0,0);//3d camera settings (pos x, pos y, pos z, alpha, alpha, alpha, look x, look y, look z)
    GRRLIB_Settings.antialias = true;//Enable anti-aliasing

    //Functions
    bool player1hittingball(){
        if(ballX < -2.25 + (p1w / 2) && ballX > -2.25 - (p1w / 2)){
            if(ballY < p1y + (p1h / 2) && ballY > p1y - (p1h / 2)){
                return true;
            }
        }
        return false;
    }
    bool player2hittingball(){
        if(ballX < 2.25 + (p1w / 2) && ballX > 2.25 - (p1w / 2)){
            if(ballY < p2y + (p1h / 2) && ballY > p2y - (p1h / 2)){
                return true;
            }
        }
        return false;
    }
    void ResetBall(){
        ballX = 0;//ball x pos
        ballY = 0;//ball y pos
        bool tf = true; if ((rand() % 2) + 1 ==  2) tf = false;
        if(tf == true){
            ballspeedX = RandomFloat(0.01,bspeed); //Ball horitzontal speed. Changes when contacts borders or Bars.
            ballspeedY = RandomFloat(-0.01,-bspeed); //Ball vertical speed. Changes when contacts borders or Bars.
        }
        else{
            ballspeedX = RandomFloat(-0.01,-bspeed); //Ball horitzontal speed. Changes when contacts borders or Bars.
            ballspeedY = RandomFloat(0.01,bspeed); //Ball vertical speed. Changes when contacts borders or Bars.
        }
    }
    void ResetCamera(){
        camrotx = 0;//camera x
        camroty = 0;//camera y
        camrotz = 5;//camera z
    }
    void StartGame(){
        selectorcooldown = 10;
        gamemode = 4;
        p1s = 0;
        p2s = 0;
        p1y = 0;//player 1 y pos
        p2y = 0;//player 2 y pos
        MP3Player_Stop();
        ResetBall();
        bool tf = true; if ((rand() % 2) + 1 ==  2) tf = false;
        if(tf == true){
            bgmchoice = 1;
        }
        else{
            bgmchoice = 0;
        }
    }

    //Loop
    while(1) {
        if(HWButton != -1)
			break;
        WPAD_ScanPads();//Get inputs
        WPAD_IR(WPAD_CHAN_0, &P1Mote); //player 1 IR Pointer
        WPAD_IR(WPAD_CHAN_1, &P2Mote); //player 2 IR Pointer
        int P1MX = P1Mote.sx - 200;// WiiMote IR Viewport Correction
        int P1MY = P1Mote.sy - 150;// WiiMote IR Viewport Correction
        int P2MX = P2Mote.sx - 200;// WiiMote IR Viewport Correction
        int P2MY = P2Mote.sy - 150;// WiiMote IR Viewport Correction
        if(rumbletime1 > 0){//timer for rumble player 1
            WPAD_Rumble(0, 1);
            rumbletime1--;
        }
        else{
            WPAD_Rumble(0, 0);
        }
        if(rumbletime2 > 0){//timer for rumble player 2
            WPAD_Rumble(1, 1);
            rumbletime2--;
        }
        else{
            WPAD_Rumble(1, 0);
        }
        if(selectorcooldown > 0){//timer for inputs
            selectorcooldown--;
        }
        if(waitbeforeoptionmenu > 0){//timer for waiting before the options menu to hear the sound effect
            waitbeforeoptionmenu--;
        }
        if(waitbeforeoptionmenu == 0){
            gamemode = 3;
        }
        if(gamemode == 0){//Safety Warning
            if(wiiwarningcooldown > 0){
                GRRLIB_DrawImg(WinW / 2, WinH / 2, GFX_wii_warning_texture, 0, 0.76, 1, 0xFFFFFFFF);
                wiiwarningcooldown--;
            }
            else{
                gamemode = 1;
                wiiwarningcooldown = 300;
            }
        }
        if(gamemode == 1){//Logo
            if(wiiwarningcooldown > 0){
                GRRLIB_DrawImg(WinW / 2, WinH / 2, GFX_logo_intro_texture, 0, 0.76, 1, 0xFFFFFFFF);
                wiiwarningcooldown--;
            }
            else{
                gamemode = 2;
                wiiwarningcooldown = 300;
            }
        }
        if(gamemode == 2){//Title Screen
            if(MP3Player_IsPlaying() == false && stopdamusic == 0){
                MP3Player_Stop();
                MP3Player_PlayBuffer(titlemusic_mp3, titlemusic_mp3_size, NULL);
            }
            if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME)  ExitGame();//close game when home pressed
            //Scrolling Background
            GRRLIB_DrawImg(gsbgW - 128, 0, GFX_tbg5, 0, 1, 1, 0xFFFFFFFF);//Draw Two bg(5)
            GRRLIB_DrawImg(gsbgW - 256, 0, GFX_tbg4, 0, 1, 1, 0xFFFFFFFF);//Draw Two bg(4)
            GRRLIB_DrawImg(gsbgW - 384, 0, GFX_tbg3, 0, 1, 1, 0xFFFFFFFF);//Draw Two bg(3)
            GRRLIB_DrawImg(gsbgW - 512, 0, GFX_tbg2, 0, 1, 1, 0xFFFFFFFF);//Draw Two bg(2)
            GRRLIB_DrawImg(gsbgW - 640, 0, GFX_tbg1, 0, 1, 1, 0xFFFFFFFF);//Draw Two bg(1)
            GRRLIB_DrawImg(gsbgW, 0, GFX_tbg1, 0, 1, 1, 0xFFFFFFFF);//Draw One bg (1)
            GRRLIB_DrawImg(gsbgW + 128, 0, GFX_tbg2, 0, 1, 1, 0xFFFFFFFF);//Draw Two bg(2)
            GRRLIB_DrawImg(gsbgW + 256, 0, GFX_tbg3, 0, 1, 1, 0xFFFFFFFF);//Draw Two bg(3)
            GRRLIB_DrawImg(gsbgW + 384, 0, GFX_tbg4, 0, 1, 1, 0xFFFFFFFF);//Draw Two bg(4)
            GRRLIB_DrawImg(gsbgW + 512, 0, GFX_tbg5, 0, 1, 1, 0xFFFFFFFF);//Draw Two bg(5)
            gsbgW += gsbgS;
            if(gsbgW == WinW){
                gsbgW = 0;
            }
            GRRLIB_DrawImg(WinW/2, WinH/2, GFX_logo, 0, 0.8, 0.8, 0xFFFFFFFF);//Logo
            GRRLIB_PrintfTTF(80, 400, RubikFont, "Press       and       to get started!", 32, 0x404040FF);//Press _ and _ to get started
            if(WPAD_ButtonsHeld(0) & WPAD_BUTTON_A && WPAD_ButtonsHeld(0) & WPAD_BUTTON_B){
                stopdamusic = 1;//prevent title music from being played
                MP3Player_Stop();//^^^
                ASND_SetVoice(ASND_GetFirstUnusedVoice(), VOICE_STEREO_16BIT, 22050, 0, (char *) start_pcm, start_pcm_size, 2000, 2000, NULL);
                // Voice, Format, Pitch, Delay, Sound, Sound Size, Left Audio, Right Audio, Callback
                waitbeforeoptionmenu = 100;//read variable init
            }
            if(WPAD_ButtonsHeld(0) & WPAD_BUTTON_A){
                GRRLIB_DrawImg(165, 395, GFX_a_button, 0, 0.6, 0.6, 0xFFFFFFFF);
            }
            else{
                GRRLIB_DrawImg(170, 400, GFX_a_button, 0, 0.5, 0.5, 0xFFFFFFFF);
            }
            if(WPAD_ButtonsHeld(0) & WPAD_BUTTON_B){
                GRRLIB_DrawImg(275, 395, GFX_b_button, 0, 0.6, 0.6, 0xFFFFFFFF);
            }
            else{
                GRRLIB_DrawImg(280, 400, GFX_b_button, 0, 0.5, 0.5, 0xFFFFFFFF);
            }

            if(WPAD_ButtonsUp(0) & WPAD_BUTTON_A){
                aupstate = 1;
            }
            if(WPAD_ButtonsUp(0) & WPAD_BUTTON_B){
                bupstate = 1;
            }
            if(WPAD_ButtonsDown(0) & WPAD_BUTTON_A && aupstate != 0){
                aupstate = 0;
                ASND_SetVoice(ASND_GetFirstUnusedVoice(), VOICE_STEREO_16BIT, 22050, 0, (char *) one_button_pcm, one_button_pcm_size, 2000, 2000, NULL);
                // Voice, Format, Pitch, Delay, Sound, Sound Size, Left Audio, Right Audio, Callback
                aupstate = 1;
            }
            if(WPAD_ButtonsDown(0) & WPAD_BUTTON_B && bupstate != 0){
                bupstate = 0;
                ASND_SetVoice(ASND_GetFirstUnusedVoice(), VOICE_STEREO_16BIT, 22050, 0, (char *) one_button_pcm, one_button_pcm_size, 2000, 2000, NULL);
                // Voice, Format, Pitch, Delay, Sound, Sound Size, Left Audio, Right Audio, Callback
                bupstate = 1;
            }

            if(WPAD_ButtonsHeld(0) & WPAD_BUTTON_A){
                wiimote1state = 1;
                rumbletime1 = 1;
            }
            else{
                wiimote1state = 0;
            }
            if(WPAD_ButtonsHeld(1) & WPAD_BUTTON_A){
                wiimote2state = 1;
                rumbletime2 = 1;
            }
            else{
                wiimote2state = 0;
            }
            //pointers
            if (P2Mote.state == 1) {//If wii pointer active, try to keep this the last thing rendered
                if(wiimote2state == 1){
                    GRRLIB_DrawImg( P2MX, P2MY, GFX_player2_openhand, P2Mote.angle, 0.85, 0.85, 0xFFFFFFFF);//Draw Wii Pointer
                }
                else{
                    GRRLIB_DrawImg( P2MX, P2MY, GFX_player2_point, P2Mote.angle, 0.85, 0.85, 0xFFFFFFFF);//Draw Wii Pointer
                }
            }
            if (P1Mote.state == 1) {//If wii pointer active, try to keep this the last thing rendered
                if(wiimote1state == 1){
                    GRRLIB_DrawImg( P1MX, P1MY, GFX_player1_openhand, P1Mote.angle, 0.85, 0.85, 0xFFFFFFFF);//Draw Wii Pointer
                }
                else{
                    GRRLIB_DrawImg( P1MX, P1MY, GFX_player1_point, P1Mote.angle, 0.85, 0.85, 0xFFFFFFFF);//Draw Wii Pointer
                }
            }
        }
        if(gamemode == 3){//Options Menu
            waitbeforeoptionmenu = -10;
            if(MP3Player_IsPlaying() == false){
                MP3Player_Stop();
                MP3Player_PlayBuffer(optionsmusic_mp3, optionsmusic_mp3_size, NULL);
            }
            if (WPAD_ButtonsDown(0) & WPAD_BUTTON_MINUS || WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME){
                MP3Player_Stop();
                stopdamusic = 0;
                waitbeforeoptionmenu = -10;
                gamemode = 2;//go to title screen when minus pressed
            }
            if (WPAD_ButtonsDown(0) & WPAD_BUTTON_PLUS){
                StartGame();//start game when plus pressed
            }

            //Scrolling Background
            GRRLIB_DrawImg(gsbgW - 256, 0, GFX_options_scrolling_background, 0, 1, 1, 0xFFFFFFFF);//Draw Two bg
            GRRLIB_DrawImg(gsbgW - 384, 0, GFX_options_scrolling_background, 0, 1, 1, 0xFFFFFFFF);//Draw Two bg
            GRRLIB_DrawImg(gsbgW - 512, 0, GFX_options_scrolling_background, 0, 1, 1, 0xFFFFFFFF);//Draw Two bg
            GRRLIB_DrawImg(gsbgW - 640, 0, GFX_options_scrolling_background, 0, 1, 1, 0xFFFFFFFF);//Draw Two bg
            GRRLIB_DrawImg(gsbgW - 128, 0, GFX_options_scrolling_background, 0, 1, 1, 0xFFFFFFFF);//Draw Two bg
            GRRLIB_DrawImg(gsbgW, 0, GFX_options_scrolling_background, 0, 1, 1, 0xFFFFFFFF);//Draw One bg
            GRRLIB_DrawImg(gsbgW + 128, 0, GFX_options_scrolling_background, 0, 1, 1, 0xFFFFFFFF);//Draw Two bg
            GRRLIB_DrawImg(gsbgW + 256, 0, GFX_options_scrolling_background, 0, 1, 1, 0xFFFFFFFF);//Draw Two bg
            GRRLIB_DrawImg(gsbgW + 384, 0, GFX_options_scrolling_background, 0, 1, 1, 0xFFFFFFFF);//Draw Two bg
            GRRLIB_DrawImg(gsbgW + 512, 0, GFX_options_scrolling_background, 0, 1, 1, 0xFFFFFFFF);//Draw Two bg
            gsbgW += gsbgS;
            if(gsbgW == WinW){
                gsbgW = 0;
            }
            //Static Texture Background
            GRRLIB_DrawImg(0, 0, GFX_options_static_background, 0, 0.76, 1, 0xFFFFFFFF);
            //Decor
            GRRLIB_DrawImg(0, 0, GFX_options_static_decor, 0, 0.76, 1, 0xFFFFFFFF);
            //Options
            //CPU SELECTOR
            if(true){//CPU SELECTOR
                if(cpuselectscreenchoice == 1){
                    //text lmao
                    GRRLIB_PrintfTTF(175, 325, RubikFont, "Easy", 32, 0x000000FF); //Players Sign
                    //increase
                    if(GRRLIB_PtInRect(300, 300, 102, 102, P1MX, P1MY)){//If button is being hovered
                        GRRLIB_DrawImg(300, 300, GFX_triangle_button_highlighted_texture, 0, 0.4, 0.4, 0xFFFFFFFF);//Draw the button highlighted
                        if(WPAD_ButtonsDown(0) & WPAD_BUTTON_A){
                            if(selectorcooldown == 0){
                                cpuselectscreenchoice = 2;
                                selectorcooldown = 10;
                                cpuspeed = 0.03;
                                bspeed = 0.05;
                                ASND_SetVoice(ASND_GetFirstUnusedVoice(), VOICE_STEREO_16BIT, 22050, 0, (char *) click_pcm, click_pcm_size, 2000, 2000, NULL);
                                // Voice, Format, Pitch, Delay, Sound, Sound Size, Left Audio, Right Audio, Callback
                            }
                        }
                    }
                    else{//If button is not hovered
                        GRRLIB_DrawImg(300, 300, GFX_triangle_button_texture, 0, 0.4, 0.4, 0xFFFFFFFF);//Draw the button not highlighted
                    }
                    //decrease
                    if(GRRLIB_PtInRect(50, 300, 102, 102, P1MX, P1MY)){//If button is being hovered
                        GRRLIB_DrawImg(50, 300, GFX_left_triangle_button_highlighted_texture, 0, 0.4, 0.4, 0xFFFFFFFF);//Draw the button highlighted
                        if(WPAD_ButtonsDown(0) & WPAD_BUTTON_A){
                            if(selectorcooldown == 0){
                                cpuselectscreenchoice = 4;
                                selectorcooldown = 10;
                                cpuspeed = 0.20;
                                bspeed = 0.20;
                                ASND_SetVoice(ASND_GetFirstUnusedVoice(), VOICE_STEREO_16BIT, 22050, 0, (char *) click_pcm, click_pcm_size, 2000, 2000, NULL);
                                // Voice, Format, Pitch, Delay, Sound, Sound Size, Left Audio, Right Audio, Callback
                            }
                        }
                    }
                    else{//If button is not hovered
                        GRRLIB_DrawImg(50, 300, GFX_left_triangle_button_texture, 0, 0.4, 0.4, 0xFFFFFFFF);//Draw the button not highlighted
                    }
                }
                if(cpuselectscreenchoice == 2){
                    //text lmao
                    GRRLIB_PrintfTTF(170, 325, RubikFont, "Normal", 32, 0x000000FF); //Players Sign
                    //increase
                    if(GRRLIB_PtInRect(300, 300, 102, 102, P1MX, P1MY)){//If button is being hovered
                        GRRLIB_DrawImg(300, 300, GFX_triangle_button_highlighted_texture, 0, 0.4, 0.4, 0xFFFFFFFF);//Draw the button highlighted
                        if(WPAD_ButtonsDown(0) & WPAD_BUTTON_A){
                            if(selectorcooldown == 0){
                                cpuselectscreenchoice = 3;
                                selectorcooldown = 10;
                                cpuspeed = 0.04;
                                bspeed = 0.07;
                                ASND_SetVoice(ASND_GetFirstUnusedVoice(), VOICE_STEREO_16BIT, 22050, 0, (char *) click_pcm, click_pcm_size, 2000, 2000, NULL);
                                // Voice, Format, Pitch, Delay, Sound, Sound Size, Left Audio, Right Audio, Callback
                            }
                        }
                    }
                    else{//If button is not hovered
                        GRRLIB_DrawImg(300, 300, GFX_triangle_button_texture, 0, 0.4, 0.4, 0xFFFFFFFF);//Draw the button not highlighted
                    }
                    //decrease
                    if(GRRLIB_PtInRect(50, 300, 102, 102, P1MX, P1MY)){//If button is being hovered
                        GRRLIB_DrawImg(50, 300, GFX_left_triangle_button_highlighted_texture, 0, 0.4, 0.4, 0xFFFFFFFF);//Draw the button highlighted
                        if(WPAD_ButtonsDown(0) & WPAD_BUTTON_A){
                            if(selectorcooldown == 0){
                                cpuselectscreenchoice = 1;
                                selectorcooldown = 10;
                                cpuspeed = 0.01;
                                bspeed = 0.02;
                                ASND_SetVoice(ASND_GetFirstUnusedVoice(), VOICE_STEREO_16BIT, 22050, 0, (char *) click_pcm, click_pcm_size, 2000, 2000, NULL);
                                // Voice, Format, Pitch, Delay, Sound, Sound Size, Left Audio, Right Audio, Callback
                            }
                        }
                    }
                    else{//If button is not hovered
                        GRRLIB_DrawImg(50, 300, GFX_left_triangle_button_texture, 0, 0.4, 0.4, 0xFFFFFFFF);//Draw the button not highlighted
                    }
                }
                if(cpuselectscreenchoice == 3){
                    //text lmao
                    GRRLIB_PrintfTTF(175, 325, RubikFont, "Hard", 32, 0x000000FF); //Players Sign
                    //increase
                    if(GRRLIB_PtInRect(300, 300, 102, 102, P1MX, P1MY)){//If button is being hovered
                        GRRLIB_DrawImg(300, 300, GFX_triangle_button_highlighted_texture, 0, 0.4, 0.4, 0xFFFFFFFF);//Draw the button highlighted
                        if(WPAD_ButtonsDown(0) & WPAD_BUTTON_A){
                            if(selectorcooldown == 0){
                                cpuselectscreenchoice = 4;
                                selectorcooldown = 10;
                                cpuspeed = 0.20;
                                bspeed = 0.20;
                                ASND_SetVoice(ASND_GetFirstUnusedVoice(), VOICE_STEREO_16BIT, 22050, 0, (char *) click_pcm, click_pcm_size, 2000, 2000, NULL);
                                // Voice, Format, Pitch, Delay, Sound, Sound Size, Left Audio, Right Audio, Callback
                            }
                        }
                    }
                    else{//If button is not hovered
                        GRRLIB_DrawImg(300, 300, GFX_triangle_button_texture, 0, 0.4, 0.4, 0xFFFFFFFF);//Draw the button not highlighted
                    }
                    //decrease
                    if(GRRLIB_PtInRect(50, 300, 102, 102, P1MX, P1MY)){//If button is being hovered
                        GRRLIB_DrawImg(50, 300, GFX_left_triangle_button_highlighted_texture, 0, 0.4, 0.4, 0xFFFFFFFF);//Draw the button highlighted
                        if(WPAD_ButtonsDown(0) & WPAD_BUTTON_A){
                            if(selectorcooldown == 0){
                                cpuselectscreenchoice = 2;
                                selectorcooldown = 10;
                                cpuspeed = 0.03;
                                bspeed = 0.05;
                                ASND_SetVoice(ASND_GetFirstUnusedVoice(), VOICE_STEREO_16BIT, 22050, 0, (char *) click_pcm, click_pcm_size, 2000, 2000, NULL);
                                // Voice, Format, Pitch, Delay, Sound, Sound Size, Left Audio, Right Audio, Callback
                            }
                        }
                    }
                    else{//If button is not hovered
                        GRRLIB_DrawImg(50, 300, GFX_left_triangle_button_texture, 0, 0.4, 0.4, 0xFFFFFFFF);//Draw the button not highlighted
                    }
                }
                if(cpuselectscreenchoice == 4){
                    GRRLIB_PrintfTTF(175, 325, RubikFont, "Kaizo", 32, 0x000000FF); //Players Sign
                    //increase
                    if(GRRLIB_PtInRect(300, 300, 102, 102, P1MX, P1MY)){//If button is being hovered
                        GRRLIB_DrawImg(300, 300, GFX_triangle_button_highlighted_texture, 0, 0.4, 0.4, 0xFFFFFFFF);//Draw the button highlighted
                        if(WPAD_ButtonsDown(0) & WPAD_BUTTON_A){
                            if(selectorcooldown == 0){
                                cpuselectscreenchoice = 1;
                                selectorcooldown = 10;
                                cpuspeed = 0.01;
                                bspeed = 0.02;
                                ASND_SetVoice(ASND_GetFirstUnusedVoice(), VOICE_STEREO_16BIT, 22050, 0, (char *) click_pcm, click_pcm_size, 2000, 2000, NULL);
                                // Voice, Format, Pitch, Delay, Sound, Sound Size, Left Audio, Right Audio, Callback
                            }
                        }
                    }
                    else{//If button is not hovered
                        GRRLIB_DrawImg(300, 300, GFX_triangle_button_texture, 0, 0.4, 0.4, 0xFFFFFFFF);//Draw the button not highlighted
                    }
                    //decrease
                    if(GRRLIB_PtInRect(50, 300, 102, 102, P1MX, P1MY)){//If button is being hovered
                        GRRLIB_DrawImg(50, 300, GFX_left_triangle_button_highlighted_texture, 0, 0.4, 0.4, 0xFFFFFFFF);//Draw the button highlighted
                        if(WPAD_ButtonsDown(0) & WPAD_BUTTON_A){
                            if(selectorcooldown == 0){
                                cpuselectscreenchoice = 3;
                                selectorcooldown = 10;
                                cpuspeed = 0.04;
                                bspeed = 0.07;
                                ASND_SetVoice(ASND_GetFirstUnusedVoice(), VOICE_STEREO_16BIT, 22050, 0, (char *) click_pcm, click_pcm_size, 2000, 2000, NULL);
                                // Voice, Format, Pitch, Delay, Sound, Sound Size, Left Audio, Right Audio, Callback
                            }
                        }
                    }
                    else{//If button is not hovered
                        GRRLIB_DrawImg(50, 300, GFX_left_triangle_button_texture, 0, 0.4, 0.4, 0xFFFFFFFF);//Draw the button not highlighted
                    }
                }
            }
            //GAME SELECTOR
            if(true){//GAME SELECTOR
                if(gameplayselector == 1){
                    //text lmao
                    GRRLIB_PrintfTTF(150, 150, RubikFont, "Freeplay", 32, 0x000000FF); //Players Sign
                    //increase
                    if(GRRLIB_PtInRect(300, 125, 102, 102, P1MX, P1MY)){//If button is being hovered
                        GRRLIB_DrawImg(300, 125, GFX_triangle_button_highlighted_texture, 0, 0.4, 0.4, 0xFFFFFFFF);//Draw the button highlighted
                        if(WPAD_ButtonsDown(0) & WPAD_BUTTON_A){
                            if(selectorcooldown == 0){
                                gameplayselector = 2;
                                selectorcooldown = 10;
                                ASND_SetVoice(ASND_GetFirstUnusedVoice(), VOICE_STEREO_16BIT, 22050, 0, (char *) click_pcm, click_pcm_size, 2000, 2000, NULL);
                                // Voice, Format, Pitch, Delay, Sound, Sound Size, Left Audio, Right Audio, Callback
                            }
                        }
                    }
                    else{//If button is not hovered
                        GRRLIB_DrawImg(300, 125, GFX_triangle_button_texture, 0, 0.4, 0.4, 0xFFFFFFFF);//Draw the button not highlighted
                    }
                    //decrease
                    if(GRRLIB_PtInRect(50, 125, 102, 102, P1MX, P1MY)){//If button is being hovered
                        GRRLIB_DrawImg(50, 125, GFX_left_triangle_button_highlighted_texture, 0, 0.4, 0.4, 0xFFFFFFFF);//Draw the button highlighted
                        if(WPAD_ButtonsDown(0) & WPAD_BUTTON_A){
                            if(selectorcooldown == 0){
                                gameplayselector = 4;
                                selectorcooldown = 10;
                                ASND_SetVoice(ASND_GetFirstUnusedVoice(), VOICE_STEREO_16BIT, 22050, 0, (char *) click_pcm, click_pcm_size, 2000, 2000, NULL);
                                // Voice, Format, Pitch, Delay, Sound, Sound Size, Left Audio, Right Audio, Callback
                            }
                        }
                    }
                    else{//If button is not hovered
                        GRRLIB_DrawImg(50, 125, GFX_left_triangle_button_texture, 0, 0.4, 0.4, 0xFFFFFFFF);//Draw the button not highlighted
                    }
                }
                if(gameplayselector == 2){
                    //text lmao
                    GRRLIB_PrintfTTF(170, 150, RubikFont, "5 Point", 32, 0x000000FF); //Players Sign
                    //increase
                    if(GRRLIB_PtInRect(300, 125, 102, 102, P1MX, P1MY)){//If button is being hovered
                        GRRLIB_DrawImg(300, 125, GFX_triangle_button_highlighted_texture, 0, 0.4, 0.4, 0xFFFFFFFF);//Draw the button highlighted
                        if(WPAD_ButtonsDown(0) & WPAD_BUTTON_A){
                            if(selectorcooldown == 0){
                                gameplayselector = 3;
                                selectorcooldown = 10;
                                ASND_SetVoice(ASND_GetFirstUnusedVoice(), VOICE_STEREO_16BIT, 22050, 0, (char *) click_pcm, click_pcm_size, 2000, 2000, NULL);
                                // Voice, Format, Pitch, Delay, Sound, Sound Size, Left Audio, Right Audio, Callback
                            }
                        }
                    }
                    else{//If button is not hovered
                        GRRLIB_DrawImg(300, 125, GFX_triangle_button_texture, 0, 0.4, 0.4, 0xFFFFFFFF);//Draw the button not highlighted
                    }
                    //decrease
                    if(GRRLIB_PtInRect(50, 125, 102, 102, P1MX, P1MY)){//If button is being hovered
                        GRRLIB_DrawImg(50, 125, GFX_left_triangle_button_highlighted_texture, 0, 0.4, 0.4, 0xFFFFFFFF);//Draw the button highlighted
                        if(WPAD_ButtonsDown(0) & WPAD_BUTTON_A){
                            if(selectorcooldown == 0){
                                gameplayselector = 1;
                                selectorcooldown = 10;
                                ASND_SetVoice(ASND_GetFirstUnusedVoice(), VOICE_STEREO_16BIT, 22050, 0, (char *) click_pcm, click_pcm_size, 2000, 2000, NULL);
                                // Voice, Format, Pitch, Delay, Sound, Sound Size, Left Audio, Right Audio, Callback
                            }
                        }
                    }
                    else{//If button is not hovered
                        GRRLIB_DrawImg(50, 125, GFX_left_triangle_button_texture, 0, 0.4, 0.4, 0xFFFFFFFF);//Draw the button not highlighted
                    }
                }
                if(gameplayselector == 3){
                    //text lmao
                    GRRLIB_PrintfTTF(150, 150, RubikFont, "10 Point", 32, 0x000000FF); //Players Sign
                    //increase
                    if(GRRLIB_PtInRect(300, 125, 102, 102, P1MX, P1MY)){//If button is being hovered
                        GRRLIB_DrawImg(300, 125, GFX_triangle_button_highlighted_texture, 0, 0.4, 0.4, 0xFFFFFFFF);//Draw the button highlighted
                        if(WPAD_ButtonsDown(0) & WPAD_BUTTON_A){
                            if(selectorcooldown == 0){
                                gameplayselector = 4;
                                selectorcooldown = 10;
                                ASND_SetVoice(ASND_GetFirstUnusedVoice(), VOICE_STEREO_16BIT, 22050, 0, (char *) click_pcm, click_pcm_size, 2000, 2000, NULL);
                                // Voice, Format, Pitch, Delay, Sound, Sound Size, Left Audio, Right Audio, Callback
                            }
                        }
                    }
                    else{//If button is not hovered
                        GRRLIB_DrawImg(300, 125, GFX_triangle_button_texture, 0, 0.4, 0.4, 0xFFFFFFFF);//Draw the button not highlighted
                    }
                    //decrease
                    if(GRRLIB_PtInRect(50, 125, 102, 102, P1MX, P1MY)){//If button is being hovered
                        GRRLIB_DrawImg(50, 125, GFX_left_triangle_button_highlighted_texture, 0, 0.4, 0.4, 0xFFFFFFFF);//Draw the button highlighted
                        if(WPAD_ButtonsDown(0) & WPAD_BUTTON_A){
                            if(selectorcooldown == 0){
                                gameplayselector = 2;
                                selectorcooldown = 10;
                                ASND_SetVoice(ASND_GetFirstUnusedVoice(), VOICE_STEREO_16BIT, 22050, 0, (char *) click_pcm, click_pcm_size, 2000, 2000, NULL);
                                // Voice, Format, Pitch, Delay, Sound, Sound Size, Left Audio, Right Audio, Callback
                            }
                        }
                    }
                    else{//If button is not hovered
                        GRRLIB_DrawImg(50, 125, GFX_left_triangle_button_texture, 0, 0.4, 0.4, 0xFFFFFFFF);//Draw the button not highlighted
                    }
                }
                if(gameplayselector == 4){
                    GRRLIB_PrintfTTF(150, 150, RubikFont, "25 Point", 32, 0x000000FF); //Players Sign
                    //increase
                    if(GRRLIB_PtInRect(300, 125, 102, 102, P1MX, P1MY)){//If button is being hovered
                        GRRLIB_DrawImg(300, 125, GFX_triangle_button_highlighted_texture, 0, 0.4, 0.4, 0xFFFFFFFF);//Draw the button highlighted
                        if(WPAD_ButtonsDown(0) & WPAD_BUTTON_A){
                            if(selectorcooldown == 0){
                                gameplayselector = 1;
                                selectorcooldown = 10;
                                ASND_SetVoice(ASND_GetFirstUnusedVoice(), VOICE_STEREO_16BIT, 22050, 0, (char *) click_pcm, click_pcm_size, 2000, 2000, NULL);
                                // Voice, Format, Pitch, Delay, Sound, Sound Size, Left Audio, Right Audio, Callback
                            }
                        }
                    }
                    else{//If button is not hovered
                        GRRLIB_DrawImg(300, 125, GFX_triangle_button_texture, 0, 0.4, 0.4, 0xFFFFFFFF);//Draw the button not highlighted
                    }
                    //decrease
                    if(GRRLIB_PtInRect(50, 125, 102, 102, P1MX, P1MY)){//If button is being hovered
                        GRRLIB_DrawImg(50, 125, GFX_left_triangle_button_highlighted_texture, 0, 0.4, 0.4, 0xFFFFFFFF);//Draw the button highlighted
                        if(WPAD_ButtonsDown(0) & WPAD_BUTTON_A){
                            if(selectorcooldown == 0){
                                gameplayselector = 3;
                                selectorcooldown = 10;
                                ASND_SetVoice(ASND_GetFirstUnusedVoice(), VOICE_STEREO_16BIT, 22050, 0, (char *) click_pcm, click_pcm_size, 2000, 2000, NULL);
                                // Voice, Format, Pitch, Delay, Sound, Sound Size, Left Audio, Right Audio, Callback
                            }
                        }
                    }
                    else{//If button is not hovered
                        GRRLIB_DrawImg(50, 125, GFX_left_triangle_button_texture, 0, 0.4, 0.4, 0xFFFFFFFF);//Draw the button not highlighted
                    }
                }
            }
            //PLAYER SELECTOR
            if(P2Mote.state){//if 2 wiimotes are connected
                GRRLIB_DrawImg(430, 60, GFX_p1mote, 0, 0.9, 1, 0xFFFFFFFF);
                GRRLIB_DrawImg(500, 60, GFX_p2mote, 0, 0.9, 1, 0xFFFFFFFF);
                GRRLIB_PrintfTTF(380, 380, RubikFont, "2 Players Connected", 24, 0x101010FF);
                players = 2;//for the game
            }
            else{//If 1 wiimote connected
                GRRLIB_DrawImg(460, 60, GFX_p1mote, 0, 0.9, 1, 0xFFFFFFFF);
                GRRLIB_PrintfTTF(400, 380, RubikFont, "1 Player Connected", 24, 0x101010FF);
                players = 1;//for the game
            }
            
            if(WPAD_ButtonsHeld(0) & WPAD_BUTTON_A){
                wiimote1state = 1;
                rumbletime1 = 1;
            }
            else{
                wiimote1state = 0;
            }
            if(WPAD_ButtonsHeld(1) & WPAD_BUTTON_A){
                wiimote2state = 1;
                rumbletime2 = 1;
            }
            else{
                wiimote2state = 0;
            }
            //pointers
            if (P2Mote.state == 1) {//If wii pointer active, try to keep this the last thing rendered
                if(wiimote2state == 1){
                    GRRLIB_DrawImg( P2MX, P2MY, GFX_player2_openhand, P2Mote.angle, 0.85, 0.85, 0xFFFFFFFF);//Draw Wii Pointer
                }
                else{
                    GRRLIB_DrawImg( P2MX, P2MY, GFX_player2_point, P2Mote.angle, 0.85, 0.85, 0xFFFFFFFF);//Draw Wii Pointer
                }
            }
            if (P1Mote.state == 1) {//If wii pointer active, try to keep this the last thing rendered
                if(wiimote1state == 1){
                    GRRLIB_DrawImg( P1MX, P1MY, GFX_player1_openhand, P1Mote.angle, 0.85, 0.85, 0xFFFFFFFF);//Draw Wii Pointer
                }
                else{
                    GRRLIB_DrawImg( P1MX, P1MY, GFX_player1_point, P1Mote.angle, 0.85, 0.85, 0xFFFFFFFF);//Draw Wii Pointer
                }
            }
        }
        if(gamemode == 4){//Gameplay
            if(bgmchoice == 0){
                if(MP3Player_IsPlaying() == false){
                    MP3Player_Stop();
                    MP3Player_PlayBuffer(bgm1_mp3, bgm1_mp3_size, NULL);
                }
            }
            if(bgmchoice == 1){
                if(MP3Player_IsPlaying() == false){
                    MP3Player_Stop();
                    MP3Player_PlayBuffer(bgm2_mp3, bgm2_mp3_size, NULL);
                }
            }
            //Home Goes Back
            if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME && selectorcooldown == 0){
                pausestatus = 5;
                selectorcooldown = 10;
                gamemode = 5;
                MP3Player_Stop();
            }
            if (WPAD_ButtonsDown(0) & WPAD_BUTTON_PLUS && selectorcooldown == 0){
                pausestatus = 5;
                selectorcooldown = 10;
                gamemode = 5;
                MP3Player_Stop();
            }
            //CAMERA
            struct expansion_t data;
            WPAD_Expansion(WPAD_CHAN_0, &data); // Get expansion info from the first wiimote
            if (data.type == WPAD_EXP_NUNCHUK) { // Ensure there's a nunchuk
                int nunchuckX = data.nunchuk.js.pos.x - 128;//Get nunchuck x pos and balance it
                int nunchuckY = data.nunchuk.js.pos.y - 128;//Get nunchuck y pos and balance it
                if(nunchuckX > 30){ //nunchuck right
                    camrotx += 0.1;//rot camera
                }
                if(nunchuckX < -30){ //nunchuck left
                    camrotx -= 0.1;//rot camera
                }
                if(nunchuckY > 30){ //nunchuck up
                    camroty += 0.1;//rot camera
                }
                if(nunchuckY < -30){ //nunchuck down
                    camroty -= 0.1;//rot camera
                }
                if(WPAD_ButtonsHeld(0) & WPAD_NUNCHUK_BUTTON_C){//if C is pressed on nunchuck
                    ResetCamera();//Reset the camera
                }
            }
            //PLAYERS
            //PLAYER 1
            if(WPAD_ButtonsHeld(0) & WPAD_BUTTON_UP && p1y < 1.5f){
                p1y += pspeed;
            }
            if(WPAD_ButtonsHeld(0) & WPAD_BUTTON_DOWN && p1y > -1.5f){
                p1y -= pspeed;
            }
            //PLAYER 2
            if(players == 2){//If 2 Player Mode Is Enabled
                if(WPAD_ButtonsHeld(1) & WPAD_BUTTON_UP && p2y < 1.5f){
                    p2y += pspeed;
                }
                if(WPAD_ButtonsHeld(1) & WPAD_BUTTON_DOWN && p2y > -1.5f){
                    p2y -= pspeed;
                }
            }
            else{//If playing against cpu
                if(p2y < 1.5f || p2y > -1.5f){
                    if (p2y > ballY) p2y += - cpuspeed; 
                    if ((p2y < ballY) && (p2y < 1.5f)) p2y += cpuspeed; 
                }
            }
            //Ball Controls
            ballX += ballspeedX; //Set the new speed/direction of the ball X
            ballY += ballspeedY; //Set the new speed/direction of the ball Y
            if(ballY > 2){//If the ball bounces off the top
                ballspeedY = RandomFloat(-0.01,-bspeed);
            }
            if(ballY < -2){//If the ball bounces off the bottom
                ballspeedY = RandomFloat(0.01,bspeed);
            }
            if(ballX > 5){//If Player 1 Scores
                p1s++;
                ASND_SetVoice(ASND_GetFirstUnusedVoice(), VOICE_STEREO_16BIT, 22050, 0, (char *) score_pcm, score_pcm_size, 2000, 2000, NULL);
                // Voice, Format, Pitch, Delay, Sound, Sound Size, Left Audio, Right Audio, Callback
                ResetBall();
            }
            if(ballX < -5){//If Player 2 Scores
                p2s++;
                ASND_SetVoice(ASND_GetFirstUnusedVoice(), VOICE_STEREO_16BIT, 22050, 0, (char *) score_pcm, score_pcm_size, 2000, 2000, NULL);
                // Voice, Format, Pitch, Delay, Sound, Sound Size, Left Audio, Right Audio, Callback
                ResetBall();
            }
            //Bounce off the players
            if (player1hittingball() == true){
                ASND_SetVoice(ASND_GetFirstUnusedVoice(), VOICE_STEREO_16BIT, 22050, 0, (char *) bounce_pcm, bounce_pcm_size, 2000, 2000, NULL);
                // Voice, Format, Pitch, Delay, Sound, Sound Size, Left Audio, Right Audio, Callback
                ballspeedX = RandomFloat(0.02,bspeed);//Send it to the right on X
                if(WPAD_ButtonsHeld(0) & WPAD_BUTTON_DOWN || WPAD_ButtonsHeld(0) & WPAD_BUTTON_UP){//if actively moving
                    if(WPAD_ButtonsHeld(0) & WPAD_BUTTON_DOWN){//if going down
                        ballspeedY = RandomFloat(-0.01,-bspeed);//send it down on y
                    }
                    else if(WPAD_ButtonsHeld(0) & WPAD_BUTTON_UP){//if going up
                        ballspeedY = RandomFloat(0.01,bspeed);//send it up on y
                    }
                }
                else{//if not actively moving
                    //50% chance it goes up or down
                    bool rl = true;
                    if ((rand() % 2) + 1 ==  2){
                        rl = false;
                    }
                    if (rl == true){
                        ballspeedY = RandomFloat(-0.01,-bspeed);//send it down on y
                    }
                    else{
                        ballspeedY = RandomFloat(0.01,bspeed);//send it up on y
                    }
                }
            }
            if (player2hittingball() == true){
                ASND_SetVoice(ASND_GetFirstUnusedVoice(), VOICE_STEREO_16BIT, 22050, 0, (char *) bounce_pcm, bounce_pcm_size, 2000, 2000, NULL);
                // Voice, Format, Pitch, Delay, Sound, Sound Size, Left Audio, Right Audio, Callback
                ballspeedX = RandomFloat(-0.02,-bspeed);//Send it to the right on X
                if(players == 2){//If 2 Player Mode Is Enabled
                    if(WPAD_ButtonsHeld(1) & WPAD_BUTTON_DOWN || WPAD_ButtonsHeld(1) & WPAD_BUTTON_UP){//if actively moving
                        if(WPAD_ButtonsHeld(0) & WPAD_BUTTON_DOWN){//if going down
                            ballspeedY = RandomFloat(-0.01,-bspeed);//send it down on y
                        }
                        else if(WPAD_ButtonsHeld(0) & WPAD_BUTTON_UP){//if going up
                            ballspeedY = RandomFloat(0.01,bspeed);//send it up on y
                        }
                    }
                    else{//if not actively moving
                        //50% chance it goes up or down
                        bool rl = true;
                        if ((rand() % 2) + 1 ==  2){
                            rl = false;
                        }
                        if (rl == true){
                            ballspeedY = RandomFloat(-0.01,-bspeed);//send it down on y
                        }
                        else{
                            ballspeedY = RandomFloat(0.01,bspeed);//send it up on y
                        }
                    }
                }
                else{//If playing against CPU
                    //50% chance it goes up or down
                    bool rl = true;
                    if ((rand() % 2) + 1 ==  2){
                        rl = false;
                    }
                    if (rl == true){
                        ballspeedY = RandomFloat(-0.01,-bspeed);//send it down on y
                    }
                    else{
                        ballspeedY = RandomFloat(0.01,bspeed);//send it up on y
                    }
                }
            }

            //Init 3d camera and lighting
            GRRLIB_3dMode(0.1,1000,45,0,1);//Init 3d
            GRRLIB_Camera3dSettings(camrotx,camroty,camrotz, 0,1,0, 0,0,0);//Camera Settings
            GRRLIB_SetLightAmbient(0x808080FF);//Set ambient light level
            GRRLIB_SetLightDiff(0,(guVector){0.0f,3.0f,3.0f},20.0f,1.0f,0xFFFFFFFF);//Add a light or something


            //LEFT PADDLE
            GRRLIB_ObjectViewBegin();//start options
            GRRLIB_ObjectViewRotate(90,0,0);//rot
            GRRLIB_ObjectViewTrans(p1x,p1y,0);//pos
            GRRLIB_ObjectViewScale(p1w,p1h,p1z);//scale
            GRRLIB_ObjectViewEnd();//end options
            GRRLIB_DrawCube(1,1,0xFFFFFFFF);//size, filled, color

            //RIGHT PADDLE
            GRRLIB_ObjectViewBegin();//start
            GRRLIB_ObjectViewRotate(90,0,0);//rot
            GRRLIB_ObjectViewTrans(p2x,p2y,0);//pos
            GRRLIB_ObjectViewScale(p1w,p1h,p1z);//scale
            GRRLIB_ObjectViewEnd();//end
            GRRLIB_DrawCube(1,1,0xFFFFFFFF);//size, filled, color
            
            //BALL
            GRRLIB_ObjectViewBegin();//start
            GRRLIB_ObjectViewRotate(90,0,0);//rot
            GRRLIB_ObjectViewTrans(ballX,ballY,0);//pos
            GRRLIB_ObjectViewScale(1,1,1);//scale
            GRRLIB_ObjectViewEnd();//end
            GRRLIB_DrawSphere(ballW,10,10,true,0xFFFFFFFF);//radius, lats, longs, filled, color

            //2D Rendering
            GRRLIB_2dMode();

            //Score Display
            char scr1 [100];
            sprintf(scr1, "P1: %d", p1s);
            GRRLIB_PrintfTTF(50, 50, LCDSolidFont, scr1, 32, 0xC4C4C4FF);
            if(players == 2){
                char scr2 [100];
                sprintf(scr2, "P2: %d", p2s);
                if(p2s > 99){
                    GRRLIB_PrintfTTF(435, 50, LCDSolidFont, scr2, 32, 0xC4C4C4FF);
                }
                else if(p2s > 9){
                    GRRLIB_PrintfTTF(460, 50, LCDSolidFont, scr2, 32, 0xC4C4C4FF);
                }
                else{
                    GRRLIB_PrintfTTF(485, 50, LCDSolidFont, scr2, 32, 0xC4C4C4FF);
                }
            }
            else{
                char scr2 [100];
                sprintf(scr2, "CPU: %d", p2s);
                if(p2s > 99){
                    GRRLIB_PrintfTTF(435, 50, LCDSolidFont, scr2, 32, 0xC4C4C4FF);
                }
                else if(p2s > 9){
                    GRRLIB_PrintfTTF(460, 50, LCDSolidFont, scr2, 32, 0xC4C4C4FF);
                }
                else{
                    GRRLIB_PrintfTTF(485, 50, LCDSolidFont, scr2, 32, 0xC4C4C4FF);
                }
            }

            //Logic
            //Score Analyser
            if(gameplayselector == 2){//if game set to stop at 5 points
                if(p1s == 5){//if player 1 wins
                    gamemode = 5;
                    pausestatus = 6;
                    MP3Player_Stop();
                    MP3Player_PlayBuffer(win_mp3, win_mp3_size, NULL);
                }
                if(p2s == 5){//if player 2 wins
                    gamemode = 5;
                    pausestatus = 7;
                    MP3Player_Stop();
                    MP3Player_PlayBuffer(win_mp3, win_mp3_size, NULL);
                }
            }
            if(gameplayselector == 3){//if game set to stop at 10 points
                if(p1s == 10){//if player 1 wins
                    gamemode = 5;
                    pausestatus = 6;
                    MP3Player_Stop();
                    MP3Player_PlayBuffer(win_mp3, win_mp3_size, NULL);
                }
                if(p2s == 10){//if player 2 wins
                    gamemode = 5;
                    pausestatus = 7;
                    MP3Player_Stop();
                    MP3Player_PlayBuffer(win_mp3, win_mp3_size, NULL);
                }
            }
            if(gameplayselector == 4){//if game set to stop at 25 points
                if(p1s == 25){//if player 1 wins
                    gamemode = 5;
                    pausestatus = 8;//secret screen
                    MP3Player_Stop();
                }
                if(p2s == 25 && players == 2){//if player 2 wins while 2 players playing
                    gamemode = 5;
                    pausestatus = 8;//secret screen
                    MP3Player_Stop();
                }
                else if(p2s == 25){//if player 2 wins
                    gamemode = 5;
                    pausestatus = 7;//normal screen
                    MP3Player_Stop();
                }
            }
        }
        if(gamemode == 5){//pause menus
            if(pausestatus == 5){//Pause
                GRRLIB_PrintfTTF(200, 150, RubikFont, "Paused", 64, 0xFFFFFFFF); //Players Sign
                GRRLIB_PrintfTTF(15, 300, RubikFont, "Press + To Continue or Home To Quit", 32, 0xFFFFFFFF); //Instruction
                if (WPAD_ButtonsDown(0) & WPAD_BUTTON_PLUS && selectorcooldown == 0){
                    selectorcooldown = 10;
                    gamemode = 4;
                    MP3Player_Stop();
                }
                if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME && selectorcooldown == 0){
                    selectorcooldown = 10;
                    gamemode = 3;
                    MP3Player_Stop();
                }
            }
            if(pausestatus == 6){//Player One Wins
                GRRLIB_DrawImg(0, 0, GFX_titlebg, 0, 1, 1, 0xFFFFFFFF);
                GRRLIB_DrawImg(WinW / 2, WinH / 2, GFX_popup, 0, 1, 1, 0xFFFFFFFF);
                GRRLIB_PrintfTTF(200, 150, RubikFont, "P1 Wins", 64, 0x000000FF); //Players Sign
                GRRLIB_PrintfTTF(140, 300, RubikFont, "Press + or Home To Quit", 32, 0x000000FF); //Instruction
                if (WPAD_ButtonsDown(0) & WPAD_BUTTON_PLUS && selectorcooldown == 0){
                    selectorcooldown = 10;
                    gamemode = 3;
                    MP3Player_Stop();
                }
                if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME && selectorcooldown == 0){
                    selectorcooldown = 10;
                    gamemode = 3;
                    MP3Player_Stop();
                }
            }
            if(pausestatus == 7){//Player Two Wins
                GRRLIB_DrawImg(0, 0, GFX_titlebg, 0, 1, 1, 0xFFFFFFFF);
                GRRLIB_DrawImg(WinW / 2, WinH / 2, GFX_popup, 0, 1, 1, 0xFFFFFFFF);
                GRRLIB_PrintfTTF(200, 150, RubikFont, "P2 Wins", 64, 0x000000FF); //Players Sign
                GRRLIB_PrintfTTF(140, 300, RubikFont, "Press + or Home To Quit", 32, 0x000000FF); //Instruction
                if (WPAD_ButtonsDown(0) & WPAD_BUTTON_PLUS && selectorcooldown == 0){
                    selectorcooldown = 10;
                    gamemode = 3;
                    MP3Player_Stop();
                }
                if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME && selectorcooldown == 0){
                    selectorcooldown = 10;
                    gamemode = 3;
                    MP3Player_Stop();
                }
            }
            if(pausestatus == 8){//Credits
                GRRLIB_DrawImg(0, 0, GFX_credits, 0, 1, 1, 0xFFFFFFFF);
                if(MP3Player_IsPlaying() == false){
                    MP3Player_Stop();
                    MP3Player_PlayBuffer(titlemusic_mp3, titlemusic_mp3_size, NULL);
                }
                if (WPAD_ButtonsDown(0) & WPAD_BUTTON_PLUS && selectorcooldown == 0){
                    selectorcooldown = 10;
                    gamemode = 2;
                    MP3Player_Stop();
                    MP3Player_PlayBuffer(titlemusic_mp3, titlemusic_mp3_size, NULL);
                }
                if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME && selectorcooldown == 0){
                    selectorcooldown = 10;
                    gamemode = 2;
                    MP3Player_Stop();
                    MP3Player_PlayBuffer(titlemusic_mp3, titlemusic_mp3_size, NULL);
                }
            }
        }
        GRRLIB_Render();// Render the frame buffer to the TV
    }
    GRRLIB_Exit();
    if(HWButton != -1)
	{
		SYS_ResetSystem(HWButton, 0, 0);
	}
    return 0;
}