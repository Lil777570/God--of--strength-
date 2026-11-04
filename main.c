#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspctrl.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

PSP_MODULE_INFO("GOD_OF_STRENGTH", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);

#define W 480
#define H 272
#define BW 512
#define LIST_SIZE (256 * 1024)

static unsigned int __attribute__((aligned(16)))
displayList[LIST_SIZE / 4];

typedef struct {
    unsigned int color;
    float x,y,z;
} Vertex;

static Vertex cube[36];

static float px=0, pz=0, py=0;
static float enemyX=3, enemyZ=-3;
static float enemyHP=100;
static int playerHP=100;
static int level=0;
static int alive=1;
static int jump=0;
static int attack=0;
static int gameState=0;
static int chest=0;
static int frame=0;

static char *temples[]={
    "TEMPLE OF SHARADA",
    "TEMPLE OF GIDAN MARIYA",
    "TEMPLE OF GIDAN SHANU",
    "TEMPLE OF DORAYI",
    "TEMPLE OF RAMIN JIGAWA"
};

static char message[100]="THE JOURNEY BEGINS";

static void makeCube(void)
{
    int i;
    float v[36][3]={
    {-1,-1,1},{1,-1,1},{1,1,1},
    {-1,-1,1},{1,1,1},{-1,1,1},

    {1,-1,-1},{-1,-1,-1},{-1,1,-1},
    {1,-1,-1},{-1,1,-1},{1,1,-1},

    {-1,-1,-1},{-1,-1,1},{-1,1,1},
    {-1,-1,-1},{-1,1,1},{-1,1,-1},

    {1,-1,1},{1,-1,-1},{1,1,-1},
    {1,-1,1},{1,1,-1},{1,1,1},

    {-1,1,1},{1,1,1},{1,1,-1},
    {-1,1,1},{1,1,-1},{-1,1,-1},

    {-1,-1,-1},{1,-1,-1},{1,-1,1},
    {-1,-1,-1},{1,-1,1},{-1,-1,1}
    };

    for(i=0;i<36;i++){
        cube[i].color=0xffffffff;
        cube[i].x=v[i][0];
        cube[i].y=v[i][1];
        cube[i].z=v[i][2];
    }
}

static void box(float x,float y,float z,
                float sx,float sy,float sz,
                unsigned int color)
{
    int i;
    for(i=0;i<36;i++) cube[i].color=color;

    sceGumPushMatrix();

    ScePspFVector3 p={x,y,z};
    ScePspFVector3 s={sx,sy,sz};

    sceGumTranslate(&p);
    sceGumScale(&s);

    sceGumDrawArray(
        GU_TRIANGLES,
        GU_COLOR_8888 |
        GU_VERTEX_32BITF |
        GU_TRANSFORM_3D,
        36,0,cube
    );

    sceGumPopMatrix();
}

static void drawTemple(void)
{
    int i;

    /* Stone floor */
    box(0,-1,0,14,0.5f,14,0xff37343a);

    /* Temple walls */
    box(0,3,-12,14,4,0.5f,0xff514638);
    box(-13,3,0,0.5f,4,13,0xff62513f);
    box(13,3,0,0.5f,4,13,0xff62513f);

    /* Ancient pillars */
    for(i=-2;i<=2;i+=2){
        box(i*4.5f,2,-8,0.65f,3,0.65f,0xffb08b55);
        box(i*4.5f,2,8,0.65f,3,0.65f,0xffb08b55);
    }

    /* Entrance */
    box(0,2,-11.3f,2,3,0.2f,0xff11111c);

    /* Ancient treasure */
    if(!chest){
        box(-7,0,-4,0.9f,0.5f,0.7f,0xff263e9b);
        box(-7,0.55f,-4,1,0.15f,0.75f,0xffd5a52c);
    }

    /* Fire altar */
    box(0,0.1f,-7,1,0.2f,1,0xffa52b18);
    box(0,0.8f,-7,0.2f,0.8f,0.2f,0xff22aaff);
}

