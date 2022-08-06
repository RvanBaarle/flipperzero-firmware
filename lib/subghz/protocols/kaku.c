#include "kaku.h"

#include "../blocks/const.h"
#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"
#include "../blocks/math.h"

#define TAG "SubGhzProtocolKAKU"

static const SubGhzBlockConst subghz_protocol_kaku_const = {
    .te_short = 270,
    .te_long = 1300,
    .te_delta = 200,
    .min_count_bit_for_found = 64,
};

struct SubGhzProtocolDecoderKAKU {
    SubGhzProtocolDecoderBase base;

    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;
};

struct SubGhzProtocolEncoderKAKU {
    SubGhzProtocolEncoderBase base;

    SubGhzProtocolBlockEncoder encoder;
    SubGhzBlockGeneric generic;
};

typedef enum {
    X10DecoderStepReset = 0,
    X10DecoderStepDecoderData,
} X10DecoderStep;

const SubGhzProtocolDecoder subghz_protocol_kaku_decoder = {
    .alloc = subghz_protocol_decoder_kaku_alloc,
    .free = subghz_protocol_decoder_kaku_free,

    .feed = subghz_protocol_decoder_kaku_feed,
    .reset = subghz_protocol_decoder_kaku_reset,

    .get_hash_data = subghz_protocol_decoder_kaku_get_hash_data,
    .serialize = subghz_protocol_decoder_kaku_serialize,
    .deserialize = subghz_protocol_decoder_kaku_deserialize,
    .get_string = subghz_protocol_decoder_kaku_get_string,
};

const SubGhzProtocolEncoder subghz_protocol_kaku_encoder = {
    .alloc = subghz_protocol_encoder_kaku_alloc,
    .free = subghz_protocol_encoder_kaku_free,

    .deserialize = subghz_protocol_encoder_kaku_deserialize,
    .stop = subghz_protocol_encoder_kaku_stop,
    .yield = subghz_protocol_encoder_kaku_yield,
};

const SubGhzProtocol subghz_protocol_kaku = {
    .name = SUBGHZ_PROTOCOL_KAKU_NAME,
    .type = SubGhzProtocolTypeStatic,
    .flag = SubGhzProtocolFlag_433 | SubGhzProtocolFlag_868 | SubGhzProtocolFlag_315 |
            SubGhzProtocolFlag_AM | SubGhzProtocolFlag_Decodable | SubGhzProtocolFlag_Load |
            SubGhzProtocolFlag_Save | SubGhzProtocolFlag_Send,

    .decoder = &subghz_protocol_kaku_decoder,
    .encoder = &subghz_protocol_kaku_encoder,
};

void* subghz_protocol_encoder_kaku_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolEncoderKAKU* instance = malloc(sizeof(SubGhzProtocolEncoderKAKU));

    instance->base.protocol = &subghz_protocol_kaku;
    instance->generic.protocol_name = instance->base.protocol->name;

    instance->encoder.repeat = 6;
    instance->encoder.size_upload = 148;
    instance->encoder.upload = malloc(instance->encoder.size_upload * sizeof(LevelDuration));
    instance->encoder.is_runing = false;
    return instance;
}

void subghz_protocol_encoder_kaku_free(void* context) {
    furi_assert(context);
    SubGhzProtocolEncoderKAKU* instance = context;
    free(instance->encoder.upload);
    free(instance);
}

static bool subghz_protocol_encoder_kaku_get_upload(SubGhzProtocolEncoderKAKU* instance) {
    furi_assert(instance);
    size_t index = 0;
    size_t size_upload = (instance->generic.data_count_bit * 4) + 4;
    if(size_upload > instance->encoder.size_upload) {
        FURI_LOG_E(TAG, "Size upload exceeds allocated encoder buffer.");
        return false;
    } else {
        instance->encoder.size_upload = size_upload;
    }

    bool has_dimdata = instance->generic.data_count_bit == 36;

    instance->encoder.upload[index++] =
        level_duration_make(true, subghz_protocol_kaku_const.te_short);
    instance->encoder.upload[index++] =
        level_duration_make(false, subghz_protocol_kaku_const.te_short * 10);

    for(uint i = instance->generic.data_count_bit; i > 0; i--) {
        if(i == 9 && has_dimdata) {
            instance->encoder.upload[index++] =
                level_duration_make(true, subghz_protocol_kaku_const.te_short);
            instance->encoder.upload[index++] =
                level_duration_make(false, subghz_protocol_kaku_const.te_short);
            instance->encoder.upload[index++] =
                level_duration_make(true, subghz_protocol_kaku_const.te_short);
            instance->encoder.upload[index++] =
                level_duration_make(false, subghz_protocol_kaku_const.te_short);
        } else if(bit_read(instance->generic.data, i - 1) != 0) {
            instance->encoder.upload[index++] =
                level_duration_make(true, subghz_protocol_kaku_const.te_short);
            instance->encoder.upload[index++] =
                level_duration_make(false, subghz_protocol_kaku_const.te_long);
            instance->encoder.upload[index++] =
                level_duration_make(true, subghz_protocol_kaku_const.te_short);
            instance->encoder.upload[index++] =
                level_duration_make(false, subghz_protocol_kaku_const.te_short);
        } else {
            instance->encoder.upload[index++] =
                level_duration_make(true, subghz_protocol_kaku_const.te_short);
            instance->encoder.upload[index++] =
                level_duration_make(false, subghz_protocol_kaku_const.te_short);
            instance->encoder.upload[index++] =
                level_duration_make(true, subghz_protocol_kaku_const.te_short);
            instance->encoder.upload[index++] =
                level_duration_make(false, subghz_protocol_kaku_const.te_long);
        }
    }
    instance->encoder.upload[index++] =
        level_duration_make(true, subghz_protocol_kaku_const.te_short);
    instance->encoder.upload[index++] = level_duration_make(false, 10000);
    return true;
}

