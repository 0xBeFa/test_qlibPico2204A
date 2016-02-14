#ifndef QLIBPICO2204A_INTERFACE_H
#define QLIBPICO2204A_INTERFACE_H

#include <QObject>
#include "../common.h"

class qlibPico2204A_Interface
{
public:
    virtual ~qlibPico2204A_Interface() {}
};

Q_DECLARE_INTERFACE(qlibPico2204A_Interface, "befa.qlibPico2204A_Interface/1.0")

#endif // QLIBPICO2204A_INTERFACE_H
