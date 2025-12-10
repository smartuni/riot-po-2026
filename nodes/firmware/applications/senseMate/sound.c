#include <stdio.h>

#include "include/sound.h"
#include "mutex.h"
#include "ztimer.h"
#include "thread.h"
#include "periph/pwm.h"

#define MSG_QUEUE_SIZE (8)
#define SOUND_BLOCKING_MSG_TYPE (0x77)
#define SOUND_ASYNC_MSG_TYPE    (0x78)

#define BEEP_SEQ_INIT(X) { .beeps = &X[0], .len = ARRAY_SIZE(X) }

typedef struct {
    beep_seq_t *beep_seq;
    mutex_t    playback_finished_lock;
} sound_blocking_msg_t;

/* Stack memory for the sound thread.
 * Note: currently the sound module is used as a singleton instance. */
static char thread_stack[THREAD_STACKSIZE_DEFAULT];

static beep_t startup_beeps[] = {
    { .f_target_hz = 1318, .duration_ms = 500 },
    { .f_target_hz = 659,  .duration_ms = 500 },
    { .f_target_hz = 988,  .duration_ms = 500 },
    { .f_target_hz = 880,  .duration_ms = 500 },
    { .f_target_hz = 1318, .duration_ms = 600 },
    { .f_target_hz = 988,  .duration_ms = 800 },
};

static beep_t downlink_rx_beeps[] = {
    { .f_target_hz = 988, .duration_ms = 400 },
    { .f_target_hz = 880, .duration_ms = 400 },
    { .f_target_hz = 784, .duration_ms = 500 },
};

static beep_t uplink_tx_beeps[] = {
    { .f_target_hz = 784, .duration_ms = 300 },
    { .f_target_hz = 880, .duration_ms = 300 },
    { .f_target_hz = 988, .duration_ms = 300 },
};

static beep_t ble_rx_beeps[] = {
    { .f_target_hz = 932, .duration_ms = 30 },
    { .f_target_hz = 831, .duration_ms = 30 },
};

static beep_t ble_tx_beeps[] = {
    { .f_target_hz = 831, .duration_ms =  400 },
    { .f_target_hz = 932, .duration_ms =  300 },
};

static beep_t tables_news_beeps[] = {
    { .f_target_hz = 784, .duration_ms =  50 },
    { .f_target_hz = 587, .duration_ms =  50 },
    { .f_target_hz = 932, .duration_ms =  50 },
};

static beep_t closeby_todo_beeps[] = {
    { .f_target_hz = 698, .duration_ms =  300 },
    { .f_target_hz = 880, .duration_ms =  300 },
    { .f_target_hz = 0,   .duration_ms =  500 },
    { .f_target_hz = 698, .duration_ms =  300 },
    { .f_target_hz = 880, .duration_ms =  300 },
};

beep_seq_t startup_beep_seq = BEEP_SEQ_INIT(startup_beeps);
beep_seq_t downlink_rx_seq = BEEP_SEQ_INIT(downlink_rx_beeps);
beep_seq_t uplink_tx_seq = BEEP_SEQ_INIT(uplink_tx_beeps);
beep_seq_t ble_rx_seq = BEEP_SEQ_INIT(ble_rx_beeps);
beep_seq_t ble_tx_seq = BEEP_SEQ_INIT(ble_tx_beeps);
beep_seq_t tables_news_seq = BEEP_SEQ_INIT(tables_news_beeps);
beep_seq_t closeby_todo_seq = BEEP_SEQ_INIT(closeby_todo_beeps);

static beep_seq_t* _beep_seqs_to_init[] = {
&startup_beep_seq,
&downlink_rx_seq,
&uplink_tx_seq,
&ble_rx_seq,
&ble_tx_seq,
&tables_news_seq,
&closeby_todo_seq,
};

int _match_pwm_res(sound_module_t *sm, beep_t *b)
{
    uint32_t f_target = b->f_target_hz;
    uint32_t best_diff = 0xFFFFFFFF;
    uint32_t best_res = 0;
    uint32_t best_freq = 0;

    for (uint32_t i = 0; i < MAX_PWM_RESOLUTION; i++) {
        uint32_t matched_f = pwm_init(sm->pwm_dev, PWM_LEFT, b->f_target_hz, i);
        if (matched_f != 0) {
            int abs_diff = f_target - matched_f;
            if (abs_diff < 0) {
                abs_diff *= -1;
            }
            if ((uint32_t)abs_diff < best_diff) {
                best_diff = abs_diff;
                best_res = i;
                best_freq = matched_f;
            }
        }
    }
    if (best_diff != 0xFFFFFFFF) {
        b->f_matched_hz = best_freq;
        b->pwm_res_val = best_res;
        return 0;
    } else {
        return -1;
    }
}

