//
// Created by scaled
//
// Based on image-source.c from OBS Studio
//

#include <obs-module.h>
#include <graphics/image-file.h>
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

	char *file1;
	char *file2;
	bool persistent;
	bool linear_alpha;
	bool active;

	gs_image_file3_t if31;
	gs_image_file3_t if32;
	
	obs_source_t *audio_source;
	
	bool loud;
	float threshold;
	float smoothness;
	float average;
};

/*int MAX(int a, int b) {
	return a > b ? a : b;
}*/

#define MIN(a,b) ((a)<(b) ? (a):(b))
#define MAX(a,b) ((a)>(b) ? (a):(b))

static const char *image_reaction_source_get_name(void *unused)
{
	UNUSED_PARAMETER(unused);
	return obs_module_text("Image Reaction Source");
}

static void image_reaction_source_load(struct image_reaction_source *context)
{
	for (int i = 0; i <=1; i++) {
		char *file = i == 0 ? context->file1 : context->file2;
		gs_image_file3_t *if3 = i == 0 ? &context->if31 : &context->if32;

		obs_enter_graphics();
		gs_image_file3_free(if3);
		obs_leave_graphics();
		
		if (file && *file) {
			debug("loading texture '%s'", file);
			gs_image_file3_init(if3, file,
					    context->linear_alpha
						    ? GS_IMAGE_ALPHA_PREMULTIPLY_SRGB
						    : GS_IMAGE_ALPHA_PREMULTIPLY);

			obs_enter_graphics();
			gs_image_file3_init_texture(if3);
			obs_leave_graphics();

			if (!if3->image2.image.loaded)
				warn("failed to load texture '%s'", file);
		}
	}
	context->loud = false;
}

static void image_reaction_source_unload(struct image_reaction_source *context)
{
	obs_enter_graphics();
	gs_image_file3_free(&context->if31);
	gs_image_file3_free(&context->if32);
	obs_leave_graphics();
}

static void audio_capture(void *param, obs_source_t *src, const struct audio_data *data, bool muted)
{
	struct image_reaction_source *context = param;
	
	context->loud = false;
	
	uint32_t samplesCount = data->frames;
	float* samples = (float*)data->data[0];
	
	float averageLocal = 0.0f;
	
	for (uint32_t i = 0; i < samplesCount; i++) {
		averageLocal += fabs(samples[i]) / samplesCount;
	}
	
	context->average += context->smoothness * (averageLocal - context->average);
	if (context->average > context->threshold) {
		context->loud = true;
	}
	//printf("%f\n", average);
}

static void image_reaction_source_update(void *data, obs_data_t *settings)
{
	struct image_reaction_source *context = data;
	const char *file1 = obs_data_get_string(settings, "file1");
	const char *file2 = obs_data_get_string(settings, "file2");
	const bool unload = obs_data_get_bool(settings, "unload");
	const bool linear_alpha = obs_data_get_bool(settings, "linear_alpha");
	const double threshold = obs_data_get_double(settings, "threshold");
	const double smoothness = obs_data_get_double(settings, "smoothness");

	if (context->file1)
		bfree(context->file1);
	context->file1 = bstrdup(file1);
	
	if (context->file2)
		bfree(context->file2);
	context->file2 = bstrdup(file2);
	
	context->persistent = !unload;
	context->linear_alpha = linear_alpha;
	context->threshold = db_to_mul(threshold);
	context->smoothness = pow(0.1, smoothness);
	//printf("%f\n",context->smoothness); 
	//info("%f, %d\n", threshold, context->threshold);

	/* Load the image if the source is persistent or showing */
	if (context->persistent || obs_source_showing(context->source))
		image_reaction_source_load(data);
	else
		image_reaction_source_unload(data);
	
	const char* new_name = obs_data_get_string(settings, "audio_source");
	const char* old_name = obs_source_get_name(context->audio_source);

	if (old_name == NULL || (new_name != "" && strcmp(new_name, old_name) != 0)) {
		info("name changed");
		
		obs_source_t *capture = obs_get_source_by_name(new_name);
		//obs_weak_source_t *weak_capture = capture ? obs_source_get_weak_source(capture) : NULL;
		
		if (capture) {
			obs_source_remove_audio_capture_callback(context->audio_source, audio_capture, context);
			
			context->audio_source = capture;
			
			info("Added audio capture to '%s'", obs_source_get_name(capture));
			obs_source_add_audio_capture_callback(capture, audio_capture, context);
			//obs_weak_source_release(weak_capture);
			obs_source_release(capture);
		}
	}
}

static void image_reaction_source_defaults(obs_data_t *settings)
{
	obs_data_set_default_bool(settings, "unload", false);
	obs_data_set_default_bool(settings, "linear_alpha", false);
        obs_data_set_default_string(settings, "audio_source", "");
        obs_data_set_default_double(settings, "threshold", -40.0f);
        obs_data_set_default_double(settings, "smoothness", 1.0f);
}

