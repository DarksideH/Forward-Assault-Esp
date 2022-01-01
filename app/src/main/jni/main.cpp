#include <list>
#include <vector>
#include <string>
#include <string.h>
#include <pthread.h>
#include <cstring>
#include <jni.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include "Unity/Vector2.h"
#include "Unity/Vector3.h"
#include "Unity/Rect.h"
#include "Unity/Color.h"
#include "Unity/Quaternion.h"
#include "Includes/Logger.h"
#include "Includes/obfuscate.h"
#include "Includes/Utils.h"
#include "Includes/Chams.h"
#include "Includes/MonoString.h"
#include "Includes/Strings.h"
#include "KittyMemory/MemoryPatch.h"
#include "menu.h"

#include <Substrate/SubstrateHook.h>
#include <Substrate/CydiaSubstrate.h>

#define libName OBFUSCATE("libil2cpp.so")

struct My_Patches {
    MemoryPatch GodMode, GodMode2;
} hexPatches;

bool ESP, ESPTeamminate, ESPLine, ESPBox, ESPHealth, ESPCrosshair, ESPCircle, underground, above, wireframe, glow, outline, chams, wchams, rainbow, hudtext, kills, telekill, masskill, fireshoot, spamchat, enemyscale, Fly, speedHack, noAntiHack;

Color color; //Color esp
int ESPBoxStyle = 1; //Settings Box
int ESPLineLocation = 2.4; //Location line
int Speed;
int scaleX, scaleY, scaleZ;
//private extern void get_position_Injected(out Vector3 ret);
//Transform.get_position_Injected();
Vector3 get_position(void *transform) {
    if (!transform)
    return Vector3();
    Vector3 position;
    static const auto get_position_injected = reinterpret_cast<uint64_t(__fastcall *)(void *,
    Vector3 &)>(getAbsoluteAddress(libName, 0x108D1C8));
    get_position_injected(transform, position);
    return position;
}

//Component.get_transform();
void *get_transform(void *player) {
    if (!player)
    return NULL;
    static const auto get_transform_injected = reinterpret_cast<uint64_t(__fastcall *)(
    void *)>(getAbsoluteAddress(libName, 0x136B7F0));
    return (void *) get_transform_injected(player);
}

//CameraManager.get_MainCamera();
void *get_camera() {
    static const auto get_camera_injected = reinterpret_cast<uint64_t(__fastcall *)()>(getAbsoluteAddress(
    libName, 0xC0667C));
    return (void *) get_camera_injected();
}

//Screen.get_height();
int get_screen_height() {
    static const auto get_height_injected = reinterpret_cast<uint64_t(__fastcall *)()>(getAbsoluteAddress(
    libName, 0x108208C));
    return (int) get_height_injected();
}

//Screen.get_width();
int get_screen_width() {
    static const auto get_width_injected = reinterpret_cast<uint64_t(__fastcall *)()>(getAbsoluteAddress(
    libName, 0x1082048));
    return (int) get_width_injected();
}

//Camera.WorldToScreenPoint_Injected
//public Vector3 WorldToScreenPoint(Vector3 position, MonoorSetreo..., out Vector3)
Vector3 WorldToScreenPoint(void *transform, Vector3 test) {
    if (!transform)
        return Vector3();
    Vector3 position;
    static const auto WorldToScreenPoint_Injected = reinterpret_cast<uint64_t(__fastcall *)(void *,
     Vector3, int, Vector3 &)>(getAbsoluteAddress(
      libName, 0x1368E2C));
      WorldToScreenPoint_Injected(transform, test, 2, position);
      return position;
}

//FEHAJLBCGIN
int GetPlayerHealth(void *player) {
    return *(float *) ((uint64_t) player + 0x16C);
}

bool PlayerAlive(void *player) {
    return player != NULL && GetPlayerHealth(player) > 0;
}

//BMFGOOEECIC
int GetPlayerTeam(void *player) {
    return *(int *) ((uint64_t) player + 0x184);
}

bool IsPlayerDead(void *player) {
    return player == NULL && GetPlayerHealth(player) < 1;
}