int sound_beep_seq_init(sound_module_t *sm, beep_seq_t *beep_seq)
{
    int res = 0;
    mutex_lock(&sm->pwm_lock);
    for (size_t i = 0; i < beep_seq->len; i++) {
        beep_t *beep = &beep_seq->beeps[i];
        /* f_target_hz == 0 defines a pause, dont try to match */
        if (beep->f_target_hz != 0 && _match_pwm_res(sm, beep) != 0) {
            res = -1;
            break;
        }
    }
    mutex_unlock(&sm->pwm_lock);
    return res;
}

void _sound_play(sound_module_t *sm, beep_seq_t *bs)
{
    pwm_poweron(sm->pwm_dev);
    for (size_t i = 0; i < bs->len; i++) {
        beep_t *beep = &bs->beeps[i];
        if (beep->f_target_hz != 0) {
            pwm_init(sm->pwm_dev, PWM_LEFT, beep->f_target_hz, beep->pwm_res_val);
            pwm_set(sm->pwm_dev, sm->pwm_channel, beep->pwm_res_val / 2);
            ztimer_sleep(ZTIMER_MSEC, beep->duration_ms);
            pwm_set(sm->pwm_dev, sm->pwm_channel, 0);
        } else {
            ztimer_sleep(ZTIMER_MSEC, beep->duration_ms);
        }
    }
    pwm_poweroff(sm->pwm_dev);
}

int sound_play(sound_module_t *sm, beep_seq_t *beep_seq)
{
    msg_t msg = { .content.ptr = beep_seq, .type = SOUND_ASYNC_MSG_TYPE };
    if (msg_send(&msg, sm->pid) == 1) {
        return 0;
    }
    return -1;
}

int sound_play_blocking(sound_module_t *sm, beep_seq_t *beep_seq)
{
    sound_blocking_msg_t blk_snd_msg = { .beep_seq = beep_seq,
                                         .playback_finished_lock = MUTEX_INIT, };
    msg_t msg = { .content.ptr = &blk_snd_msg, .type = SOUND_BLOCKING_MSG_TYPE };
    /* lock the mutex in order to block on the next call to lock */
    mutex_lock(&blk_snd_msg.playback_finished_lock);
    if (msg_send(&msg, sm->pid) == 1) {
        mutex_lock(&blk_snd_msg.playback_finished_lock);
        return 0;
    }
    return -1;
}

static void* sound_thread(void *arg) {
    sound_module_t *sm = (sound_module_t*)(arg);
    static msg_t tx_msg_queue[MSG_QUEUE_SIZE];
    /* initialize the message queue] */
    msg_init_queue(tx_msg_queue, MSG_QUEUE_SIZE);

    while(1){
        msg_t m;
        msg_receive(&m);

        if (m.type == SOUND_BLOCKING_MSG_TYPE) {
            sound_blocking_msg_t *sound_msg = (sound_blocking_msg_t*)m.content.ptr;
            _sound_play(sm, sound_msg->beep_seq);
            mutex_unlock(&sound_msg->playback_finished_lock);
        } else if (m.type == SOUND_ASYNC_MSG_TYPE) {
            beep_seq_t *bs = (beep_seq_t*)m.content.ptr;
            _sound_play(sm, bs);
        }
    }

    return NULL;
}

int sound_init(sound_module_t *sm, pwm_t pwm_dev, uint8_t channel)
{
    mutex_init(&sm->pwm_lock);
    sm->pwm_dev = pwm_dev;
    sm->pwm_channel = channel;

    for (unsigned i = 0; i < ARRAY_SIZE(_beep_seqs_to_init); i++) {
        int res = sound_beep_seq_init(sm, _beep_seqs_to_init[i]);
        if (res != 0) {
            return -1;
        }
    }

    sm->pid = thread_create(thread_stack, sizeof(thread_stack), THREAD_PRIORITY_MAIN - 1,
                            THREAD_CREATE_STACKTEST, sound_thread, sm, "sound");
    if (sm->pid > 0) {
        return 0;
    }
    return -2;
}
