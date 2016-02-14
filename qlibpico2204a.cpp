#include "qlibpico2204a.h"
#include <QFile>
#include <QDataStream>


//Copy Constuctor for struct ST_SCOPE_DATA
ST_SCOPE_DATA::ST_SCOPE_DATA()
{
    b_closed = false;
    b_connected = false;
    b_in_connection = false;
    s16_handle = 0;
}
ST_SCOPE_DATA::ST_SCOPE_DATA(ST_SCOPE_DATA *pst_copy)
{
    b_closed = pst_copy->b_closed;
    b_connected = pst_copy->b_connected;
    b_in_connection = pst_copy->b_in_connection;
    q_serial_number = pst_copy->q_serial_number;
    s16_handle = pst_copy->s16_handle;
    u32_id = pst_copy->u32_id;
}
ST_SCOPE_DATA::~ST_SCOPE_DATA()
{

}

//constructor for static members
ST_STATIC_RECEIVER_DATA qlibPico2204A::st_srd;


qlibPico2204A::qlibPico2204A(QObject *parent) :
    QObject(parent)
{
    gd.b_use = false;
    gd.b_scan = false;

    gd.u32_connect_timeout = 300;
    gd.u32_connect_timeout_counter = 0;

    gd.pq_Timer = new QTimer();
    connect(gd.pq_Timer, SIGNAL(timeout()), this, SLOT(tick_100ms()));
    gd.pq_Timer->start(100);
}

void qlibPico2204A::tick_100ms()
{
    if( gd.b_use && gd.b_scan ) ScanTick();
    if( gd.b_use ) UseTick();
}

void qlibPico2204A::scan() { gd.b_scan = true; }


void qlibPico2204A::UseTick()
{
}

void qlibPico2204A::ScanTick()
{
    int i;
    sintx16 s16_ret;
    sintx16 s16_handle;
    sintx16 s16_progress;
    if( gd.st_scope_temp.b_in_connection == false &&
        gd.st_scope_temp.b_connected     == false &&
        gd.st_scope_temp.b_closed        == false )
    {
        /* ps2000_open_unit_async()
        This function opens a PicoScope 2000 Series oscilloscope without waiting for the
        operation to finish. You can find out when it has finished by periodically calling
        ps2000_open_unit_progress() until that function returns a non-zero value and a valid
        oscilloscope handle.
        The driver can support up to 64 oscilloscopes.
        Applicability All modes
        Arguments None
        Returns 0: if there is a previous open operation in progress.
        */
        s16_ret = ps2000_open_unit_async();
        if( s16_ret !=  0)
        {
            gd.st_scope_temp.b_in_connection = true;
            gd.u32_connect_timeout_counter = 0;
        }
    }
    else if( gd.st_scope_temp.b_in_connection == true  &&
             gd.st_scope_temp.b_connected     == false &&
             gd.st_scope_temp.b_closed        == false )
    {
        /* ps2000_open_unit_progress()
        This function checks on the progress of ps2000_open_unit_async().
        Applicability All modes.
        Use only with ps2000_open_unit_async().
        Arguments handle: a pointer to where the function should store the handle of
        the opened oscilloscope.
        0 if no oscilloscope is found or the oscilloscope fails to open,
        handle of oscilloscope (valid only if function returns 1)
        progress_percent: a pointer to an estimate of the progress
        towards opening the oscilloscope. The function will write a value
        from 0 to 100, where 100 implies that the operation is complete.
        Returns >0: if the driver successfully opens the oscilloscope
        0: if opening still in progress
        -1: if the oscilloscope failed to open or was not found
        */
        s16_ret = ps2000_open_unit_progress(&s16_handle, &s16_progress);
        if( (s16_progress == 100) && (s16_ret > 0) )
        {
            if( s16_handle != 0 )
            {
                gd.st_scope_temp.b_connected = true;
                gd.vs16_scan_handles += s16_handle;
            }
            else
            {
                emit(deb( "Scan finished, scope was not found" ));
                for( i=0; i<gd.vs16_scan_handles.count(); i++ )
                {
                    emit(deb(QString("closing scope Nr. ").append(QString::number(gd.vs16_scan_handles.at(i)))));
                    ps2000_close_unit(gd.vs16_scan_handles.at(i));
                }
                gd.b_scan = false;
            }
        }
        else
        {
            gd.u32_connect_timeout_counter++;
            if( gd.u32_connect_timeout_counter >= gd.u32_connect_timeout )
            {
                gd.st_scope_temp.b_in_connection = false;
                gd.st_scope_temp.b_connected     = false;
                gd.st_scope_temp.b_closed        = false;
            }
        }
    }
    else if( gd.st_scope_temp.b_in_connection == true  &&
             gd.st_scope_temp.b_connected     == true  &&
             gd.st_scope_temp.b_closed        == false )
    {
        s16_ret = ps2000_get_unit_info(gd.vs16_scan_handles.last(),as8_buffer,255, 3); // varaint Info
        if( QString((char*)as8_buffer).startsWith("2204A") )
        {
               s16_ret = ps2000_get_unit_info(gd.vs16_scan_handles.last(),as8_buffer,255, 4); // the batch and serial number of the oscilloscope
               gd.st_scope_temp.q_serial_number = QString((char*)as8_buffer);
               emit(deb(  QString("founnd scope ").append(gd.st_scope_temp.q_serial_number) ));
               if( gd.st_scope_temp.q_serial_number == q_target_SN )
               {
                   emit(deb( "--> target scope found" ));
                   gd.st_scope_temp.s16_handle = gd.vs16_scan_handles.takeLast();
                   gd.st_scope = gd.st_scope_temp;
                   for( i=0; i<gd.vs16_scan_handles.count(); i++ )
                   {
                       emit(deb(QString("closing scope Nr. ").append(QString::number(gd.vs16_scan_handles.at(i)))));
                       ps2000_close_unit(gd.vs16_scan_handles.at(i));
                   }
                   gd.b_scan = false;
               }
               else
               {
                   s16_ret = ps2000_close_unit(gd.vs16_scan_handles.last());
                   if( s16_ret == 0 ) emit(deb("ERROR: couldn't close scope unit!"));
               }
        }
        gd.st_scope_temp.b_in_connection = false;
        gd.st_scope_temp.b_connected     = false;
        gd.st_scope_temp.b_closed        = false;
    }
}