static void drawHero(void)
{
    /* Ismail warrior */

    /* Legs */
    box(px-0.25f,py-0.65f,pz,0.19f,0.55f,0.23f,0xff22252d);
    box(px+0.25f,py-0.65f,pz,0.19f,0.55f,0.23f,0xff22252d);

    /* Boots */
    box(px-0.25f,py-1.05f,pz+0.08f,0.25f,0.13f,0.35f,0xff171717);
    box(px+0.25f,py-1.05f,pz+0.08f,0.25f,0.13f,0.35f,0xff171717);

    /* Armour */
    box(px,py+0.05f,pz,0.48f,0.68f,0.30f,0xff353b48);

    /* Chest plate */
    box(px,py+0.20f,pz+0.31f,0.30f,0.36f,0.06f,0xffa17a36);

    /* Shoulders */
    box(px-0.60f,py+0.35f,pz,0.25f,0.28f,0.30f,0xff9c773e);
    box(px+0.60f,py+0.35f,pz,0.25f,0.28f,0.30f,0xff9c773e);

    /* Arms */
    box(px-0.65f,py-0.15f,pz,0.15f,0.48f,0.18f,0xff9d674a);
    box(px+0.65f,py-0.15f,pz,0.15f,0.48f,0.18f,0xff9d674a);

    /* Head and neck */
    box(px,py+0.90f,pz,0.15f,0.15f,0.16f,0xffa16e50);
    box(px,py+1.28f,pz,0.31f,0.34f,0.29f,0xffa96f4e);

    /* Hair */
    box(px,py+1.56f,pz,0.32f,0.10f,0.30f,0xff17120f);

    /* Sword */
    box(px+0.82f,py+0.30f,pz,0.07f,0.80f,0.07f,0xffd7e0ed);
    box(px+0.82f,py-0.50f,pz,0.20f,0.08f,0.16f,0xff9a5d25);
}

static void drawEnemy(void)
{
    if(!alive) return;

    /* Giant temple demon */

    box(enemyX,0,enemyZ,0.75f,0.95f,0.55f,0xff64221c);

    /* Chest */
    box(enemyX,0.35f,enemyZ+0.48f,
        0.43f,0.48f,0.10f,0xffa12b20);

    /* Head */
    box(enemyX,1.30f,enemyZ,
        0.48f,0.48f,0.45f,0xff501b17);

    /* Horns */
    box(enemyX-0.38f,1.78f,enemyZ,
        0.12f,0.35f,0.12f,0xff171111);

    box(enemyX+0.38f,1.78f,enemyZ,
        0.12f,0.35f,0.12f,0xff171111);

    /* Arms */
    box(enemyX-0.90f,0.1f,enemyZ,
        0.25f,0.75f,0.30f,0xff76251d);

    box(enemyX+0.90f,0.1f,enemyZ,
        0.25f,0.75f,0.30f,0xff76251d);

    /* Glowing eyes */
    box(enemyX-0.17f,1.35f,enemyZ+0.42f,
        0.07f,0.06f,0.04f,0xff22ccff);

    box(enemyX+0.17f,1.35f,enemyZ+0.42f,
        0.07f,0.06f,0.04f,0xff22ccff);
}

static void drawFinalBoss(void)
{
    box(enemyX,0,enemyZ,1.5f,1.8f,1.0f,0xff36121b);
    box(enemyX,2.3f,enemyZ,0.8f,0.8f,0.7f,0xff6b1717);

    box(enemyX-0.8f,2.9f,enemyZ,
        0.2f,0.7f,0.2f,0xff151515);

    box(enemyX+0.8f,2.9f,enemyZ,
        0.2f,0.7f,0.2f,0xff151515);

    box(enemyX,2.35f,enemyZ+0.65f,
        0.12f,0.1f,0.08f,0xffffff00);
}

static void setup3D(void)
{
    sceGuInit();

    sceGuStart(GU_DIRECT,displayList);

    sceGuDrawBuffer(GU_PSM_8888,(void*)0,BW);
    sceGuDispBuffer(W,H,(void*)0x88000,BW);
    sceGuDepthBuffer((void*)0x110000,BW);

    sceGuOffset(2048-W/2,2048-H/2);
    sceGuViewport(2048,2048,W,H);

    sceGuDepthRange(65535,0);
    sceGuScissor(0,0,W,H);

    sceGuEnable(GU_SCISSOR_TEST);
    sceGuEnable(GU_DEPTH_TEST);
    sceGuDepthFunc(GU_GEQUAL);

    sceGuFrontFace(GU_CW);
    sceGuShadeModel(GU_SMOOTH);

    sceGuClearColor(0xff100c16);
    sceGuClearDepth(0);

    sceGuFinish();
    sceGuSync(0,0);

    sceDisplayWaitVblankStart();
    sceGuDisplay(GU_TRUE);
}