static void image_reaction_source_show(void *data)
{
	struct image_reaction_source *context = data;

	if (!context->persistent)
		image_reaction_source_load(context);
}

static void image_reaction_source_hide(void *data)
{
	struct image_reaction_source *context = data;

	if (!context->persistent)
		image_reaction_source_unload(context);
}

static void *image_reaction_source_create(obs_data_t *settings, obs_source_t *source)
{
	struct image_reaction_source *context = bzalloc(sizeof(struct image_reaction_source));
	context->source = source;

	image_reaction_source_update(context, settings);
	return context;
}

static void image_reaction_source_destroy(void *data)
{
	struct image_reaction_source *context = data;

	image_reaction_source_unload(context);

	if (context->file1)
		bfree(context->file1);

	if (context->file2)
		bfree(context->file2);
	
	if (context->audio_source) {
		//obs_source_t *source = obs_weak_source_get_source(context->audio_source);
		//if (source) {
			info("Removed audio capture from '%s'", obs_source_get_name(context->audio_source));
			obs_source_remove_audio_capture_callback(context->audio_source, audio_capture, context);
			//obs_source_release(source);
		//}
		//obs_weak_source_release(context->audio_source);
	}
	
	bfree(context);
}

static uint32_t image_reaction_source_getwidth(void *data)
{
	struct image_reaction_source *context = data;
	return MAX(context->if31.image2.image.cx, context->if32.image2.image.cx);
}

static uint32_t image_reaction_source_getheight(void *data)
{
	struct image_reaction_source *context = data;
	return MAX(context->if31.image2.image.cy, context->if32.image2.image.cy);
}

static void image_reaction_source_render(void *data, gs_effect_t *effect)
{
	struct image_reaction_source *context = data;

	const bool previous = gs_framebuffer_srgb_enabled();
	gs_enable_framebuffer_srgb(true);
	gs_blend_state_push();
	gs_blend_function(GS_BLEND_ONE, GS_BLEND_INVSRCALPHA);
	
	gs_image_file3_t *if3 = context->loud ? &context->if32 : &context->if31;
	if (if3->image2.image.texture)
	{
		gs_eparam_t *const param = gs_effect_get_param_by_name(effect, "image");
		gs_effect_set_texture_srgb(param, if3->image2.image.texture);

		gs_draw_sprite(if3->image2.image.texture, 0,
			       if3->image2.image.cx,
			       if3->image2.image.cy);
	}
	//context->loud = false;

	gs_blend_state_pop();

	gs_enable_framebuffer_srgb(previous);
}

static const char *image_filter =
	"All formats (*.bmp *.tga *.png *.jpeg *.jpg *.gif *.psd *.webp);;"
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

static bool source_changed(obs_properties_t *props, obs_property_t * prop, obs_data_t *data)
{
    obs_data_get_string(data, "audio_source");
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
				OBS_PATH_FILE, image_filter, path.array);
	obs_properties_add_path(props, "file2", obs_module_text("Reaction2"),
				OBS_PATH_FILE, image_filter, path.array);
	dstr_free(&path);
	
	obs_properties_add_bool(props, "unload",
				obs_module_text("UnloadWhenNotShowing"));
	obs_properties_add_bool(props, "linear_alpha",
				obs_module_text("LinearAlpha"));
	obs_property_t* sources_list = obs_properties_add_list(props, "audio_source",
				obs_module_text("AudioSource"), OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
				
	obs_property_t *p = obs_properties_add_float_slider(props, "threshold",
		obs_module_text("Threshold"), -60.0, 0.0, 0.1);
		obs_property_float_set_suffix(p, " dB");
	
	obs_properties_add_float_slider(props, "smoothness",
		obs_module_text("Smoothness"), 0.0, 5.0, 0.1);
	
	//obs_property_set_modified_callback(src, source_changed);
	obs_enum_sources(add_source, sources_list);
	
	return props;
}

uint64_t image_reaction_source_get_memory_usage(void *data)
{
	struct image_reaction_source *s = data;
	return s->if31.image2.mem_usage + s->if32.image2.mem_usage;
}

static void missing_file_callback(void *src, const char *new_path, void *data)
{
	struct image_reaction_source *s = src;

	obs_source_t *source = s->source;
	obs_data_t *settings = obs_source_get_settings(source);
	obs_data_set_string(settings, "file", new_path);
	obs_source_update(source, settings);
	obs_data_release(settings);

	UNUSED_PARAMETER(data);
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
	.get_properties = image_reaction_source_properties,
	.icon_type = OBS_ICON_TYPE_IMAGE,
};

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("image-source", "en-US")
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
