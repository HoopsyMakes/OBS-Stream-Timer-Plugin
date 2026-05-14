
#include <ctime>
#include <obs/obs-data.h>
#include <obs/obs-module.h>
#include <obs/obs-properties.h>
#include <string>
#include <time.h>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("countdown-source", "en-US")

struct custom_source {
  obs_source_t *source;
  obs_source_t *browser;
};

/* --------------------------------------------------------- */
/* Source name */
/* --------------------------------------------------------- */

static const char *custom_source_get_name(void *unused) {
  UNUSED_PARAMETER(unused);
  return obs_module_text("CustomBrowserWrapper");
}

/* --------------------------------------------------------- */
/* Width / Height proxy */
/* --------------------------------------------------------- */

static uint32_t custom_source_get_width(void *data) {
  auto *ctx = (custom_source *)data;

  if (!ctx->browser)
    return 1920;

  return obs_source_get_width(ctx->browser);
}

static uint32_t custom_source_get_height(void *data) {
  auto *ctx = (custom_source *)data;

  if (!ctx->browser)
    return 1080;

  return obs_source_get_height(ctx->browser);
}

/* --------------------------------------------------------- */
/* Render proxy */
/* --------------------------------------------------------- */

static void custom_source_render(void *data, gs_effect_t *effect) {
  UNUSED_PARAMETER(effect);

  auto *ctx = (custom_source *)data;

  if (!ctx->browser)
    return;

  obs_source_video_render(ctx->browser);
}

/* --------------------------------------------------------- */
/* Create */
/* --------------------------------------------------------- */

static void *custom_source_create(obs_data_t *settings, obs_source_t *source) {
  auto *ctx = new custom_source;

  ctx->source = source;
  ctx->browser = nullptr;

  /* ------------------------------------ */
  /* Create internal browser source */
  /* ------------------------------------ */

  obs_data_t *browser_settings = obs_data_create();

  obs_data_set_string(browser_settings, "url", "https://example.com");

  obs_data_set_bool(browser_settings, "shutdown", true);

  obs_data_set_int(browser_settings, "width", 1920);

  obs_data_set_int(browser_settings, "height", 1080);

  ctx->browser = obs_source_create_private("browser_source", "internal-browser",
                                           browser_settings);
  if (!ctx->browser) {
    blog(LOG_ERROR, "[custom-browser] Browser source creation FAILED");
  } else {
    obs_source_inc_showing(ctx->browser);
    obs_source_inc_active(ctx->browser);
    blog(LOG_INFO, "[custom-browser] Browser source created");
  }
  if (ctx->browser) {
    obs_source_add_active_child(source, ctx->browser);
  }

  obs_data_release(browser_settings);

  /* Apply current settings immediately */

  if (settings)
    obs_source_update(source, settings);

  return ctx;
}

/* --------------------------------------------------------- */
/* Destroy */
/* --------------------------------------------------------- */

static void custom_source_destroy(void *data) {
  auto *ctx = (custom_source *)data;

  if (ctx->browser) {
    obs_source_dec_showing(ctx->browser);
    obs_source_dec_active(ctx->browser);
    obs_source_remove_active_child(ctx->source, ctx->browser);
    obs_source_release(ctx->browser);
  }
  delete ctx;
}

/* --------------------------------------------------------- */
/* Update */
/* --------------------------------------------------------- */
// static void custom_source_tick(void *data, float seconds)
// {
//     UNUSED_PARAMETER(data);
//     UNUSED_PARAMETER(seconds);
// }
static void custom_source_update(void *data, obs_data_t *settings) {
  auto *ctx = (custom_source *)data;

  if (!ctx->browser)
    return;

  /* ------------------------------------ */
  /* Read custom property */
  /* ------------------------------------ */

  const int h = obs_data_get_int(settings, "hours");
  const int m = obs_data_get_int(settings, "minutes");
  const bool type = obs_data_get_bool(settings, "type");
  const int width = obs_data_get_int(settings, "width");
  const int height = obs_data_get_int(settings, "height");
  /* ------------------------------------ */
  /* Create date long */
  /* ------------------------------------ */
  time_t time = std::time(0);
  if (!type) {
    time *= 1000;
    time += 1000 * (h * 3600 + m * 60);
  } else {
    struct tm *tm_info = localtime(&time);
    tm_info->tm_hour = h;
    tm_info->tm_min = m;
    tm_info->tm_sec = 0;
    time = mktime(tm_info);
    time *= 1000;
  }

  /* ------------------------------------ */
  /* Generate final URL */
  /* ------------------------------------ */

  std::string url =
      "https://hoopsymakes.github.io/OBS-Stream-Timer#/countdown/" +
      std::to_string(time);

  blog(LOG_INFO, "[custom-browser] URL = %s", url.c_str());

  /* ------------------------------------ */
  /* Update internal browser source */
  /* ------------------------------------ */

  obs_data_t *browser_settings = obs_source_get_settings(ctx->browser);

  obs_data_set_string(browser_settings, "url", url.c_str());

  const char *css = obs_data_get_string(settings, "css");
  obs_data_set_string(browser_settings, "css", css);
  obs_data_set_int(browser_settings, "width", width);
  obs_data_set_int(browser_settings, "height", height);

  obs_source_update(ctx->browser, browser_settings);

  obs_data_release(browser_settings);
}