static void render(void)
{
    sceGuStart(GU_DIRECT,displayList);

    sceGuClearColor(0xff100c16);
    sceGuClearDepth(0);
    sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT);

    sceGumMatrixMode(GU_PROJECTION);
    sceGumLoadIdentity();
    sceGumPerspective(55,16.0f/9.0f,0.5f,100);

    sceGumMatrixMode(GU_VIEW);
    sceGumLoadIdentity();

    ScePspFVector3 eye={px,7.0f,pz+12};
    ScePspFVector3 target={px,0.5f,pz-2};
    ScePspFVector3 up={0,1,0};

    sceGumLookAt(&eye,&target,&up);

    sceGumMatrixMode(GU_MODEL);
    sceGumLoadIdentity();

    sceGuEnable(GU_CULL_FACE);

    drawTemple();
    drawHero();

    if(level==4)
        drawFinalBoss();
    else
        drawEnemy();

    sceGuDisable(GU_CULL_FACE);

    sceGuFinish();
    sceGuSync(0,0);
    sceGuSwapBuffers();
}

int main(void)
{
    SceCtrlData pad,old;
    memset(&old,0,sizeof(old));

    makeCube();

    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

    setup3D();

    while(1){

        sceCtrlReadBufferPositive(&pad,1);

        unsigned int pressed=pad.Buttons&~old.Buttons;
        old=pad;

        frame++;

        if(pressed&PSP_CTRL_START)
            break;

        if(gameState==0){

            if(pressed&PSP_CTRL_CROSS){
                gameState=1;
                strcpy(message,"THE ADVENTURE BEGINS");
            }

        }else{

            float speed=0.09f;

            if(pad.Buttons&PSP_CTRL_LEFT) px-=speed;
            if(pad.Buttons&PSP_CTRL_RIGHT) px+=speed;
            if(pad.Buttons&PSP_CTRL_UP) pz-=speed;
            if(pad.Buttons&PSP_CTRL_DOWN) pz+=speed;

            if(pad.Lx<100) px-=speed;
            if(pad.Lx>155) px+=speed;
            if(pad.Ly<100) pz-=speed;
            if(pad.Ly>155) pz+=speed;

            if(px<-10) px=-10;
            if(px>10) px=10;
            if(pz<-10) pz=-10;
            if(pz>10) pz=10;

            if(pressed&PSP_CTRL_CROSS)
                jump=25;

            if(jump>0){
                py=(float)jump*0.045f;
                jump--;
            }else py=0;

            float dx=px-enemyX;
            float dz=pz-enemyZ;
            float distance=dx*dx+dz*dz;

            if(pressed&PSP_CTRL_SQUARE){
                if(distance<20){
                    enemyHP-=20;
                    strcpy(message,"POWER ATTACK!");
                }else{
                    strcpy(message,"MOVE CLOSER!");
                }
            }

            if(pressed&PSP_CTRL_TRIANGLE){
                if(distance<16){
                    enemyHP-=12;
                    strcpy(message,"SWORD COMBO!");
                }
            }

            if(pressed&PSP_CTRL_RTRIGGER){
                if(distance<25){
                    enemyHP-=30;
                    strcpy(message,"SPECIAL ATTACK!");
                }
            }

            if(pressed&PSP_CTRL_CIRCLE){
                if(!chest &&
                   (px+7)*(px+7)+(pz+4)*(pz+4)<5){
                    chest=1;
                    playerHP+=30;
                    if(playerHP>100) playerHP=100;
                    strcpy(message,"TREASURE FOUND!");
                }
            }

            if(enemyHP<=0 && alive){
                alive=0;
                strcpy(message,"TEMPLE GUARDIAN DEFEATED!");
            }

            if(!alive && (pressed&PSP_CTRL_CIRCLE)){
                if(level<4){
                    level++;
                    enemyHP=100+(level*30);
                    alive=1;
                    enemyX=3;
                    enemyZ=-3;
                    px=0;
                    pz=0;
                    chest=0;
                    strcpy(message,"NEW TEMPLE UNLOCKED!");
                }
            }

            if(alive && distance<2.5f && frame%60==0){
                playerHP-=5;
                if(playerHP<=0){
                    playerHP=0;
                    gameState=2;
                    strcpy(message,"ISMAIL HAS FALLEN!");
                }
            }

            if(gameState==2 && (pressed&PSP_CTRL_CROSS)){
                playerHP=100;
                enemyHP=100+(level*30);
                alive=1;
                px=0;
                pz=0;
                gameState=1;
            }
        }

        render();

        sceDisplayWaitVblankStart();
    }

    sceGuTerm();
    sceKernelExitGame();
    return 0;
}