Vector3 GetPlayerLocation(void *player) {
    Vector3 location;
    location = get_position(get_transform(player));
    return location;
}

void *myPlayer = NULL;
struct enemy_t {
    void *object;
    Vector3 location;
    int health;
    
};

/**Entity_Manager**/
class ESPManager {
public:
    std::vector<enemy_t *> *enemies;

    ESPManager() {
        enemies = new std::vector<enemy_t *>();
    }

    bool isEnemyPresent(void *enemyObject) {
        for (std::vector<enemy_t *>::iterator it = enemies->begin(); it != enemies->end(); it++) {
            if ((*it)->object == enemyObject) {
                return true;
            }
        }
        return false;
    }

    void removeEnemy(enemy_t *enemy) {
        for (int i = 0; i < enemies->size(); i++) {
            if ((*enemies)[i] == enemy) {
                enemies->erase(enemies->begin() + i);
                return;
            }
        }
    }

    void tryAddEnemy(void *enemyObject) {
        if (isEnemyPresent(enemyObject)) {
        return;
        }

        if (IsPlayerDead(enemyObject)) {
        return;
        }

        enemy_t *newEnemy = new enemy_t();

        newEnemy->object = enemyObject;

        enemies->push_back(newEnemy);
    }

    void updateEnemies(void *enemyObject) {
        for (int i = 0; i < enemies->size(); i++) {
            enemy_t *current = (*enemies)[i];

            if (IsPlayerDead(current->object)) {
                enemies->erase(enemies->begin() + i);
            }
            if ((GetPlayerTeam(myPlayer) == GetPlayerTeam(current->object))) {
                enemies->erase(enemies->begin() + i);
            }
        }
    }

    void removeEnemyGivenObject(void *enemyObject) {
        for (int i = 0; i < enemies->size(); i++) {
            if ((*enemies)[i]->object == enemyObject) {
                enemies->erase(enemies->begin() + i);
                return;
            }
        }
    }

};

ESPManager *espManager;

Color GetPlayerColor(void *Player){
int MyTeam = GetPlayerTeam(myPlayer);
int TeamNum = GetPlayerTeam(Player);
Color color;
}
float upval;
float upval2;
float upval3;

