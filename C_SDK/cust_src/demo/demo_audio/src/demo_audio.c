#include "string.h"
#include "iot_debug.h"
#include "iot_audio.h"

#define audio_print iot_debug_print
HANDLE g_demo_timer1;

/* 注: 每次20ms一帧, 不同的音频类型一帧长度不同
    音频类型                            一帧长度
    OPENAT_REC_MODE_AMR475,             0x0000000D,
    OPENAT_REC_MODE_AMR515,             0x0000000E,
    OPENAT_REC_MODE_AMR59,              0x00000010,
    OPENAT_REC_MODE_AMR67,              0x00000012,
    OPENAT_REC_MODE_AMR74,              0x00000014,
    OPENAT_REC_MODE_AMR795,             0x00000015,
    OPENAT_REC_MODE_AMR102,             0x0000001B,
    OPENAT_REC_MODE_AMR122,             0x00000020,

    每秒钟录音数据的大小 = 1000(ms) / 20(ms) * 帧长度
    例如OPENAT_REC_MODE_AMR795 每秒钟数据大小为1000 / 20 * 21
*/
#define DEMO_RECORD_LENGTH 96000 
#define DEMO_RECORD_AMR_HEAD_BUFF "#!AMR\n"
#define DEMO_RECORD_AMR_HEAD_LEN 6
#define DEMO_RECODE_AMR_FRAME_LEN (21)
#define DEMO_RECORD_MODE OPENAT_REC_MODE_AMR795 
#define DEMO_RECORD_CH OPENAT_AUD_CHANNEL_LOUDSPEAKER
#define DEMO_CYCLE_BUFF_LEN (0X10000)
#define DEMO_TIMER_TIMEOUT (5000) // 5000ms

static UINT8  g_s_audio_rec_buff[DEMO_RECORD_LENGTH];
static UINT32 g_s_audio_rec_total_len = DEMO_RECORD_AMR_HEAD_LEN;
static UINT32 g_s_first_play_len = 0;
static UINT8 g_s_cycle_buff[DEMO_CYCLE_BUFF_LEN];

static E_AMOPENAT_PLAY_MODE demo_play_format_get(void)
{
    switch(DEMO_RECORD_MODE)
    {
        case OPENAT_REC_MODE_AMR475:
        case OPENAT_REC_MODE_AMR515:
        case OPENAT_REC_MODE_AMR59:
        case OPENAT_REC_MODE_AMR67:
        case OPENAT_REC_MODE_AMR74:
        case OPENAT_REC_MODE_AMR795:
        case OPENAT_REC_MODE_AMR102:
        case OPENAT_REC_MODE_AMR122:
        case OPENAT_REC_MODE_AMR_RING:
            return OPENAT_AUD_PLAY_MODE_AMR_RING;

        default:
            return OPENAT_AUD_PLAY_MODE_QTY; // 其他的音频暂时不支持
    }
}

VOID demo_paly_handle(E_AMOPENAT_PLAY_ERROR result, int *len)
{
    /*
        1.通过int *len;来判断播放是否结束
            如果len不存在, 表示播放结束
            如果len存在.可以通过设置*len和g_s_cycle_buff,设置下一个播放的数据的长度和内容,
            当*len设置为0,表示下一个播放buff为空,会终止播放
          注:
             给*len和g_s_cycle_buff需要设置帧的整数倍, 如下demo操作
    */
    static UINT32 total_len = 0;
    
    int frame_num = 1; // 添加到g_s_cycle_buff中的帧个数

    if (total_len == 0)
    {
        total_len += g_s_first_play_len;
    }
   
    if (len)
    {
        if (total_len + DEMO_RECODE_AMR_FRAME_LEN > g_s_audio_rec_total_len)
        {
            //数据播放结束给*len复制为0
            *len = 0;
            audio_print("[audio] demo_paly_handle data copy end %d", result);
            return;
        }
        // 给g_s_cycle_buff中填充数据
        memcpy(g_s_cycle_buff, &g_s_audio_rec_buff[total_len], DEMO_RECODE_AMR_FRAME_LEN*frame_num);
        
        // 填充数据的长度
        *len = DEMO_RECODE_AMR_FRAME_LEN*frame_num;
        total_len += *len;
        //audio_print("[audio] demo_paly_handle %x, %x, %x", total_len, g_s_audio_rec_total_len, g_s_audio_rec_buff[total_len]);
    }
    else
    {
        audio_print("[audio] demo_paly_handle play end %d", result);
    }
}

