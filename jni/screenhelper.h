#include "shared_header.h"

#define SEND_EVENT_MACRO(_ts, _type, _code, _value) \
    /* printf("sending %ld.%ld " #_type " " #_code " %08x\n", _ts.tv_sec, _ts.tv_usec, _value); */ \
    m_Sender.SendEvent({    \
        .time = _ts,             \
        .type = _type,         \
        .code = _code,    \
        .value = _value,          \
    })
#define SEND_EVENT_SYNC() SEND_EVENT_MACRO(ts, EV_SYN, SYN_REPORT, 0)


class GamePadHelper
{
};

class EventSender
{
public:
    EventSender(const char* input_path)
    {
        m_fd = open(input_path, O_RDWR);
        if(m_fd < 0) {
            fprintf(stderr, "could not open %s, %s\n", input_path, strerror(errno));
            return;
        }
        int version = 0;
        if (ioctl(m_fd, EVIOCGVERSION, &version)) {
            fprintf(stderr, "could not get driver version for %s, %s\n", input_path, strerror(errno));
            return;
        }
    }
    ~EventSender()
    {
        if(m_fd >= 0)
            close(m_fd);
    }
    bool SendEvent(struct input_event&& event)
    {
        if(m_fd < 0)
            return false; 
        ssize_t ret = write(m_fd, &event, sizeof(event));
        if(ret < (ssize_t) sizeof(event)) {
            fprintf(stderr, "write event failed, %s\n", strerror(errno));
            return false;
        }
        return true;
    }
private:
    int m_fd = -1;
};

class MultiTouchHelper {
    int m_ActiveSlotNum = 0;

    EventSender& m_Sender;
    int m_Slots[10] = { 0 };
    int m_LastTrackingId = 0x10000000;

    int GetSlot(int tracking_id){
        int slot = -1;
        for (int i=0; i<sizeof(m_Slots)/sizeof(int); i++) {
            if (m_Slots[i] == tracking_id) {
                slot = i;
                break;
            }
        }
        return slot;
    }
    int GetActiveSlotNum() {
        return m_ActiveSlotNum;
    }

    int GetFirstEmptySlot() {
        for (int i=0; i<sizeof(m_Slots)/sizeof(int); i++) {
            if (m_Slots[i] == 0) {
                return i;
            }
        }
        return -1;
    }

public:
    MultiTouchHelper(EventSender& sender) : m_Sender(sender) {}

    int FingerDown(struct timeval ts, ScreenPos pos) {
        int slot = GetFirstEmptySlot();
        assert(slot != -1);
        int tracking_id = ++m_LastTrackingId;
        m_Slots[slot] = tracking_id;
        m_ActiveSlotNum++;
        
        SEND_EVENT_MACRO(ts, EV_ABS, ABS_MT_SLOT, slot);
        SEND_EVENT_MACRO(ts, EV_ABS, ABS_MT_TRACKING_ID, tracking_id);

        if (GetActiveSlotNum() == 1) {
            SEND_EVENT_MACRO(ts, EV_KEY, BTN_TOOL_FINGER, 1);
        }
        // send pos && sync
        SEND_EVENT_MACRO(ts, EV_ABS, ABS_MT_POSITION_X, pos.x);
        SEND_EVENT_MACRO(ts, EV_ABS, ABS_MT_POSITION_Y, pos.y);
        SEND_EVENT_SYNC();
        // printf("finger %d down at [%d, %d], total: %d\n", slot, pos.x, pos.y, GetActiveSlotNum());
        return slot;
    }

    void FingerUp(struct timeval ts, int slot) {
        int tracking_id = m_Slots[slot];
        if (tracking_id == 0) return;
        
        SEND_EVENT_MACRO(ts, EV_ABS, ABS_MT_SLOT, slot);

        // slot up
        SEND_EVENT_MACRO(ts, EV_ABS, ABS_MT_TRACKING_ID, -1);

        if (GetActiveSlotNum() == 1) {
            // last fingers up
            SEND_EVENT_MACRO(ts, EV_KEY, BTN_TOOL_FINGER, 0);
        }

        // sync_report
        SEND_EVENT_SYNC();
        
        // clear slot
        m_Slots[slot] = 0;
        m_ActiveSlotNum--;
        // printf("finger %d up, total: %d\n", slot, GetActiveSlotNum());
    }
    
    void FingerUpdatePos(struct timeval ts, int slot, ScreenPos new_pos) {
        SEND_EVENT_MACRO(ts, EV_ABS, ABS_MT_SLOT, slot);
        SEND_EVENT_MACRO(ts, EV_ABS, ABS_MT_POSITION_X, new_pos.x);
        SEND_EVENT_MACRO(ts, EV_ABS, ABS_MT_POSITION_Y, new_pos.y);
        SEND_EVENT_SYNC();
        // printf("finger %d update, total: %d\n", slot, GetActiveSlotNum());
    }
};