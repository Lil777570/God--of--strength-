#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspctrl.h>
#include <psputility.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

PSP_MODULE_INFO("ISMAIL_FIVE_TEMPLES_3D", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);

#define BUF_WIDTH 512
#define SCR_WIDTH 480
#define SCR_HEIGHT 272
#define LIST_SIZE (256 * 1024)

static unsigned int __attribute__((aligned(16))) list[LIST_SIZE / 4];
static unsigned short __attribute__((aligned(16))) depth[BUF_WIDTH * SCR_HEIGHT];

typedef struct {
    unsigned int color;
    float x, y, z;
} Vertex;

static float px = 0.0f, pz = 0.0f, py = 0.0f;
static float enemyX = 3.0f, enemyZ = -2.0f;
static int hp = 100, enemyHP = 100, chestOpened = 0, enemyAlive = 1;
static int jumpTicks = 0, blockOn = 0, gameOver = 0;
static char statusText[80] = "Find the guardian in the Temple of Sharada.";

static Vertex cubeVerts[] = {
    /* front */
    {0xffffffff, -1,-1, 1}, {0xffffffff, 1,-1, 1}, {0xffffffff, 1, 1, 1},
    {0xffffffff, -1,-1, 1}, {0xffffffff, 1, 1, 1}, {0xffffffff, -1, 1, 1},
    /* back */
    {0xffffffff, 1,-1,-1}, {0xffffffff, -1,-1,-1}, {0xffffffff, -1, 1,-1},
    {0xffffffff, 1,-1,-1}, {0xffffffff, -1, 1,-1}, {0xffffffff, 1, 1,-1},
    /* left */
    {0xffffffff, -1,-1,-1}, {0xffffffff, -1,-1,1}, {0xffffffff, -1,1,1},
    {0xffffffff, -1,-1,-1}, {0xffffffff, -1,1,1}, {0xffffffff, -1,1,-1},
    /* right */
    {0xffffffff, 1,-1,1}, {0xffffffff, 1,-1,-1}, {0xffffffff, 1,1,-1},
    {0xffffffff, 1,-1,1}, {0xffffffff, 1,1,-1}, {0xffffffff, 1,1,1},
    /* top */
    {0xffffffff, -1,1,1}, {0xffffffff, 1,1,1}, {0xffffffff, 1,1,-1},
    {0xffffffff, -1,1,1}, {0xffffffff, 1,1,-1}, {0xffffffff, -1,1,-1},
    /* bottom */
    {0xffffffff, -1,-1,-1}, {0xffffffff, 1,-1,-1}, {0xffffffff, 1,-1,1},
    {0xffffffff, -1,-1,-1}, {0xffffffff, 1,-1,1}, {0xffffffff, -1,-1,1}
};

static void drawCube(float x, float y, float z, float sx, float sy, float sz, unsigned int color) {
    int i;
    for (i = 0; i < 36; i++) cubeVerts[i].color = color;
    sceGumPushMatrix();
    ScePspFVector3 pos = {x, y, z};
    ScePspFVector3 scale = {sx, sy, sz};
    sceGumTranslate(&pos);
    sceGumScale(&scale);
    sceGumDrawArray(GU_TRIANGLES, GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_3D,
                    36, 0, cubeVerts);
    sceGumPopMatrix();
}

static void drawTemple(void) {
    int i;
    drawCube(0, -1.0f, 0, 13, 0.5f, 13, 0xff383b42);       /* stone floor */
    drawCube(0, 4.2f, -12, 13, 0.5f, 1, 0xff56504a);      /* rear wall */
    drawCube(-12.5f, 2.0f, 0, 0.5f, 3.0f, 13, 0xff62594d);
    drawCube(12.5f, 2.0f, 0, 0.5f, 3.0f, 13, 0xff62594d);
    for (i = -2; i <= 2; i += 2) {
        drawCube((float)i * 4.5f, 2.0f, -7.0f, 0.65f, 3.0f, 0.65f, 0xff8d8068);
        drawCube((float)i * 4.5f, 2.0f, 7.0f, 0.65f, 3.0f, 0.65f, 0xff8d8068);
    }
    drawCube(0, 5.2f, -7.0f, 11, 0.45f, 0.7f, 0xff766b5a);
    /* distant doorway */
    drawCube(0, 1.7f, -11.35f, 2.1f, 2.7f, 0.15f, 0xff171b26);
    drawCube(-2.5f, 2.0f, -11.0f, 0.3f, 3.0f, 0.4f, 0xffb88b42);
    drawCube(2.5f, 2.0f, -11.0f, 0.3f, 3.0f, 0.4f, 0xffb88b42);
    /* chest */
    if (!chestOpened) {
        drawCube(-7.0f, -0.1f, -4.0f, 0.8f, 0.55f, 0.65f, 0xff245eaa);
        drawCube(-7.0f, 0.45f, -4.0f, 0.85f, 0.16f, 0.7f, 0xffd9ad36);
    }
}

