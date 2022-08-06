#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_KAKU_NAME "KAKU"

typedef struct SubGhzProtocolDecoderKAKU SubGhzProtocolDecoderKAKU;
typedef struct SubGhzProtocolEncoderKAKU SubGhzProtocolEncoderKAKU;

extern const SubGhzProtocolDecoder subghz_protocol_kaku_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_kaku_encoder;
extern const SubGhzProtocol subghz_protocol_kaku;

/**
 * Allocate SubGhzProtocolEncoderKAKU.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderKAKU* pointer to a SubGhzProtocolEncoderKAKU instance
 */
void* subghz_protocol_encoder_kaku_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolEncoderKAKU.
 * @param context Pointer to a SubGhzProtocolEncoderKAKU instance
 */
void subghz_protocol_encoder_kaku_free(void* context);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderKAKU instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return true On success
 */
bool subghz_protocol_encoder_kaku_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Forced transmission stop.
 * @param context Pointer to a SubGhzProtocolEncoderKAKU instance
 */
void subghz_protocol_encoder_kaku_stop(void* context);

/**
 * Getting the level and duration of the upload to be loaded into DMA.
 * @param context Pointer to a SubGhzProtocolEncoderKAKU instance
 * @return LevelDuration
 */
LevelDuration subghz_protocol_encoder_kaku_yield(void* context);

/**
 * Allocate SubGhzProtocolDecoderKAKU.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderKAKU* pointer to a SubGhzProtocolDecoderKAKU instance
 */
void* subghz_protocol_decoder_kaku_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolDecoderKAKU.
 * @param context Pointer to a SubGhzProtocolDecoderKAKU instance
 */
void subghz_protocol_decoder_kaku_free(void* context);

/**
 * Reset decoder SubGhzProtocolDecoderKAKU.
 * @param context Pointer to a SubGhzProtocolDecoderKAKU instance
 */
void subghz_protocol_decoder_kaku_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderKAKU instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_kaku_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a SubGhzProtocolDecoderKAKU instance
 * @return hash Hash sum
 */
uint8_t subghz_protocol_decoder_kaku_get_hash_data(void* context);

/**
 * Serialize data SubGhzProtocolDecoderKAKU.
 * @param context Pointer to a SubGhzProtocolDecoderKAKU instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzPresetDefinition
 * @return true On success
 */
bool subghz_protocol_decoder_kaku_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzPresetDefinition* preset);

/**
 * Deserialize data SubGhzProtocolDecoderKAKU.
 * @param context Pointer to a SubGhzProtocolDecoderKAKU instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return true On success
 */
bool subghz_protocol_decoder_kaku_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderKAKU instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_kaku_get_string(void* context, string_t output);