bool subghz_protocol_encoder_kaku_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolEncoderKAKU* instance = context;
    bool res = false;
    do {
        if(!subghz_block_generic_deserialize(&instance->generic, flipper_format)) {
            FURI_LOG_E(TAG, "Deserialize error");
            break;
        }
        if(instance->generic.data_count_bit != 32 && instance->generic.data_count_bit != 36) {
            FURI_LOG_E(TAG, "Wrong number of bits in key");
            break;
        }

        subghz_protocol_encoder_kaku_get_upload(instance);
        instance->encoder.is_runing = true;

        res = true;
    } while(false);
    return res;
}

void subghz_protocol_encoder_kaku_stop(void* context) {
    SubGhzProtocolEncoderKAKU* instance = context;
    instance->encoder.is_runing = false;
}

LevelDuration subghz_protocol_encoder_kaku_yield(void* context) {
    SubGhzProtocolEncoderKAKU* instance = context;

    if(instance->encoder.repeat == 0 || !instance->encoder.is_runing) {
        instance->encoder.is_runing = false;
        return level_duration_reset();
    }

    LevelDuration ret = instance->encoder.upload[instance->encoder.front];

    if(++instance->encoder.front == instance->encoder.size_upload) {
        instance->encoder.repeat--;
        instance->encoder.front = 0;
    }

    return ret;
}

void* subghz_protocol_decoder_kaku_alloc(SubGhzEnvironment* environment) {
    UNUSED(environment);
    SubGhzProtocolDecoderKAKU* instance = malloc(sizeof(SubGhzProtocolDecoderKAKU));

    instance->base.protocol = &subghz_protocol_kaku;
    instance->generic.protocol_name = instance->base.protocol->name;
    return instance;
}

void subghz_protocol_decoder_kaku_free(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderKAKU* instance = context;
    free(instance);
}

void subghz_protocol_decoder_kaku_reset(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderKAKU* instance = context;
    instance->decoder.parser_step = X10DecoderStepReset;
}