VOID demo_time_handle(T_AMOPENAT_TIMER_PARAMETER *pParameter)
{
    T_AMOPENAT_PLAY_PARAM playParam;
    BOOL err;

    //6. 关闭录音
    err = iot_audio_rec_stop();
    audio_print("[audio] AUDREC stop BOOL %d", err);
    
    //7. 播放录音
    // 第一次播放的长度, 
    g_s_first_play_len = DEMO_RECORD_AMR_HEAD_LEN + DEMO_RECODE_AMR_FRAME_LEN * 6;
    // 第一次播放的数据
    memcpy(g_s_cycle_buff, g_s_audio_rec_buff, g_s_first_play_len);
    
    playParam.playBuffer = TRUE;
    playParam.playBufferParam.callback = demo_paly_handle;
    playParam.playBufferParam.format = demo_play_format_get();
    playParam.playBufferParam.len = g_s_first_play_len;
    playParam.playBufferParam.pBuffer = g_s_cycle_buff;
    playParam.playBufferParam.loop = 0;
    err = iot_audio_play_music(&playParam);
    
    audio_print("[audio] AUDREC play BOOL %d", err);
}

VOID demo_audio_rec_handle(UINT8* data, UINT8 len)
{
    static BOOL init = FALSE; 

    //4. 设置buff amr head
    if (!init)
    {
        memset(g_s_audio_rec_buff, 0, DEMO_RECORD_LENGTH);
        memcpy(g_s_audio_rec_buff, DEMO_RECORD_AMR_HEAD_BUFF, DEMO_RECORD_AMR_HEAD_LEN);
    }
    init = TRUE;

    if (DEMO_RECORD_LENGTH < g_s_audio_rec_total_len + len)
        return;
    //5. 保存录音数据, 每次一帧
    memcpy(g_s_audio_rec_buff+g_s_audio_rec_total_len, data, len);
    g_s_audio_rec_total_len += len;
    
    //audio_print("[audio] AUDREC data total_len %x, %x", g_s_total_len, len);
}

VOID demo_audio_set_channel(VOID)
{
    // 设置通道
    switch(DEMO_RECORD_CH)
    {
        case OPENAT_AUD_CHANNEL_HANDSET:
            
            iot_audio_set_channel(OPENAT_AUD_CHANNEL_HANDSET);
            iot_audio_set_channel_with_same_mic(OPENAT_AUD_CHANNEL_HANDSET, OPENAT_AUD_CHANNEL_LOUDSPEAKER);
            break;

         case OPENAT_AUD_CHANNEL_LOUDSPEAKER:
         default:   
            iot_audio_set_channel(OPENAT_AUD_CHANNEL_LOUDSPEAKER);
            iot_audio_set_speaker_gain(OPENAT_AUD_SPK_GAIN_18dB);
            iot_audio_set_channel_with_same_mic(OPENAT_AUD_CHANNEL_HANDSET, OPENAT_AUD_CHANNEL_LOUDSPEAKER);
            break;   
    }

    audio_print("[audio] AUDREC channel %d", DEMO_RECORD_CH);
}


VOID demo_audRecStart(VOID)
{
    BOOL err = FALSE;

    //录音格式AMR795,
    err = iot_audio_rec_start(DEMO_RECORD_MODE, demo_audio_rec_handle);

    audio_print("[audio] AUDREC start BOOL %d", err);
}

VOID demo_audRecStopTimer(VOID)
{
    // 定时5秒钟停止录音
    g_demo_timer1 = iot_os_create_timer(demo_time_handle, NULL);
    iot_os_start_timer(g_demo_timer1, DEMO_TIMER_TIMEOUT);
}


VOID demo_audio_init(VOID)
{
   
    // 1.设置通道和声音
    demo_audio_set_channel();

    //2.  开始录音
    demo_audRecStart();

    //3. 设置定时关闭录音, 并播放录音
    demo_audRecStopTimer();
}

VOID app_main(VOID)
{
    audio_print("[audio] app_main");

    demo_audio_init();
}
