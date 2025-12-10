#ifndef SOUND_H
#define SOUND_H

#include <stdint.h>
#include "mutex.h"
#include "periph/pwm.h"

/* Max value for pwm resolution setting tried when matching the pwm config to a given frequency. */
#ifndef MAX_PWM_RESOLUTION
#define MAX_PWM_RESOLUTION (32768)
#endif

typedef struct {
  uint16_t f_target_hz;
  uint16_t f_matched_hz;
  uint16_t duration_ms;
  uint16_t pwm_res_val;
} beep_t;

typedef struct {
    beep_t *beeps;
    size_t len;
} beep_seq_t;

typedef struct {
    kernel_pid_t pid;
    mutex_t pwm_lock;
    pwm_t pwm_dev;
    uint8_t pwm_channel;
} sound_module_t;

/* Some predefined sounds to be used by other modules.
 * Are automatically inizialized by @ref sound_init(). */
extern beep_seq_t startup_beep_seq;
extern beep_seq_t downlink_rx_seq;
extern beep_seq_t uplink_tx_seq;
extern beep_seq_t ble_rx_seq;
extern beep_seq_t ble_tx_seq;
extern beep_seq_t tables_news_seq;
extern beep_seq_t closeby_todo_seq;

/**
 * @brief   Initialize the sound module
 *
 * @param[in] sm           where to store the sound module context
 * @param[in] pwm_dev      the pwm device to use
 * @param[in] channel      channel connected to a buzzer / speaker
 *
 * @retval                 0 on success
 * @retval                 negative value on error
 */
int sound_init(sound_module_t *sm, pwm_t pwm_dev, uint8_t channel);

/**
 * @brief   Initialize a sequence of beep sounds
 *
 * This determines applicable pwm settings to generate the beep.
 *
 * @pre The beep sequence ust be set to a valid length and
 *
 * @param[in] sm           the sound module instance
 * @param[in] beep_seq     the beep sequence to initialize
 *
 * @retval                 0 on success
 * @retval                 negative value on error
 */
int sound_beep_seq_init(sound_module_t *sm, beep_seq_t *beep_seq);

/**
 * @brief   Play a sound defined as beep_sequence_t
 *
 * @param[in] sm           the sound module instance
 * @param[in] beep_seq     the beep sequence to play
 *
 * @retval                 0 on success
 * @retval                 negative value on error
 */
int sound_play(sound_module_t *sm, beep_seq_t *beep_seq);

/**
 * @brief   Play a sound defined as beep_sequence_t (blocking until playback finished)
 *
 * @param[in] sm           the sound module instance
 * @param[in] beep_seq     the beep sequence to play
 *
 * @retval                 0 on success
 * @retval                 negative value on error
 */
int sound_play_blocking(sound_module_t *sm, beep_seq_t *beep_seq);
#endif // SOUND_H