void subghz_protocol_decoder_kaku_feed(void* context, bool level, uint32_t duration) {
    furi_assert(context);
    SubGhzProtocolDecoderKAKU* instance = context;

    switch(instance->decoder.parser_step) {
    case X10DecoderStepReset:
        if((!level) && (DURATION_DIFF(duration, subghz_protocol_kaku_const.te_short * 10) <
                        subghz_protocol_kaku_const.te_delta)) {
            //            FURI_LOG_D(TAG, "Reset to decode");
            instance->decoder.parser_step = X10DecoderStepDecoderData;
            instance->decoder.decode_data = 0;
            instance->decoder.decode_count_bit = 0;
        }
        break;
    case X10DecoderStepDecoderData:
        if(!level) {
            if(DURATION_DIFF(duration, subghz_protocol_kaku_const.te_short) <
               subghz_protocol_kaku_const.te_delta) {
                //                FURI_LOG_D(TAG, "Short low");
                instance->decoder.decode_data <<= 1;
                instance->decoder.decode_count_bit++;
            } else if(
                DURATION_DIFF(duration, subghz_protocol_kaku_const.te_long) <
                subghz_protocol_kaku_const.te_delta) {
                //                FURI_LOG_D(TAG, "Long low");
                instance->decoder.decode_data <<= 1;
                instance->decoder.decode_data |= 1;
                instance->decoder.decode_count_bit++;
            } else if(
                duration >= ((uint32_t)subghz_protocol_kaku_const.te_short * 10 +
                             subghz_protocol_kaku_const.te_delta)) {
                if(instance->decoder.decode_count_bit ==
                       subghz_protocol_kaku_const.min_count_bit_for_found ||
                   instance->decoder.decode_count_bit ==
                       subghz_protocol_kaku_const.min_count_bit_for_found + 8) {
                    FURI_LOG_D(TAG, "Signal detected");
                    bool is_valid = true;
                    instance->generic.data_count_bit = 0;
                    instance->generic.data = 0;
                    for(uint i = 0; i < instance->decoder.decode_count_bit; i += 2) {
                        uint8_t cur_bit = instance->decoder.decode_data >> 62;
                        instance->decoder.decode_data <<= 2;
                        if(cur_bit == 0x1) {
                            instance->generic.data <<= 1;
                            instance->generic.data_count_bit++;
                        } else if(cur_bit == 0x02) {
                            instance->generic.data <<= 1;
                            instance->generic.data |= 1;
                            instance->generic.data_count_bit++;
                        } else if(i == 54) {
                            instance->generic.data <<= 1;
                            instance->generic.data_count_bit++;
                        } else {
                            is_valid = false;
                            FURI_LOG_D(TAG, "Signal not valid");
                            break;
                        }
                    }
                    if(is_valid && instance->base.callback) {
                        FURI_LOG_D(TAG, "Signal valid");
                        instance->base.callback(&instance->base, instance->base.context);
                    }
                }
                //                FURI_LOG_D(TAG, "End of signal %d", duration);
                instance->decoder.parser_step = X10DecoderStepReset;
            } else {
                FURI_LOG_D(TAG, "Invalid pause %d", duration);
                instance->decoder.parser_step = X10DecoderStepReset;
            }
        } else {
            if(DURATION_DIFF(duration, subghz_protocol_kaku_const.te_short) <
               subghz_protocol_kaku_const.te_delta) {
                //                FURI_LOG_D(TAG, "Short high");
            } else {
                FURI_LOG_D(TAG, "Invalid pulse %d", duration);
                instance->decoder.parser_step = X10DecoderStepReset;
            }
        }
        if(instance->decoder.decode_count_bit > 0 && instance->decoder.decode_count_bit != 56 &&
           instance->decoder.decode_count_bit % 2 == 0) {
            unsigned int cur_bit = (instance->decoder.decode_data & 0x3);
            if(cur_bit == 0x03 || cur_bit == 0x00) {
                FURI_LOG_D(TAG, "Decode error");
                instance->decoder.parser_step = X10DecoderStepReset;
            }
        }
        break;
    }
}

uint8_t subghz_protocol_decoder_kaku_get_hash_data(void* context) {
    furi_assert(context);
    SubGhzProtocolDecoderKAKU* instance = context;
    return subghz_protocol_blocks_get_hash_data(
        &instance->decoder, (instance->decoder.decode_count_bit - 1 / 8) + 1);
}

void subghz_protocol_decoder_kaku_get_string(void* context, string_t output) {
    furi_assert(context);
    SubGhzProtocolDecoderKAKU* instance = context;

    uint32_t shortened;
    if(instance->generic.data_count_bit == 32) {
        shortened = instance->generic.data;
    } else {
        shortened = instance->generic.data >> 4;
    }
    uint32_t address = shortened >> 6;
    bool group_bit = (shortened >> 5) & 1;
    bool power_bit = (shortened >> 4) & 1;
    uint8_t unit = shortened & 0x0F;
    if(instance->generic.data_count_bit == 32) {
        string_cat_printf(
            output,
            "%s %dbit\r\n"
            "Address:0x%07lX\r\n"
            "Group:%s\r\n"
            "Power:%s\r\n"
            "Unit:0x%02X\r\n",
            instance->generic.protocol_name,
            instance->generic.data_count_bit,
            address,
            group_bit ? "yes" : "no",
            power_bit ? "on" : "off",
            unit);
    } else {
        uint8_t dim_level = instance->generic.data & 0x0f;
        string_cat_printf(
            output,
            "%s %dbit\r\n"
            "Address:0x%07lX\r\n"
            "Group:%s\r\n"
            "Power:%s\r\n"
            "Unit:0x%02X\r\n"
            "Brightness:% 2d/15\r\n",
            instance->generic.protocol_name,
            instance->generic.data_count_bit,
            address,
            group_bit ? "yes" : "no",
            power_bit ? "on" : "off",
            unit,
            dim_level);
    }
}

bool subghz_protocol_decoder_kaku_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzPresetDefinition* preset) {
    furi_assert(context);
    SubGhzProtocolDecoderKAKU* instance = context;
    return subghz_block_generic_serialize(&instance->generic, flipper_format, preset);
}

bool subghz_protocol_decoder_kaku_deserialize(void* context, FlipperFormat* flipper_format) {
    furi_assert(context);
    SubGhzProtocolDecoderKAKU* instance = context;
    bool ret = false;
    do {
        if(!subghz_block_generic_deserialize(&instance->generic, flipper_format)) {
            break;
        }
        if(instance->generic.data_count_bit != 32 && instance->generic.data_count_bit != 36) {
            FURI_LOG_E(TAG, "Wrong number of bits in key");
            break;
        }
        ret = true;
    } while(false);
    return ret;
}