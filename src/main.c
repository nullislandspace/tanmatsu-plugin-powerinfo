// SPDX-License-Identifier: MIT
// Tanmatsu Plugin - Power Info Status Widget
//
// This plugin displays the current system voltage in the status bar.

#include "tanmatsu_plugin.h"
#include "asp/power.h"
#include "asp/err.h"
#include "pax_gfx.h"

// External font used by launcher status bar (exported to plugins)
extern const pax_font_t chakrapetchmedium;

// Plugin state
static int widget_id = -1;

// Format voltage as "XXXXmV" without using snprintf (not exported to plugins)
static int format_voltage_mv(char* buf, uint16_t voltage_mv) {
    int i = 0;
    int v = voltage_mv;

    // Handle 5 digits max (up to 65535mV)
    if (v >= 10000) {
        buf[i++] = '0' + (v / 10000);
        v %= 10000;
        buf[i++] = '0' + (v / 1000);
        v %= 1000;
    } else if (v >= 1000) {
        buf[i++] = '0' + (v / 1000);
        v %= 1000;
    }
    buf[i++] = '0' + (v / 100);
    v %= 100;
    buf[i++] = '0' + (v / 10);
    buf[i++] = '0' + (v % 10);

    buf[i++] = 'm';
    buf[i++] = 'V';
    buf[i] = '\0';

    return i;  // Length of string
}

// Status widget callback - displays system voltage
// Draws to the left of x_right, returns width used
static int status_widget_callback(pax_buf_t* buffer, int x_right, int y, int height, void* user_data) {
    (void)user_data;

    uint16_t voltage_mv;
    if (asp_power_get_system_voltage(&voltage_mv) != ASP_OK) {
        return 0;  // No width if failed
    }

    // Format voltage as "XXXXmV"
    char text[8];
    int text_len = format_voltage_mv(text, voltage_mv);

    // Draw text to the left of x_right
    // Use font size 16 (matches status bar), estimate ~10 pixels per character
    int text_width = text_len * 10;
    int text_x = x_right - text_width - 2;
    int text_y = y + (height - 16) / 2;

    pax_draw_text(buffer, 0xFF340132, &chakrapetchmedium, 16, text_x, text_y, text);
    return text_width + 4;  // Width used including margins
}

// Plugin metadata
static const plugin_info_t plugin_info = {
    .name = "Power Info",
    .slug = "at.cavac.powerinfo",
    .version = "1.0.0",
    .author = "Rene Schickbauer",
    .description = "Displays system voltage in status bar",
    .api_version = TANMATSU_PLUGIN_API_VERSION,
    .type = PLUGIN_TYPE_SERVICE,
    .flags = 0,
};

// Return plugin info
static const plugin_info_t* get_info(void) {
    return &plugin_info;
}

// Plugin initialization
static int plugin_init(plugin_context_t* ctx) {
    asp_log_info("powerinfo", "Power Info plugin initializing...");

    // Register status widget to show voltage
    widget_id = asp_plugin_status_widget_register(ctx, status_widget_callback, NULL);
    if (widget_id < 0) {
        asp_log_error("powerinfo", "Failed to register status widget");
        return -1;
    }

    asp_log_info("powerinfo", "Power Info plugin initialized, widget_id=%d", widget_id);
    return 0;
}

// Plugin cleanup
static void plugin_cleanup(plugin_context_t* ctx) {
    (void)ctx;

    // Unregister status widget
    if (widget_id >= 0) {
        asp_plugin_status_widget_unregister(widget_id);
        widget_id = -1;
    }

    asp_log_info("powerinfo", "Power Info plugin cleaned up");
}

// Service main loop - just keeps plugin alive
// The widget is redrawn automatically when the screen refreshes
static void plugin_service_run(plugin_context_t* ctx) {
    asp_log_info("powerinfo", "Power Info service starting...");

    while (!asp_plugin_should_stop(ctx)) {
        // Just sleep - the widget callback is called when screen refreshes
        asp_plugin_delay_ms(100);
    }

    asp_log_info("powerinfo", "Power Info service stopped");
}

// Plugin entry point structure
static const plugin_entry_t entry = {
    .get_info = get_info,
    .init = plugin_init,
    .cleanup = plugin_cleanup,
    .menu_render = NULL,
    .menu_select = NULL,
    .service_run = plugin_service_run,
    .hook_event = NULL,
};

// Register this plugin with the host
TANMATSU_PLUGIN_REGISTER(entry);