extern "C"
JNIEXPORT void JNICALL
Java_uk_lgl_modmenu_FloatingModMenuService_DrawOn(JNIEnv *env, jclass clazz, jobject esp, jobject canvas) {
    jclass Esp = env->GetObjectClass(esp);

    jmethodID DrawFilledRects = env->GetMethodID(Esp, "DrawFilledRect",
                                                 "(Landroid/graphics/Canvas;IIIIFFFF)V");
                                                 
    jmethodID DrawRect = env->GetMethodID(Esp, "DrawRect",
                                          "(Landroid/graphics/Canvas;IIIIIFFFF)V");
                                          
    jmethodID DrawLines = env->GetMethodID(Esp, "DrawLine",
                                           "(Landroid/graphics/Canvas;IIIIFFFFF)V");
                                           
    //jmethodID DrawFilledCircle = env->GetMethodID(Esp, "DrawFilledCircle",
                                           //"(Landroid/graphics/Canvas;IIIIFFF)V");                                       
                                                                                                                                                       
    jmethodID DrawText = env->GetMethodID(Esp, "DrawText",
                                          "(Landroid/graphics/Canvas;IIIILjava/lang/String;FFF)V");

    if (ESP) {
        if (espManager->enemies->empty()) {
            return;
        }
        for (int i = 0; i < espManager->enemies->size(); i++) {

            void *Player = (*espManager->enemies)[i]->object;

            if (PlayerAlive(Player) &&
                (GetPlayerTeam(myPlayer) != GetPlayerTeam(Player) || ESPTeamminate)) {
                Color clr;
                clr.a = 255;
                Rect rect;
                Vector3 PlayerPos = GetPlayerLocation(Player);
                void *Cam = get_camera();
                Vector3 PosNew = {0.f, 0.f, 0.f};
                PosNew = WorldToScreenPoint(Cam, PlayerPos);
                if (PosNew.z < 1.f) continue;
                Vector3 Origin;
                Origin = PlayerPos;
                Origin.y += 0.7f;
                float posnum = upval; /******* Hight slider * 0.1 ********/
                float posnum2 = upval2;/******* Size slider * 0.1 ********/
                float posnum3 = upval3;
                Origin.y += posnum;
                Vector3 BoxPosNew = {0.f, 0.f, 0.f};
                BoxPosNew = WorldToScreenPoint(Cam, Origin);
                float Hight =
                abs(BoxPosNew.y - PosNew.y) * (posnum2 / posnum), Width = Hight * 0.6f;

                rect = Rect(BoxPosNew.x - Width / 2.f,
                            get_screen_height() - BoxPosNew.y,
                            Width, Hight
                );
                if (ESPBox) {            
                    if (ESPBoxStyle == 0) {
                        env->CallVoidMethod(esp, DrawFilledRects, canvas, (jint) color.a,
                                            (jint) color.r, (jint) color.g, (jint) color.b,
                                            (jfloat) rect.x, (jfloat) rect.y, (jfloat) rect.w,
                                            (jfloat) rect.h);
                    } else if (ESPBoxStyle == 1) {
                        env->CallVoidMethod(esp, DrawRect, canvas, (jint) color.a, (jint) color.r,
                                            (jint) color.g, (jint) color.b, (jint) upval3,//1.5
                                            (jfloat) rect.x, (jfloat) rect.y, (jfloat) rect.w,
                                            (jfloat) rect.h);                   
                    } else if (ESPBoxStyle == 2) {
                        env->CallVoidMethod(esp, DrawFilledRects, canvas, (jint) clr.a,
                                            (jint) color.r, (jint) color.g, (jint) color.b,
                                            (jfloat) rect.x, (jfloat) rect.y, (jfloat) rect.w,
                                            (jfloat) rect.h);
                        env->CallVoidMethod(esp, DrawRect, canvas, (jint) color.a, (jint) color.r,
                                            (jint) color.g, (jint) color.b, (jint) upval3,//1.5
                                            (jfloat) rect.x, (jfloat) rect.y, (jfloat) rect.w,
                                            (jfloat) rect.h);
                    }
                }
               if (ESPLine) {              
                    env->CallVoidMethod(esp, DrawLines, canvas, (jint) color.a, (jint) color.r,
                                        (jint) color.g, (jint) color.b, (jfloat) upval3,//1.5
                                        (jfloat) get_screen_width() / ESPLineLocation,
                                        (jfloat) get_screen_height() / 1000,
                                        (jfloat) rect.x + rect.w / 2,
                                        (jfloat) rect.y + rect.h / 20);          
                }
                 if (ESPHealth) {                                                                                                                                                                                                              
                                                                                          
                }
            } else {
                espManager->removeEnemyGivenObject(Player);
            }
        }
    }
 }

bool (*isShooting)(void* instance);

void (*set_localScale)(void *transform, Vector3 scale);

void* (*Component_GetTransform)(void* component);

void (*Transform_INTERNAL_set_position)(void* transform, Vector3 newPosition);

void (*Transform_INTERNAL_get_position)(void* transform, Vector3* out);