/* --------------------------------------------------------- */
/* Properties */
/* --------------------------------------------------------- */
static obs_properties_t *custom_source_properties(void *data) {
  UNUSED_PARAMETER(data);

  /* Get normal Browser Source properties */

  obs_properties_t *props = obs_get_source_properties("browser_source");

  if (!props)
    return obs_properties_create();

  /* Remove default URL field */
  obs_properties_remove_by_name(props, "is_local_file");
  obs_properties_remove_by_name(props, "local_file");
  obs_properties_remove_by_name(props, "fps_custom");
  obs_properties_remove_by_name(props, "fps");
  obs_properties_remove_by_name(props, "webpage_control_level");
  obs_properties_remove_by_name(props, "refreshnocache");
  obs_properties_remove_by_name(props, "url");
  obs_properties_remove_by_name(props, "css");
  obs_properties_remove_by_name(props, "width");
  obs_properties_remove_by_name(props, "height");
  obs_properties_remove_by_name(props, "fps");
  obs_properties_remove_by_name(props, "reroute_audio");
  obs_properties_remove_by_name(props, "shutdown");
  obs_properties_remove_by_name(props, "restart_when_active");
  obs_properties_remove_by_name(props, "hwaccel");

  /* Add custom field */

  obs_properties_add_int(props, "hours", "Hours", 0, 999, 1);
  obs_properties_add_int(props, "minutes", "Minutes", 0, 59, 5);
  obs_properties_add_bool(props, "type",
                          "True 24h clock, False means time from now");
  obs_properties_add_text(props, "css", "Custom CSS", OBS_TEXT_MULTILINE);
  obs_properties_add_int(props, "width", "Width", 1, 100000, 1);
  obs_properties_add_int(props, "height", "Height", 1, 100000, 1);

  return props;
}

/* --------------------------------------------------------- */
/* Defaults */
/* --------------------------------------------------------- */

static void custom_source_defaults(obs_data_t *settings) {
  obs_data_set_default_string(settings, "hours", "");
  obs_data_set_default_string(settings, "minutes", "");
  obs_data_set_default_bool(settings, "type", false);
  obs_data_set_default_string(settings, "css", "");
  obs_data_set_default_string(settings, "width", "250");
  obs_data_set_default_string(settings, "height", "250");
}

/* --------------------------------------------------------- */
/* Source registration */
/* --------------------------------------------------------- */

static struct obs_source_info custom_source_info = {};

bool obs_module_load(void) {
  custom_source_info.id = "custom_browser_wrapper";

  custom_source_info.type = OBS_SOURCE_TYPE_INPUT;

  custom_source_info.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_CUSTOM_DRAW;

  custom_source_info.get_name = custom_source_get_name;

  custom_source_info.create = custom_source_create;

  custom_source_info.destroy = custom_source_destroy;

  custom_source_info.update = custom_source_update;

  custom_source_info.get_properties = custom_source_properties;

  custom_source_info.get_defaults = custom_source_defaults;

  custom_source_info.video_render = custom_source_render;

  custom_source_info.get_width = custom_source_get_width;

  custom_source_info.get_height = custom_source_get_height;

  obs_register_source(&custom_source_info);

  blog(LOG_INFO, "[custom-browser] loaded");

  return true;
}