void qlibPico2204A::use(bool b_use)
{
    gd.b_scan = false;
    gd.b_use = b_use;
    emit(deb("test"));


}
void qlibPico2204A::SetTargetSN(const QString &rq_str) { q_target_SN = rq_str; }
void qlibPico2204A::StartFastStreaming()
{
    sint16 s16_ret;
    emit(deb(""));
    emit(deb("Fast streaming mode"));
    s16_ret = ps2000_set_channel(gd.st_scope.s16_handle, PS2000_CHANNEL_A, true, PS2000_DC_VOLTAGE, PS2000_1V);
    if( s16_ret == 0 ) emit(deb("ERROR: ps2000_set_channel failed"));
    s16_ret = ps2000_set_trigger(gd.st_scope.s16_handle, PS2000_NONE, 0, 0, 0, 0);
    if( s16_ret == 0 ) emit(deb("ERROR: ps2000_set_trigger failed"));
    s16_ret = ps2000_run_streaming_ns(  gd.st_scope.s16_handle,
                                        10,
                                        PS2000_US,
                                        1024*64,
                                        1,
                                        64,
                                        15000 );
    if( s16_ret == 0 ) emit(deb("ERROR: ps2000_run_streaming_ns failed"));

    sr = &st_srd.ast_data[0];
    sr->avps16_data_CH1[0].clear();
    sr->avps16_data_CH1[1].clear();
    sr->vu32_length_CH1.clear();
    sr->vps16_data_CH2[0].clear();
    sr->vps16_data_CH2[1].clear();
    sr->vu32_length_CH2.clear();
    sr->b_collecting = true;
    sr->u32_collected_samples = 0;
    QTimer::singleShot(250,this,SLOT(_FinishFastStreaming()));

}
void qlibPico2204A::_FinishFastStreaming()
{
    sint16 s16_ret;
    if( sr->b_collecting )
    {
        s16_ret = ps2000_get_streaming_last_values(gd.st_scope.s16_handle, qlibPico2204A::FastStreamingCallback2);
        //emit(deb("b_collecting = true"));
        //emit(deb(QString::number(s16_ret)));
        //emit(deb(QString::number(sr->u32_collected_samples)));
        if( (s16_ret==0) && (sr->u32_collected_samples!=0) )
        {
            sr->b_collecting = false;
        }
        QTimer::singleShot(250,this,SLOT(_FinishFastStreaming()));
    }
    else
    {
        _test_post_streaming();
    }
}

