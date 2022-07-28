#include <furi.h>
#include <furi_hal.h>
#include <cli/cli.h>
#include "notification/notification.h"
#include "notification/notification_messages.h"

static void dcf77_cli(Cli* cli, string_t args, void* context);

extern "C" void dcf77_on_system_start() {
#ifdef SRV_CLI
    Cli* cli = static_cast<Cli*>(furi_record_open("cli"));
    cli_add_command(cli, "dcf77", CliCommandFlagDefault, dcf77_cli, nullptr);
    furi_record_close("cli");
#else
    UNUSED(dcf77_cli);
#endif
}

struct CompContext {
    volatile bool level;
    volatile bool flag;
};

static const NotificationSequence highSeq = {
    &message_blue_255,
    &message_do_not_reset,
    nullptr,
};

static const NotificationSequence lowSeq = {
    &message_blue_0,
    &message_do_not_reset,
    nullptr,
};

static void comparator_trigger_callback(bool level, void* context) {
    auto* data = static_cast<CompContext*>(context);
    data->level = level;
    data->flag = true;
}

static void dcf77_cli(Cli* cli, string_t args, void* context) {
    UNUSED(args);
    UNUSED(context);
    printf("Press CTRL+C to stop...\r\n");
    auto* notif = static_cast<NotificationApp*>(furi_record_open(RECORD_NOTIFICATION));

    CompContext comparatorContext = {
        false,
        false,
    };

    furi_hal_rfid_pins_read();
    furi_hal_rfid_tim_read(77500, 0.5);
    furi_hal_rfid_tim_read_start();
    furi_hal_rfid_comp_set_callback(comparator_trigger_callback, (void*)&comparatorContext);
    furi_hal_rfid_comp_start();
    bool last_level = false;
    uint32_t last_time = DWT->CYCCNT;

    while(!cli_cmd_interrupt_received(cli)) {
        if(comparatorContext.flag) {
            if(comparatorContext.level != last_level) {
                last_level = comparatorContext.level;
                uint32_t cur_time = DWT->CYCCNT;
                printf("Trigger: %s %lu\r\n", last_level ? "HIGH" : "LOW", cur_time - last_time);
                notification_message(notif, last_level ? &highSeq : &lowSeq);
                last_time = cur_time;
            }
            comparatorContext.flag = false;
        }
    }

    furi_hal_rfid_pins_reset();
    furi_hal_rfid_tim_read_stop();
    furi_hal_rfid_tim_reset();
    furi_hal_rfid_comp_stop();
    furi_hal_rfid_comp_set_callback(nullptr, nullptr);
    furi_hal_light_set(LightGreen, 0);
    notification_message(notif, &sequence_reset_rgb);
    furi_record_close(RECORD_NOTIFICATION);
}