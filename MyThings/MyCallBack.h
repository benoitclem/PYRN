
#ifndef MY_CALLBACK_H
#define MY_CALLBACK_H

class MyCallBack {
public:
    MyCallBack() {};
    virtual void event(int ID, void *data) = 0;
};

#endif // MY_CALLBACK_H