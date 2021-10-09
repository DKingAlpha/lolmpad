#include "shared_header.h"

namespace ButtonPos {
    ScreenPos Walk   = {304, 340};
    ScreenPos Attack = {215, 1928};
    
    ScreenPos SkillQ = {143, 1631};
    ScreenPos SkillW = {310, 1677};
    ScreenPos SkillE = {432, 1780};
    ScreenPos SkillR = {500, 1928};
    ScreenPos SkillD = {140, 1300};
    ScreenPos SkillF = {140, 1420};
    ScreenPos SkillB = {133, 760};

    ScreenPos Ward = {702, 1934};
    
    ScreenPos Attack_Minion = {76, 1787};
    ScreenPos Attack_Turret = {365, 2005};

    
    ScreenPos Scoreboard = {1036, 1962};
    ScreenPos Shop = {650, 68};

    ScreenPos CancelSkill = {850, 1945};
    ScreenPos CancelUltra = {990, 1960};

    ScreenPos ShopItem1 = {650, 180};
    ScreenPos ShopItem2 = {535, 180};
}

class PadStatus {
public:
    int ABS[ABS_MAX] = {0};
    int KEY[KEY_MAX] = {0};
    int Modifier = 0;

    int KEY_SLOTS[KEY_MAX] = {0};

    PadStatus() {
        CenterStickLeft();
        CenterStickRight();
        Modifier = 0;
        memset(KEY_SLOTS, 0xff, sizeof(KEY_SLOTS));
    }

    void CenterStickLeft() {
        ABS[ABS_X] = 0x8000;
        ABS[ABS_Y] = 0x8000;
    }
    void CenterStickRight() {
        ABS[ABS_Z] = 0x8000;
        ABS[ABS_RZ] = 0x8000;
    }
};

enum GamepadPartType {
    GAMEPAD_PART_LEFTSTICK,
    GAMEPAD_PART_RIGHTSTICK,
};

class GamepadStick {
    PadStatus& m_PadStatus;
    MultiTouchHelper& m_Screen;

    ScreenPos m_Center;
    GamepadPartType m_PartType;

    ScreenPos m_DefaultCenter;
    int m_DefaultRadius;
    int m_FingerSlot = -1;
    int m_Radius = 128;
    bool m_Deactive = false;
public:
    GamepadStick(PadStatus& pad_status, MultiTouchHelper& screen,
                 ScreenPos center, GamepadPartType part_type, int radius) : 
        m_PadStatus(pad_status), m_Screen(screen), m_Center(center),m_DefaultCenter(center),
        m_PartType(part_type), m_Radius(radius), m_DefaultRadius(radius) {}

    void OnABS(struct timeval ts, int type, int code, int value)
    {
        int x,y;
        if (m_PartType == GAMEPAD_PART_LEFTSTICK) {
            x=m_PadStatus.ABS[ABS_Y];
            y=m_PadStatus.ABS[ABS_X];
        }
        else if (m_PartType == GAMEPAD_PART_RIGHTSTICK) {
            x=m_PadStatus.ABS[ABS_RZ];
            y=m_PadStatus.ABS[ABS_Z];
        } else {
            assert(false);
        }
        
        // auto release in deadzone, if no modifier.
        if (abs(x-0x8000) < 0x1000 && abs(y-0x8000) < 0x1000) {
            if (m_PartType == GAMEPAD_PART_LEFTSTICK || !m_PadStatus.Modifier)
            {
                EnsureUp(ts);
                return;
            }
        }
        
        if (m_Deactive) {
            return;
        }
        
        EnsureDown(ts, m_Center);

        if (m_FingerSlot>=0) {
            ScreenPos new_pos = {m_Center.x-m_Radius*(x-0x8000)/0x8000, m_Center.y+m_Radius*(y-0x8000)/0x8000};
            // printf("[%ld, %ld]\n", new_pos.x, new_pos.y);
            m_Screen.FingerUpdatePos(ts, m_FingerSlot, new_pos);
        }
    }

    void ResetDefault(struct timeval ts)
    {
        UpdateStickConfig(ts, m_DefaultCenter, m_DefaultRadius);
    }

    void UpdateStickConfig(struct timeval ts, ScreenPos center, int radius)
    {
        EnsureUp(ts);
        m_Center = center;
        m_Radius = radius;
    }

