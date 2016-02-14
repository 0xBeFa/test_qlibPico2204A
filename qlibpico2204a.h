#ifndef QLIBPICO2204A_H
#define QLIBPICO2204A_H

#include <QVector>
#include <QObject>
#include <QTimer>
#include "../common.h"
#include "qlibpico2204a_interface.h"
#include <ps2000.h>

class qlibPico2204A;

#define PS2000_FULL_SCALE_VOLTAGE 32767.0


//-------------------------------------
// Static Data
//-------------------------------------
struct ST_DATA
{
    bool             b_in_use;           // every object that uses this struct to reveice data has to set this to true
    qlibPico2204A*   pc_scope;           // pointer to the object that uses this struct
    uint32           u32_max_samples;    // maxium samples to be collected in fast streaming mode, streaming will be stoped if this is exceeded
    uint32           u32_collected_samples;
    bool             b_collecting;
    QVector<sintx16*> avps16_data_CH1[2];
    QVector<uintx32>  vu32_length_CH1;
    QVector<sintx16*> vps16_data_CH2[2];
    QVector<uintx32>  vu32_length_CH2;
};

struct ST_STATIC_RECEIVER_DATA
{
    ST_DATA ast_data[64];
};
//-------------------------------------
// end Static Data
//-------------------------------------


struct ST_SCOPE_DATA
{
    ST_SCOPE_DATA();
    ST_SCOPE_DATA(ST_SCOPE_DATA* pst_copy);
    ~ST_SCOPE_DATA();
    uint32  u32_id;
    bool    b_in_connection;
    bool    b_connected;
    bool    b_closed;
    sint16  s16_handle;
    QString q_serial_number;
};


struct ST_GLOBAL_DATA
{
    bool b_use;
    bool b_scan;
    ST_SCOPE_DATA st_scope;
    ST_SCOPE_DATA st_scope_temp;
    uint32 u32_connect_timeout_counter;
    uint32 u32_connect_timeout;
    QVector<sint16> vs16_scan_handles;
    QTimer* pq_Timer;
    QVector<float32> avf32_Data_CH1[2];
    QVector<float32> avf32_Data_CH2[2];
};

class qlibPico2204A : public QObject ,public qlibPico2204A_Interface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "befa.Plugin.qlibPico2204A_Interface")
    Q_INTERFACES(qlibPico2204A_Interface)
public:
    qlibPico2204A(QObject *parent = 0);

    static void __stdcall FastStreamingCallback(  sint16** pps16_overviewBuffers,
                                        sint16   s16_overflow,
                                        uint32   u32_triggeredAt,
                                        sint16   s16_triggered,
                                        sint16   s16_auto_stop,
                                        uint32   u32_nValues );
    static void __stdcall FastStreamingCallback2(sintx16 **pps16_overviewBuffers,
                                        sintx16 s16_overflow,
                                        uintx32 u32_triggeredAt,
                                        sintx16 s16_triggered,
                                        sintx16 s16_auto_stop,
                                        uintx32 u32_nValues );

    static ST_STATIC_RECEIVER_DATA st_srd;
public slots:
    void use(bool b_use);
    void scan(void);
    void SetTargetSN(const QString& rq_str);
    void StartFastStreaming(void);
    void test_fgen(void);
    void SaveWaveformToFile(QString& rq_save_path);
signals:
    void deb( const QString& rq_str );
private slots:
    void tick_100ms(void);
    void _FinishFastStreaming(void);
private:
    ST_GLOBAL_DATA  gd; //global data
    ST_DATA*        sr; //static receiver

    void ScanTick(void);
    void UseTick(void);

    QString q_target_SN;

    void _test_post_streaming();
    sint8 as8_buffer[256];

};

#endif // QLIBPICO2204A_H

