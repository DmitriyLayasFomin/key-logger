#include "Key.h"
Key::Key(int virtualKey, int intValue)
{
    this->intValue = intValue;
    this->virtualKey = virtualKey;
}
void Key::press()
{
    keybd_event(NULL,
        this->intValue,
        KEYEVENTF_EXTENDEDKEY | 0,
        0);
};
void Key::release()
{
    keybd_event(NULL,
        this->virtualKey,
        KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP,
        0);
};
void Key::pressAndRelease(int delay = 500)
{
    this->press();
    Sleep(delay);
    this->release();
};
int Key::getHardValue()
{
    return this->intValue;
};
int Key::getVirtualKeyValue()
{
    return this->virtualKey;
};