static void drawHero(void) {
    /* Ismail: stylized low-poly warrior placeholder */
    drawCube(px, py + 0.15f, pz, 0.48f, 0.75f, 0.34f, 0xff252c39); /* armor */
    drawCube(px, py + 1.15f, pz, 0.34f, 0.34f, 0.34f, 0xffb77b55); /* head */
    drawCube(px - 0.62f, py + 0.1f, pz, 0.18f, 0.62f, 0.2f, 0xffb77b55);
    drawCube(px + 0.62f, py + 0.1f, pz, 0.18f, 0.62f, 0.2f, 0xffb77b55);
    drawCube(px - 0.23f, py - 0.85f, pz, 0.2f, 0.55f, 0.23f, 0xff20242c);
    drawCube(px + 0.23f, py - 0.85f, pz, 0.2f, 0.55f, 0.23f, 0xff20242c);
    /* weapon */
    drawCube(px + 0.82f, py + 0.05f, pz - 0.05f, 0.09f, 0.85f, 0.09f, 0xffd7dce5);
    drawCube(px + 0.82f, py - 0.78f, pz - 0.05f, 0.22f, 0.10f, 0.22f, 0xff9d672f);
}

static void drawEnemy(void) {
    if (!enemyAlive) return;
    /* Temple guardian */
    drawCube(enemyX, 0.0f, enemyZ, 0.72f, 0.95f, 0.55f, 0xff4c2628);
    drawCube(enemyX, 1.25f, enemyZ, 0.55f, 0.48f, 0.5f, 0xff7a3028);
    drawCube(enemyX - 0.75f, 0.0f, enemyZ, 0.28f, 0.75f, 0.3f, 0xff5b2525);
    drawCube(enemyX + 0.75f, 0.0f, enemyZ, 0.28f, 0.75f, 0.3f, 0xff5b2525);
    drawCube(enemyX, 1.62f, enemyZ - 0.1f, 0.12f, 0.10f, 0.08f, 0xff22ddff);
}

static void setup3D(void) {
    sceGuStart(GU_DIRECT, list);
    sceGuDrawBuffer(GU_PSM_8888, (void*)0, BUF_WIDTH);
    sceGuDispBuffer(SCR_WIDTH, SCR_HEIGHT, (void*)0x88000, BUF_WIDTH);
    sceGuDepthBuffer((void*)0x110000, BUF_WIDTH);
    sceGuOffset(2048 - (SCR_WIDTH/2), 2048 - (SCR_HEIGHT/2));
    sceGuViewport(2048, 2048, SCR_WIDTH, SCR_HEIGHT);
    sceGuDepthRange(65535, 0);
    sceGuScissor(0, 0, SCR_WIDTH, SCR_HEIGHT);
    sceGuEnable(GU_SCISSOR_TEST);
    sceGuEnable(GU_DEPTH_TEST);
    sceGuDepthFunc(GU_GEQUAL);
    sceGuFrontFace(GU_CW);
    sceGuShadeModel(GU_SMOOTH);
    sceGuClearColor(0xff171d2b);
    sceGuClearDepth(0);
    sceGuClear(GU_COLOR_BUFFER_BIT | GU_DEPTH_BUFFER_BIT);
    sceGuFinish();
    sceGuSync(0, 0);
    sceDisplayWaitVblankStart();
    sceGuDisplay(GU_TRUE);
}

static void render(void) {
    sceGuStart(GU_DIRECT, list);
    sceGuClearColor(0xff171d2b);
    sceGuClearDepth(0);
    sceGuClear(GU_COLOR_BUFFER_BIT | GU_DEPTH_BUFFER_BIT);
    sceGuMatrixMode(GU_PROJECTION);
    sceGuLoadIdentity();
    sceGumPerspective(55.0f, 16.0f/9.0f, 0.5f, 100.0f);
    sceGuMatrixMode(GU_VIEW);
    sceGuLoadIdentity();
    ScePspFVector3 eye = {px + 0.0f, 7.5f, pz + 12.0f};
    ScePspFVector3 target = {px, 0.3f, pz - 2.0f};
    ScePspFVector3 up = {0.0f, 1.0f, 0.0f};
    sceGumLookAt(&eye, &target, &up);
    sceGuMatrixMode(GU_MODEL);
    sceGuEnable(GU_CULL_FACE);
    sceGuEnable(GU_CLIP_PLANES);
    drawTemple();
    drawHero();
    drawEnemy();
    sceGuDisable(GU_CULL_FACE);
    sceGuFinish();
    sceGuSync(0, 0);
    sceGuSwapBuffers();
}