void *enemyPlayer = NULL;
void *updatePlayer = NULL;
void (*Player_update)(void *player);
void _Player_update(void *player) {
    if (player != NULL) {
        Player_update(player);

        bool isMine = *(bool *) ((uint64_t) player + 0x84);//Standoff 2 0.15.1 - 0x2C
        updatePlayer = player;

        if (isMine) {
            myPlayer = player;
        }
       
    if(myPlayer) {
        if(GetPlayerTeam(myPlayer) != GetPlayerTeam(player)) {
            enemyPlayer = player;
        }
    }
    // Checking if enemyPlayer object is not null
    if(enemyPlayer) {
        // checking if it's not dead, if it is enemyPlayer is NULL & it should look for a new one.
        if(!IsPlayerDead(enemyPlayer)) {
            if (kills) {
                *(int *)((uint64_t)player + 0x3C) = 228;
            }           
            if (telekill) {
                Vector3 enemyLocation = GetPlayerLocation(enemyPlayer);
                // You can mod these values to your liking
                Transform_INTERNAL_set_position(Component_GetTransform(myPlayer), Vector3(enemyLocation.x, enemyLocation.y, enemyLocation.z - 1));
            }
            if (enemyscale) {                   
                set_localScale(get_transform(enemyPlayer), Vector3(scaleX, scaleY, scaleZ));
                } else {
                set_localScale(get_transform(enemyPlayer), Vector3(1, 1, 1));
                }
            if (masskill) {
                Vector3 myPosition = GetPlayerLocation(myPlayer);
                Transform_INTERNAL_set_position(Component_GetTransform(enemyPlayer), Vector3(myPosition.x, myPosition.y, myPosition.z + 1));
            }
        } else {
            enemyPlayer = NULL;
            return;
        }
        }

        if (ESP) {
            if (!isMine) {
                if (myPlayer) {
                    if ((GetPlayerTeam(myPlayer) != GetPlayerTeam(player) || ESPTeamminate)) {
                        espManager->tryAddEnemy(player);             
                    }
                    espManager->updateEnemies(player);
                }
            }
        }
    }
}

void (*Player_ondestroy)(void *player);
void _Player_ondestroy(void *player) {
    if (player != NULL) {
        Player_ondestroy(player);
        espManager->removeEnemyGivenObject(player);
    }
}

void(*old_Update_Controller)(void *instance);
void Update_Controller(void *instance) {
    if (instance != NULL) {
        if (Fly) {
              *(bool *) ((uint64_t) instance + 0x1C) = true; 
        }
        if (speedHack) {
             *(float *) ((uint64_t) instance + 0x14) = Speed; 
        } 
        old_Update_Controller(instance);
    }
    old_Update_Controller(instance);
}

void *hack_thread(void *) {
    ProcMap il2cppMap;
    do {
        il2cppMap = KittyMemory::getLibraryMap("libil2cpp.so");
        sleep(1);
    } while (!isLibraryLoaded(libName) && mlovinit());
    setShader("_BumpMap");
    LogShaders();
    Wallhack();
    espManager = new ESPManager();   
    
    // Hex Code 6461726B73696465
    do {     
        sleep(1);
    } while (!isLibraryLoaded (OBFUSCATE("libdarkside.so")));
    
    Component_GetTransform = (void* (*)(void*))getAbsoluteAddress(libName, 0x136B7F0);
    Transform_INTERNAL_set_position = (void (*)(void*, Vector3))getAbsoluteAddress(libName, 0x108D27C);
    Transform_INTERNAL_get_position = (void (*)(void*, Vector3*))getAbsoluteAddress(libName, 0x108D1C8);
    set_localScale = (void (*)(void *, Vector3))getAbsoluteAddress(libName, 0x108E0C0);
    
    MSHookFunction((void *) getAbsoluteAddress(libName, 0x20C5E40), (void *) &Update_Controller,
                   (void **) &old_Update_Controller); //Update Speed
    MSHookFunction((void *) getAbsoluteAddress(libName, 0x771EB8), (void *) &_Player_update,
                   (void **) &Player_update); //Player.Update(); Standoff 2 0.15.1 - 0x10B5694
    MSHookFunction((void *) getAbsoluteAddress(libName, 0x767568), (void *) &_Player_ondestroy,
                   (void **) &Player_ondestroy); //Player.OnDestroy();
                   
    return NULL;
}

