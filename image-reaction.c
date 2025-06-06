//
// Created by scaled
//
// Based on image-source.c from OBS Studio: https://github.com/obsproject/obs-studio
// Also included some code from Spectralizer plugin: https://github.com/univrsal/spectralizer
//

#include <obs-module.h>
#include <obs-source.h>
#include <util/platform.h>
#include <util/dstr.h>
#include <sys/stat.h>
#include <media-io/audio-math.h>

#define blog(log_level, format, ...)                    \
	blog(log_level, "[image_reaction_source: '%s'] " format, \
	     obs_source_get_name(context->source), ##__VA_ARGS__)

#define debug(format, ...) blog(LOG_DEBUG, format, ##__VA_ARGS__)
#define info(format, ...) blog(LOG_INFO, format, ##__VA_ARGS__)
#define warn(format, ...) blog(LOG_WARNING, format, ##__VA_ARGS__)

struct image_reaction_source {
	obs_source_t *source;
	char source_name[255];

	char *file1;
	char *file2;

	obs_source_t *media1;
	obs_source_t *media2;
	
	obs_weak_source_t *audio_source;
	
	bool loud;
	float threshold;
	float smoothness;
	float average;
	
	uint64_t capture_check_time;
	
	bool animReset1;
	bool animReset2;
	bool loudOld;
	bool animResetTrigger;
};

/*int MAX(int a, int b) {
	return a > b ? a : b;
}*/

#define MIN(a,b) ((a)<(b) ? (a):(b))
#define MAX(a,b) ((a)>(b) ? (a):(b))

static const char *image_reaction_source_get_name(void *unused)
{
	UNUSED_PARAMETER(unused);
	return obs_module_text("MediaReactionSource");
}

static void audio_capture(void *param, obs_source_t *src, const struct audio_data *data, bool muted)
{
	struct image_reaction_source *context = param;
	
	if (muted) {
		context->average = 0;
	}
	else
	{
		uint32_t samplesCount = data->frames;
		float* samples = (float*)data->data[0];
		
		float averageLocal = 0.0f;
		
		for (uint32_t i = 0; i < samplesCount; i++) {
			averageLocal += fabs(samples[i]) / samplesCount;
		}
		
		context->average += context->smoothness * (averageLocal - context->average);
	}
	
	context->loudOld = context->loud;
	context->loud = context->average > context->threshold;
	
	if (context->loud != context->loudOld)
		context->animResetTrigger = true;
}

static void create_or_update_media(struct image_reaction_source *ctx, obs_source_t **media, const char *file, int id) {
    if (file && *file) {
        obs_data_t *settings = obs_data_create();
        obs_data_set_string(settings, "local_file", file);
        obs_data_set_bool(settings, "looping", true);

        if (*media) {
            obs_source_update(*media, settings);
        } else {
            char name[64];
            snprintf(name, sizeof(name), "media_reaction_%d_%p", id, ctx);
            *media = obs_source_create("ffmpeg_source", name, settings, NULL);
            if (obs_source_showing(ctx->source)) {
                 obs_source_add_active_child(ctx->source, *media);
            }
        }
        obs_data_release(settings);

    } else if (*media) {
        obs_source_remove_active_child(ctx->source, *media);
        obs_source_release(*media);
        *media = NULL;
    }
}

static void image_reaction_source_update(void *data, obs_data_t *settings)
{
	struct image_reaction_source *context = data;
	const char *file1 = obs_data_get_string(settings, "file1");
	const char *file2 = obs_data_get_string(settings, "file2");
	const bool anim_reset_1 = obs_data_get_bool(settings, "anim_reset_1");
	const bool anim_reset_2 = obs_data_get_bool(settings, "anim_reset_2");
	const double threshold = obs_data_get_double(settings, "threshold");
	const double smoothness = obs_data_get_double(settings, "smoothness");

	if (context->file1)
		bfree(context->file1);
	context->file1 = bstrdup(file1);
	
	if (context->file2)
		bfree(context->file2);
	context->file2 = bstrdup(file2);
	
	context->animReset1 = anim_reset_1;
	context->animReset2 = anim_reset_2;
	
	context->threshold = db_to_mul(threshold);
	context->smoothness = pow(0.1, smoothness);

	create_or_update_media(context, &context->media1, file1, 1);
	create_or_update_media(context, &context->media2, file2, 2);
	
	const char* cfg_source_name = obs_data_get_string(settings, "audio_source");
	
	obs_weak_source_t *old = NULL;
	
	if (cfg_source_name[0] == '\0') {
		if (context->audio_source) {
			old = context->audio_source;
			context->audio_source = NULL;
		}
		context->source_name[0] = '\0';
	}
	else {
		if (context->source_name[0] == '\0' || strcmp(context->source_name, cfg_source_name) != 0) {
			if (context->audio_source) {
				old = context->audio_source;
				context->audio_source = NULL;
			}
			strcpy(context->source_name, cfg_source_name);
			context->capture_check_time = os_gettime_ns() - 3000000000;
		}
	}

	if (old) {
		obs_source_t *old_source = obs_weak_source_get_source(old);
		if (old_source) {
			info("Removed audio capture from '%s'", obs_source_get_name(old_source));
			obs_source_remove_audio_capture_callback(old_source, audio_capture, context);
			obs_source_release(old_source);
		}
		obs_weak_source_release(old);
	}
}

static void image_reaction_source_defaults(obs_data_t *settings)
{
        obs_data_set_default_string(settings, "audio_source", "");
        obs_data_set_default_double(settings, "threshold", -40.0f);
        obs_data_set_default_double(settings, "smoothness", 1.0f);
}

static void image_reaction_source_show(void *data)
{
	struct image_reaction_source *context = data;
	if (context->media1)
		obs_source_add_active_child(context->source, context->media1);
	if (context->media2)
		obs_source_add_active_child(context->source, context->media2);
}

static void image_reaction_source_hide(void *data)
{
	struct image_reaction_source *context = data;
	if (context->media1)
		obs_source_remove_active_child(context->source, context->media1);
	if (context->media2)
		obs_source_remove_active_child(context->source, context->media2);
}

static void *image_reaction_source_create(obs_data_t *settings, obs_source_t *source)
{
	struct image_reaction_source *context = bzalloc(sizeof(struct image_reaction_source));
	context->source = source;
	context->media1 = NULL;
	context->media2 = NULL;
	
	context->source_name[0] = '\0';
	context->loud = false;

	image_reaction_source_update(context, settings);
	return context;
}

static void image_reaction_source_destroy(void *data)
{
	struct image_reaction_source *context = data;

	if (context->file1)
		bfree(context->file1);

	if (context->file2)
		bfree(context->file2);
	
	obs_source_release(context->media1);
	obs_source_release(context->media2);

	if (context->audio_source) {
		obs_source_t *source = obs_weak_source_get_source(context->audio_source);
		if (source) {
			info("Removed audio capture from '%s'", obs_source_get_name(source));
			obs_source_remove_audio_capture_callback(source, audio_capture, context);
			obs_source_release(source);
		}
		obs_weak_source_release(context->audio_source);
	}
	
	bfree(context);
}

static uint32_t image_reaction_source_getwidth(void *data)
{
	struct image_reaction_source *context = data;
	uint32_t w1 = context->media1 ? obs_source_get_width(context->media1) : 0;
	uint32_t w2 = context->media2 ? obs_source_get_width(context->media2) : 0;
	return MAX(w1, w2);
}

static uint32_t image_reaction_source_getheight(void *data)
{
	struct image_reaction_source *context = data;
	uint32_t h1 = context->media1 ? obs_source_get_height(context->media1) : 0;
	uint32_t h2 = context->media2 ? obs_source_get_height(context->media2) : 0;
	return MAX(h1, h2);
}

static void image_reaction_source_render(void *data, gs_effect_t *effect)
{
	struct image_reaction_source *context = data;

	obs_source_t *active_media = context->loud ? context->media2 : context->media1;

	if (active_media) {
		obs_source_video_render(active_media);
	}

	UNUSED_PARAMETER(effect);
}

static void image_reaction_tick(void *data, float seconds)
{
	struct image_reaction_source *context = data;
	UNUSED_PARAMETER(seconds);

	// Update / refresh audio capturing
	char* new_name = NULL;
	if (context->source_name[0] != '\0' && !context->audio_source) {
		uint64_t t = os_gettime_ns();

		if (t - context->capture_check_time > 3000000000) {
			new_name = context->source_name;
			context->capture_check_time = t;
		}
	}

	if (new_name != NULL) {
		obs_source_t *capture = obs_get_source_by_name(new_name);
		obs_weak_source_t *weak_capture = capture ? obs_source_get_weak_source(capture) : NULL;

		if (context->source_name[0] != '\0' && new_name == context->source_name) {
			context->audio_source = weak_capture;
			weak_capture = NULL;
		}

		if (capture) {
			info("Added audio capture to '%s'", obs_source_get_name(capture));
			obs_source_add_audio_capture_callback(capture, audio_capture, context);
			obs_weak_source_release(weak_capture);
			obs_source_release(capture);
		}
	}
	
	if (context->animResetTrigger) {
		if(context->loud) { // switched to loud
			if (context->animReset2 && context->media2) {
				obs_data_t *settings = obs_source_get_settings(context->media2);
				obs_source_update(context->media2, settings);
				obs_data_release(settings);
			}
		} else { // switched to not-loud
			if (context->animReset1 && context->media1) {
				obs_data_t *settings = obs_source_get_settings(context->media1);
				obs_source_update(context->media1, settings);
				obs_data_release(settings);
			}
		}
	}
	context->animResetTrigger = false;
}

static const char *media_filter =
	"Video files (*.mp4 *.ts *.mov *.flv *.mkv *.avi *.gif *.webm);;"
	"All formats (*.bmp *.tga *.png *.jpeg *.jpg *.gif *.psd *.webp *.mp4 *.ts *.mov *.flv *.mkv *.avi *.webm);;"
	"BMP Files (*.bmp);;"
	"Targa Files (*.tga);;"
	"PNG Files (*.png);;"
	"JPEG Files (*.jpeg *.jpg);;"
	"GIF Files (*.gif);;"
	"PSD Files (*.psd);;"
	"WebP Files (*.webp);;"
	"All Files (*.*)";

static bool add_source(void* param, obs_source_t* src)
{
    obs_property_t *list = param;
    
    uint32_t caps = obs_source_get_output_flags(src);

    if ((caps & OBS_SOURCE_AUDIO) == 0)
        return true;
    const char *name = obs_source_get_name(src);
    obs_property_list_add_string(list, name, name);
    return true;
}

static obs_properties_t *image_reaction_source_properties(void *data)
{
	struct image_reaction_source *s = data;
	struct dstr path = {0};

	obs_properties_t *props = obs_properties_create();

	if (s && s->file1 && *s->file1) {
		const char *slash;

		dstr_copy(&path, s->file1);
		dstr_replace(&path, "\\", "/");
		slash = strrchr(path.array, '/');
		if (slash)
			dstr_resize(&path, slash - path.array + 1);
	}

	obs_properties_add_path(props, "file1", obs_module_text("Reaction1"),
				OBS_PATH_FILE, media_filter, path.array);
	obs_properties_add_bool(props, "anim_reset_1",
				obs_module_text("AnimReset1"));
	obs_properties_add_path(props, "file2", obs_module_text("Reaction2"),
				OBS_PATH_FILE, media_filter, path.array);
	obs_properties_add_bool(props, "anim_reset_2",
				obs_module_text("AnimReset2"));
	dstr_free(&path);
	
	obs_property_t* sources_list = obs_properties_add_list(props, "audio_source",
				obs_module_text("AudioSource"), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
	obs_property_list_add_string(sources_list, "", "");
				
	obs_property_t *p = obs_properties_add_float_slider(props, "threshold",
		obs_module_text("Threshold"), -60.0, 0.0, 0.1);
		obs_property_float_set_suffix(p, " dB");
	
	obs_properties_add_float_slider(props, "smoothness",
		obs_module_text("Smoothness"), 0.0, 5.0, 0.1);
	
	obs_enum_sources(add_source, sources_list);
	
	return props;
}

static struct obs_source_info image_reaction_source_info = {
	.id = "image_reaction_source",
	.type = OBS_SOURCE_TYPE_INPUT,
	.output_flags = OBS_SOURCE_VIDEO | OBS_SOURCE_SRGB,
	.get_name = image_reaction_source_get_name,
	.create = image_reaction_source_create,
	.destroy = image_reaction_source_destroy,
	.update = image_reaction_source_update,
	.get_defaults = image_reaction_source_defaults,
	.show = image_reaction_source_show,
	.hide = image_reaction_source_hide,
	.get_width = image_reaction_source_getwidth,
	.get_height = image_reaction_source_getheight,
	.video_render = image_reaction_source_render,
	.video_tick = image_reaction_tick,
	.get_properties = image_reaction_source_properties,
	.icon_type = OBS_ICON_TYPE_IMAGE,
};

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("image-reaction", "en-US")
MODULE_EXPORT const char *obs_module_description(void)
{
	return "Image reaction source";
}

extern struct obs_source_info slideshow_info;

bool obs_module_load(void)
{
	obs_register_source(&image_reaction_source_info);
	return true;
}