static void setStatus(const char *s) {
    strncpy(statusText, s, sizeof(statusText)-1);
    statusText[sizeof(statusText)-1] = 0;
}

int main(void) {
    SceCtrlData pad, old;
    memset(&old, 0, sizeof(old));
    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
    sceGuInit();
    setup3D();

    while (1) {
        sceCtrlReadBufferPositive(&pad, 1);
        unsigned int pressed = pad.Buttons & ~old.Buttons;
        old = pad;

        if (pressed & PSP_CTRL_START) break;

        blockOn = (pad.Buttons & PSP_CTRL_LTRIGGER) ? 1 : 0;

        if (!gameOver) {
            float speed = blockOn ? 0.055f : 0.10f;
            if (pad.Buttons & PSP_CTRL_LEFT)  px -= speed;
            if (pad.Buttons & PSP_CTRL_RIGHT) px += speed;
            if (pad.Buttons & PSP_CTRL_UP)    pz -= speed;
            if (pad.Buttons & PSP_CTRL_DOWN)  pz += speed;
            if (pad.Lx < 100) px -= speed;
            if (pad.Lx > 155) px += speed;
            if (pad.Ly < 100) pz -= speed;
            if (pad.Ly > 155) pz += speed;
            if (px < -10.5f) px = -10.5f;
            if (px > 10.5f) px = 10.5f;
            if (pz < -9.5f) pz = -9.5f;
            if (pz > 9.5f) pz = 9.5f;

            if ((pressed & PSP_CTRL_CROSS) && jumpTicks == 0) {
                jumpTicks = 24;
                setStatus("Ismail jumps!");
            }
            if (jumpTicks > 0) {
                py = (float)jumpTicks * 0.055f;
                jumpTicks--;
            } else py = 0.0f;

            float dx = px - enemyX, dz = pz - enemyZ;
            float dist2 = dx*dx + dz*dz;
            if ((pressed & PSP_CTRL_TRIANGLE) && enemyAlive) {
                if (dist2 < 16.0f) { enemyHP -= 12; setStatus("Quick blade combo!"); }
                else setStatus("Move closer for a quick strike.");
            }
            if ((pressed & PSP_CTRL_SQUARE) && enemyAlive) {
                if (dist2 < 20.0f) { enemyHP -= 24; setStatus("POWER ATTACK! Heavy hit!"); }
                else setStatus("Heavy attack missed. Get closer.");
            }
            if ((pressed & PSP_CTRL_RTRIGGER) && enemyAlive) {
                if (dist2 < 25.0f) { enemyHP -= 35; setStatus("SPECIAL COMBO! Devastating strike!"); }
                else setStatus("Special attack: enemy out of range.");
            }
            if ((pressed & PSP_CTRL_CIRCLE)) {
                if (!chestOpened && (px + 7.0f)*(px + 7.0f) + (pz + 4.0f)*(pz + 4.0f) < 5.0f) {
                    chestOpened = 1; hp += 35; if (hp > 100) hp = 100;
                    setStatus("Health chest opened! Ismail recovered health.");
                } else if (enemyAlive && dist2 < 9.0f) {
                    enemyHP = 0; setStatus("GRAB FINISHER! Guardian defeated!");
                } else setStatus("Circle: approach the chest or grab a nearby enemy.");
            }

            if (enemyHP <= 0 && enemyAlive) {
                enemyAlive = 0;
                setStatus("Guardian defeated! The Sharada temple is cleared.");
            }

            if (enemyAlive && dist2 < 2.4f && !blockOn) {
                static int hitDelay = 0;
                if (++hitDelay > 50) {
                    hp -= 8; hitDelay = 0;
                    setStatus("Guardian hit you! Hold L to block.");
                    if (hp <= 0) { hp = 0; gameOver = 1; setStatus("ISMAIL HAS FALLEN - restart the game."); }
                }
            }
        }
        render();
        sceDisplayWaitVblankStart();
    }

    sceGuTerm();
    sceKernelExitGame();
    return 0;
}
