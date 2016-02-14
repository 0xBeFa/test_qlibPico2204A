#ifndef QLIBPICOWAVE_DATA_FORMAT
#define QLIBPICOWAVE_DATA_FORMAT

#endif // QLIBPICOWAVE_DATA_FORMAT
#include "../common.h"
#include <QVector>


struct ST_qlibpicowave_data_format
{
    QVector<float32>    vf32_data_max;
    QVector<float32>    vf32_data_min;
    float32   f32_time_per_sample;
    QString   q_Name;
};
