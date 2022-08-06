#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_X10_NAME "X10"

typedef struct SubGhzProtocolDecoderX10 SubGhzProtocolDecoderX10;
typedef struct SubGhzProtocolEncoderX10 SubGhzProtocolEncoderX10;

extern const SubGhzProtocolDecoder subghz_protocol_x10_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_x10_encoder;
extern const SubGhzProtocol subghz_protocol_x10;

/**
 * Allocate SubGhzProtocolEncoderX10.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderX10* pointer to a SubGhzProtocolEncoderX10 instance
 */
void* subghz_protocol_encoder_x10_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolEncoderX10.
 * @param context Pointer to a SubGhzProtocolEncoderX10 instance
 */
void subghz_protocol_encoder_x10_free(void* context);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderX10 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return true On success
 */
bool subghz_protocol_encoder_x10_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Forced transmission stop.
 * @param context Pointer to a SubGhzProtocolEncoderX10 instance
 */
void subghz_protocol_encoder_x10_stop(void* context);

/**
 * Getting the level and duration of the upload to be loaded into DMA.
 * @param context Pointer to a SubGhzProtocolEncoderX10 instance
 * @return LevelDuration
 */
LevelDuration subghz_protocol_encoder_x10_yield(void* context);

/**
 * Allocate SubGhzProtocolDecoderX10.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderX10* pointer to a SubGhzProtocolDecoderX10 instance
 */
void* subghz_protocol_decoder_x10_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolDecoderX10.
 * @param context Pointer to a SubGhzProtocolDecoderX10 instance
 */
void subghz_protocol_decoder_x10_free(void* context);

/**
 * Reset decoder SubGhzProtocolDecoderX10.
 * @param context Pointer to a SubGhzProtocolDecoderX10 instance
 */
void subghz_protocol_decoder_x10_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderX10 instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_x10_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a SubGhzProtocolDecoderX10 instance
 * @return hash Hash sum
 */
uint8_t subghz_protocol_decoder_x10_get_hash_data(void* context);

/**
 * Serialize data SubGhzProtocolDecoderX10.
 * @param context Pointer to a SubGhzProtocolDecoderX10 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzPresetDefinition
 * @return true On success
 */
bool subghz_protocol_decoder_x10_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzPresetDefinition* preset);

/**
 * Deserialize data SubGhzProtocolDecoderX10.
 * @param context Pointer to a SubGhzProtocolDecoderX10 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return true On success
 */
bool subghz_protocol_decoder_x10_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderX10 instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_x10_get_string(void* context, string_t output);