void qlibPico2204A::FastStreamingCallback(sint16** pps16_overviewBuffers, sint16 s16_overflow, uint32 u32_triggeredAt, sint16 s16_triggered, sint16 s16_auto_stop, uint32 u32_nValues)
{

}
void qlibPico2204A::FastStreamingCallback2(sintx16** pps16_overviewBuffers, sintx16 s16_overflow, uintx32 u32_triggeredAt, sintx16 s16_triggered, sintx16 s16_auto_stop, uintx32 u32_nValues)
{
    sintx16* ps16_mem1 = new sintx16[u32_nValues];
    sintx16* ps16_mem2 = new sintx16[u32_nValues];
    memcpy(ps16_mem1, pps16_overviewBuffers[0], u32_nValues*sizeof(sintx16));
    memcpy(ps16_mem2, pps16_overviewBuffers[1], u32_nValues*sizeof(sintx16));
    st_srd.ast_data[0].avps16_data_CH1[0] += ps16_mem1;
    st_srd.ast_data[0].avps16_data_CH1[1] += ps16_mem2;
    st_srd.ast_data[0].vu32_length_CH1    += u32_nValues;
    st_srd.ast_data[0].u32_collected_samples += u32_nValues;


    if( s16_auto_stop != 0) st_srd.ast_data[0].b_collecting = false;
}

void qlibPico2204A::_test_post_streaming()
{
    emit(deb("\npost data streaming handler"));
    emit(deb(QString::number(st_srd.ast_data[0].u32_collected_samples).prepend("total samples: ")));

    int n,i;

    gd.avf32_Data_CH1[0].clear();
    for(n=0; n<sr->avps16_data_CH1[0].count(); n++)
    {
        for(i=0; i<sr->vu32_length_CH1[n]; i++)
        {
            gd.avf32_Data_CH1[0].append( (float32)(sr->avps16_data_CH1[0][n][i]) / PS2000_FULL_SCALE_VOLTAGE );
            gd.avf32_Data_CH1[1].append( (float32)(sr->avps16_data_CH1[1][n][i]) / PS2000_FULL_SCALE_VOLTAGE );
        }
    }
}

void qlibPico2204A::test_fgen()
{
    sint16 s16_ret;
    s16_ret = ps2000_set_sig_gen_built_in(  gd.st_scope.s16_handle,
                                            0, //Offset = 000mV
                                            500000, //Peak to Peak = 500mV
                                            PS2000_SINE,
                                            10.0,
                                            10.0,
                                            0.0,
                                            0.0,
                                            PS2000_UP,
                                            1 );
    if( s16_ret == 0 ) emit(deb("test fgen failed"));
    else               emit(deb("test fgen succeeded"));
}

void qlibPico2204A::SaveWaveformToFile(QString &rq_save_path)
{
    QFile q_file(rq_save_path);
    if ( q_file.open(QIODevice::WriteOnly) == false)
    {
        emit(deb("ERROR: can't open file for write"));
        return;
    }
    QDataStream q_out(&q_file);
    emit(deb(QString("writing waveform to ").append(rq_save_path)));
    q_out << (QVector<float32>)gd.avf32_Data_CH1[0];
    q_out << (QVector<float32>)gd.avf32_Data_CH1[1];
    q_out << (float32) 0.001;
    q_out << (QString)"Test";
    q_file.close();
}









