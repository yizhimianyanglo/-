#ifndef GLOBALMACRO_H
#define GLOBALMACRO_H


namespace ModbusMacro{

enum devicStatus
{
    Offline=0,        // 离线
    Connecting,     // 正在连接
    Online,         // 在线
    Unstable,       // 不稳定（偶发错误）
    Error           // 故障（长期异常）
};

struct modbusRequest
{
    enum requestType {Read=0,Write};

    int slave;//从站
    int startAddress;//起始地址




};



}

class GlobalMacro
{
public:
    GlobalMacro();

};

#endif // GLOBALMACRO_H