    void EnsureUp(struct timeval ts)
    {
        m_Deactive = false;
        if (m_FingerSlot>=0)
        {
            m_Screen.FingerUp(ts, m_FingerSlot);
        }
        m_FingerSlot = -1;
        
        m_PartType == GAMEPAD_PART_LEFTSTICK ? m_PadStatus.CenterStickLeft() : m_PadStatus.CenterStickRight();
    }

    void EnsureDown(struct timeval ts, ScreenPos pos)
    {
        if (m_FingerSlot < 0) {
            m_FingerSlot = m_Screen.FingerDown(ts, m_Center);
        }
    }

    int GetFingerSlot() { return m_FingerSlot; }

    void DeactiveUntilUp() {
        m_Deactive = true;
    }
};

class Gamepad {
    MultiTouchHelper& m_Screen;

    GamepadStick* m_StickWalk = nullptr;
    GamepadStick* m_StickSelect = nullptr;
    PadStatus m_PadStatus;
public:
    Gamepad(MultiTouchHelper& screen) :
        m_Screen(screen)
    {
        m_StickWalk = new GamepadStick(m_PadStatus, screen, ButtonPos::Walk, GAMEPAD_PART_LEFTSTICK, 160);
        m_StickSelect = new GamepadStick(m_PadStatus, screen, ButtonPos::Attack, GAMEPAD_PART_RIGHTSTICK, 200);
    }
    ~Gamepad()
    {
        delete m_StickWalk;
        delete m_StickSelect;
    }
    void OnEvent(struct timeval ts, int type, int code, int value)
    {
        bool is_upgrading_skill = m_PadStatus.KEY[BTN_THUMBL];
        if (type == EV_ABS) {
            int old_value = m_PadStatus.ABS[code];
            int new_value = value;
            m_PadStatus.ABS[code] = new_value;
            switch (code) {
                case ABS_X:
                case ABS_Y:
                    m_StickWalk->OnABS(ts, type, code, value);
                    break;
                case ABS_Z:
                case ABS_RZ:
                    m_StickSelect->OnABS(ts, type, code, value);
                    break;
                case ABS_GAS:
                case ABS_BRAKE:
                    // regulate
                    new_value = value > 0x320 ? 1 : 0;
                    m_PadStatus.ABS[code] = new_value;
                    // update modifier
                    if (new_value) {
                        m_PadStatus.Modifier = code;
                    } else {
                        if (m_PadStatus.Modifier == code)
                            m_PadStatus.Modifier = 0;
                    }
                    // LT/RT
                    if (new_value != old_value) {
                        // flip-flop
                        if (code == ABS_BRAKE) {    // RT
                            m_StickSelect->UpdateStickConfig(ts, ButtonPos::SkillE, 200);
                        } else { // LT
                            m_StickSelect->UpdateStickConfig(ts, ButtonPos::SkillR, 200);
                        }
                        if (new_value) {
                            m_StickSelect->OnABS(ts, type, code, value);
                        }
                        else {
                            m_StickSelect->ResetDefault(ts);
                        }
                    }
                    break;

                // DPAD
                case ABS_HAT0X:
                case ABS_HAT0Y:
                    {
                        int ref_value = 0;
                        if (old_value == 0) {
                            // do press
                            ref_value = new_value;
                        } else /* new_value == 0 */ {
                            // do release
                            ref_value = old_value;
                        }
                        int dpad_btn = 0;
                        ScreenPos ref_pos;
                        if (ref_value == -1 && code == ABS_HAT0X) {         // left
                            dpad_btn = BTN_DPAD_LEFT;
                            ref_pos = ButtonPos::Attack_Minion;
                        } else if (ref_value == 1 && code == ABS_HAT0X) {   // right
                            dpad_btn = BTN_DPAD_RIGHT;
                            ref_pos = ButtonPos::Attack_Turret;
                        } else if (ref_value == -1 && code == ABS_HAT0Y) {  // up
                            dpad_btn = BTN_DPAD_UP;
                            ref_pos = ButtonPos::ShopItem1;
                        } else if (ref_value == 1 && code == ABS_HAT0Y) {   // down
                            dpad_btn = BTN_DPAD_DOWN;
                            ref_pos = ButtonPos::ShopItem2;
                        }

                        assert(dpad_btn);

                        if (old_value == 0) {
                            // do press
                            assert (m_PadStatus.KEY_SLOTS[dpad_btn] == -1);
                            m_PadStatus.KEY_SLOTS[dpad_btn] = m_Screen.FingerDown(ts, ref_pos);
                        } else /* new_value == 0 */ {
                            // do release
                            if (m_PadStatus.KEY_SLOTS[dpad_btn] != -1) {
                                m_Screen.FingerUp(ts, m_PadStatus.KEY_SLOTS[dpad_btn]);
                                m_PadStatus.KEY_SLOTS[dpad_btn] = -1;
                            }
                        }
                    }
                    break;
                default:
                    // printf("unknown event\n");
                    break;
            }
        }
        else if (type == EV_KEY) {
            // stateless action
            if (code == BTN_THUMBR)
            {
                if (m_PadStatus.Modifier)
                {
                    // cancel skill
                    if (value) {
                        m_PadStatus.Modifier == ABS_GAS ?
                            m_Screen.FingerUpdatePos(ts, m_StickSelect->GetFingerSlot(), ButtonPos::CancelUltra) : 
                            m_Screen.FingerUpdatePos(ts, m_StickSelect->GetFingerSlot(), ButtonPos::CancelSkill);
                        m_StickSelect->DeactiveUntilUp();
                    } else {
                        m_StickSelect->ResetDefault(ts);
                        m_PadStatus.Modifier = 0;
                    }
                } else {
                    // up finger to select target
                    if (value) {
                        m_StickSelect->EnsureUp(ts);
                        m_StickSelect->DeactiveUntilUp();
                    } else {
                    }
                }
                return;
            }

            int old_value = m_PadStatus.KEY[code];
            int new_value = value;
            m_PadStatus.KEY[code] = new_value;

            // update modifier
            if (new_value) {
                m_PadStatus.Modifier = code;
            } else {
                if (m_PadStatus.Modifier == code)
                    m_PadStatus.Modifier = 0;
            }
            if (new_value != old_value) {
                // flip-flop

                // LT/RT/B
                if (code == BTN_TL || code == BTN_TR || code == BTN_START) {
                    if (code == BTN_TL) // LS
                        m_StickSelect->UpdateStickConfig(ts, ButtonPos::SkillQ, 200);
                    if (code == BTN_TR) // RS
                        m_StickSelect->UpdateStickConfig(ts, ButtonPos::SkillW, 200);
                    if (code == BTN_START) // START
                        m_StickSelect->UpdateStickConfig(ts, ButtonPos::Ward, 200);
                    
                    if (new_value) {
                        m_StickSelect->OnABS(ts, type, code, value);
                    }
                    else {
                        m_StickSelect->ResetDefault(ts);
                    }
                    return;
                }

                // buttons
                ScreenPos new_center;
                bool is_hit = true;
                switch (code) {
                    case BTN_SOUTH:   // A
                        new_center = is_upgrading_skill ? ButtonPos::SkillQ : ButtonPos::Attack;
                        break;
                    case BTN_EAST:   // B
                        new_center = is_upgrading_skill ? ButtonPos::SkillR : ButtonPos::SkillB;
                        break;
                    case BTN_NORTH:   // X
                        new_center = is_upgrading_skill ? ButtonPos::SkillW : ButtonPos::SkillD;
                        break;
                    case BTN_WEST:   // Y
                        new_center = is_upgrading_skill ? ButtonPos::SkillE : ButtonPos::SkillF;
                        break;
                    case KEY_BACK:
                        new_center = ButtonPos::Scoreboard;
                        break;
                    /* axis modifier
                    case KEY_START:   // START
                        new_center = ButtonPos::Ward;
                        break;
                    */
                    default:
                        is_hit = false;
                }
                if (is_hit) {
                    // upgrading skill
                    if (is_upgrading_skill) {
                        new_center.x += 110;
                        new_center.y -= 90;
                    }
                    if (old_value == 0) {
                        // do press
                        assert (m_PadStatus.KEY_SLOTS[code] == -1);
                        m_PadStatus.KEY_SLOTS[code] = m_Screen.FingerDown(ts, new_center);
                    } else /* new_value == 0 */ {
                        // do release
                        if (m_PadStatus.KEY_SLOTS[code] != -1) {
                            m_Screen.FingerUp(ts, m_PadStatus.KEY_SLOTS[code]);
                            m_PadStatus.KEY_SLOTS[code] = -1;
                        }
                    }
                }
            }
        } else {
            // ignored
        }
    }

};