extern "C" {
JNIEXPORT jobjectArray
JNICALL
Java_uk_lgl_modmenu_FloatingModMenuService_playerList(JNIEnv *env, jobject context) {
    jobjectArray ret;
    const char *features[] = {      
            OBFUSCATE("0_Toggle_Telekill"),//0
			OBFUSCATE("1_Toggle_Masskill"),//1
            OBFUSCATE("2_Toggle_Fly hack [buggy]"),//2        
            OBFUSCATE("-5_Button_BACK"),
    };
    int Total_Feature = (sizeof features / sizeof features[0]);
    ret = (jobjectArray)
    env->NewObjectArray(Total_Feature, env->FindClass(OBFUSCATE("java/lang/String")),
    env->NewStringUTF(""));
    
    for (int i = 0; i < Total_Feature; i++)
    env->SetObjectArrayElement(ret, i, env->NewStringUTF(features[i]));
    
    return (ret);
}

JNIEXPORT jobjectArray
JNICALL
Java_uk_lgl_modmenu_FloatingModMenuService_visualsList(JNIEnv *env, jobject context) {
    jobjectArray ret;
    const char *features[] = {                 
            OBFUSCATE("3_Toggle_Default Chams"),//3
            OBFUSCATE("4_Toggle_Wireframe Chams"),//4
            OBFUSCATE("5_Toggle_Glow Chams"),//5
            OBFUSCATE("6_Toggle_Outline Chams"),//6
            OBFUSCATE("7_Toggle_Rainbow Chams"),//7
            OBFUSCATE("8_SeekBar_Line Width_0_10"),//8
            OBFUSCATE("9_SeekBar_Color Red_0_255"),//9
            OBFUSCATE("10_SeekBar_Color Green_0_255"),//10
            OBFUSCATE("11_SeekBar_Color Blue_0_255"),//11
            
            OBFUSCATE("12_Toggle_Esp"),//12
            OBFUSCATE("13_Toggle_Esp Line"),//13
            OBFUSCATE("14_Toggle_Esp Box"),//14
            OBFUSCATE("15_SeekBar_Esp Box Type_0_1"),//15
            OBFUSCATE("16_SeekBar_Esp Box Width_0_100"),//16
            OBFUSCATE("17_SeekBar_Esp Color_0_7"),//17
            OBFUSCATE("18_SeekBar_Esp Hight_0_100"),//18
            OBFUSCATE("19_SeekBar_Esp Box Size_0_100"),//19          
            OBFUSCATE("-5_Button_BACK"),
    };
    int Total_Feature = (sizeof features / sizeof features[0]);
    ret = (jobjectArray)
    env->NewObjectArray(Total_Feature, env->FindClass(OBFUSCATE("java/lang/String")),
    env->NewStringUTF(""));
    
    for (int i = 0; i < Total_Feature; i++)
    env->SetObjectArrayElement(ret, i, env->NewStringUTF(features[i]));
    
    return (ret);
}

JNIEXPORT jobjectArray
JNICALL
Java_uk_lgl_modmenu_FloatingModMenuService_visualsSettingsList(JNIEnv *env, jobject context) {
    jobjectArray ret;
    const char *features[] = {          
            OBFUSCATE("-6_Toggle_Auto size menu"),         
            OBFUSCATE("-5_Button_BACK"),
    };
    int Total_Feature = (sizeof features / sizeof features[0]);
    ret = (jobjectArray)
    env->NewObjectArray(Total_Feature, env->FindClass(OBFUSCATE("java/lang/String")),
    env->NewStringUTF(""));                           
    
    for (int i = 0; i < Total_Feature; i++)
    env->SetObjectArrayElement(ret, i, env->NewStringUTF(features[i]));
    
    return (ret);
}

JNIEXPORT jobjectArray
JNICALL
Java_uk_lgl_modmenu_FloatingModMenuService_otherList(JNIEnv *env, jobject context) {
    jobjectArray ret;
    const char *features[] = {                        
            OBFUSCATE("20_Toggle_Speed hack"),//20
            OBFUSCATE("21_SeekBar_Speed_0_100"),//21
            OBFUSCATE("22_Toggle_Enemy Scale"),//22       
            OBFUSCATE("23_SeekBar_Enemy Scale X_0_25"),//23
            OBFUSCATE("24_SeekBar_Enemy Scale Y_0_25"),//24
            OBFUSCATE("25_SeekBar_Enemy Scale Z_0_25"),//25     
            OBFUSCATE("26_Toggle_No AntiHack"),//26
            OBFUSCATE("-5_Button_BACK"),
    };

    int Total_Feature = (sizeof features / sizeof features[0]);
    ret = (jobjectArray)
    env->NewObjectArray(Total_Feature, env->FindClass(OBFUSCATE("java/lang/String")),
    env->NewStringUTF(""));
    
    for (int i = 0; i < Total_Feature; i++)
    env->SetObjectArrayElement(ret, i, env->NewStringUTF(features[i]));
    
    return (ret);
}

JNIEXPORT jobjectArray
JNICALL
Java_uk_lgl_modmenu_FloatingModMenuService_getFeatureList(JNIEnv *env, jobject context) {
    jobjectArray ret;
    const char *features[] = {
            OBFUSCATE("-1_Button_PLAYER"),
            OBFUSCATE("-2_Button_VISUALS"),
            OBFUSCATE("-3_Button_VISUALS SETTINGS"),
            OBFUSCATE("-4_Button_OTHER"),
    };

    int Total_Feature = (sizeof features / sizeof features[0]);
    ret = (jobjectArray)
    env->NewObjectArray(Total_Feature, env->FindClass(OBFUSCATE("java/lang/String")),
    env->NewStringUTF(""));
    
    for (int i = 0; i < Total_Feature; i++)
    env->SetObjectArrayElement(ret, i, env->NewStringUTF(features[i]));
    
    return (ret);
}
JNIEXPORT void JNICALL
Java_uk_lgl_modmenu_Preferences_Changes(JNIEnv *env, jclass clazz, jobject obj,
                                        jint featNum, jstring featName, jint value,
                                        jboolean boolean, jstring str) {
                                            
    switch (featNum) {
        case 0:           
            telekill = boolean;
            break;
        case 1:           
            masskill = boolean;
            break;
        case 2:           
            Fly = boolean;
            break;    
        case 3:           
        chams = boolean;
        if (chams) {
        SetWallhack(true);       
        } else {
        SetWallhack(false);               
        }
        break;    
        case 4:           
        wireframe = boolean;
        if (wireframe) {
        SetWallhackW(true);
        } else {
        SetWallhackW(false);
        }
        break;    
        case 5:          
        glow = boolean;
        if (glow) {
        SetWallhackG(true);
        } else {
        SetWallhackG(false);
        }
        break;     
        case 6:          
        outline = boolean;
        if (outline) {
        SetWallhackO(true);
        } else {
        SetWallhackO(false);
        }
        break;    
        case 7:           
        rainbow = boolean;
        if (rainbow) {
        SetRainbow(true);
        } else {
        SetRainbow(false);
        }
        break;   
        case 8:                
            SetW(value);
            break;    
        case 9:            
            SetR(value);
            break;
        case 10:                      
            SetG(value);
            break;    
        case 11:           
            SetB(value);
            break;   
        case 12:           
            ESP = boolean;
            break;    
        case 13:                    
            ESPLine = boolean;
            break;    
        case 14:           
            ESPBox = boolean;
            break;    
        case 15:                
            switch (value) {
                case 0:
                    ESPBoxStyle = 1;
                    break;
                case 1:
                    ESPBoxStyle = 2;
                    break;               
            }
            break;    
        case 16:                     
            upval3 = (float) value * 0.1;
            break;         
        case 17:                 
            switch (value) {
                case 0:
                    color = Color::Red(255);
                    break;
                case 1:
                    color = Color::Green(255);
                    break;
                case 2:
                    color = Color::Cyan(255);
                    break; 
                case 3:
                    color = Color::Black(255);
                    break;
                case 4:
                    color = Color::White(255);
                    break;
                case 5:
                    color = Color::Blue(255);
                    break; 
                case 6:
                    color = Color::Yellow(255);
                    break;
                case 7:
                    color = Color::Magenta(255);
                    break;  
            }
            break;           
        case 18:                    
            upval = (float) value * 0.1;
            break;    
        case 19:                 
            upval2 = (float) value * 0.1;
            break;         
        case 20:           
            speedHack = boolean;
            break;
        case 21:           
            Speed = value;
            break;    
        case 22:           
            enemyscale = boolean;
            break;        
        case 23:                 
            scaleX = value;
            break;   
        case 24:                      
            scaleY = value;
            break;     
        case 25:               
            scaleZ = value;
            break;   
        case 26:               
            noAntiHack = boolean;
            break;     
      }
   }
}

__attribute__((constructor))
void lib_main() {
 
    pthread_t ptid;
    pthread_create(&ptid, NULL, hack_thread, NULL);
}
