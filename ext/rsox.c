#include <ruby.h>
#include <sox.h>

static VALUE RSox;

static VALUE RSoxFormat;
static VALUE RSoxEffect;
static VALUE RSoxEffectsChain;
static VALUE RSoxSignal;
static VALUE RSoxEncoding;
static VALUE RSoxBuffer;

static void rsox_destroy(void *instance) {
  int *instance_count;

  instance_count = (int*)instance;
  *instance_count -= 1;
  if (*instance_count == 0)
    sox_quit();
  rb_iv_set(RSox, "@instance_count", INT2NUM(*instance_count));
}

VALUE rsox_new(VALUE class) {
  static int rsox_instance_count = 0;
  VALUE instance_num;

  rsox_instance_count++;
  instance_num = Data_Wrap_Struct(RSox, 0, rsox_destroy, &rsox_instance_count);

  rb_iv_set(class, "@instance_count", INT2NUM(rsox_instance_count));
  rb_obj_call_init(instance_num, 0, 0);

  return instance_num;
}

VALUE rsox_count(VALUE class) {
  return rb_iv_get(class, "@count");
}

VALUE rsox_initialize(VALUE self) {
  int *instance_count;

  Data_Get_Struct(self, int, instance_count);
  if (*instance_count == 1)
    sox_init();

  return self;
}

VALUE rsox_set_bufsize(VALUE self, VALUE bufsize) {
  sox_globals.bufsiz = FIX2INT(bufsize);

  return Qtrue;
}

VALUE rsox_get_bufsize(VALUE self) {
  return INT2FIX(sox_globals.bufsiz);
}

VALUE rsox_format_init(VALUE self) {
  int i = sox_format_init();
  return INT2NUM(i);
}

VALUE rsox_format_quit(VALUE self) {
  sox_format_quit();
  return Qnil;
}

static void rsox_format_close(void *ptr) {
  sox_close(ptr);
}

VALUE rsox_open_read(int argc, VALUE *argv, VALUE self) {
  VALUE path, signal, encoding, filetype;
  sox_signalinfo_t   *c_signal   = NULL;
  sox_encodinginfo_t *c_encoding = NULL;
  sox_format_t       *c_format;

  rb_scan_args(argc, argv, "13", &path, &signal, &encoding, &filetype);

  if (!NIL_P(signal))   Data_Get_Struct(signal,   sox_signalinfo_t,   c_signal);
  if (!NIL_P(encoding)) Data_Get_Struct(encoding, sox_encodinginfo_t, c_encoding);

  c_format = sox_open_read(StringValuePtr(path), c_signal, c_encoding, filetype == Qnil ? NULL : StringValuePtr(filetype));

  return Data_Wrap_Struct(RSoxFormat, 0, rsox_format_close, c_format);
}

sox_bool rsox_overwrite_callback(const char *filename) {
  return sox_false;
}

VALUE rsox_open_write(int argc, VALUE *argv, VALUE self) {
  VALUE path, signal, encoding, filetype, oob;
  sox_signalinfo_t   *c_signal   = NULL;
  sox_encodinginfo_t *c_encoding = NULL;
  sox_oob_t          *c_oob      = NULL;
  sox_format_t       *c_format;

  rb_scan_args(argc, argv, "14", &path, &signal, &encoding, &filetype, &oob);

  if (signal   != Qnil) Data_Get_Struct(signal,   sox_signalinfo_t,   c_signal);
  if (encoding != Qnil) Data_Get_Struct(encoding, sox_encodinginfo_t, c_encoding);
  if (oob      != Qnil) Data_Get_Struct(oob,      sox_oob_t,          c_oob);

  c_format = sox_open_write(StringValuePtr(path),
    c_signal,
    c_encoding,
    filetype == Qnil ? NULL : StringValuePtr(filetype),
    c_oob,
    rb_block_given_p() ? rsox_overwrite_callback : NULL);

  return Data_Wrap_Struct(RSoxFormat, 0, 0, c_format);
}

VALUE rsoxformat_signal(VALUE self) {
  sox_format_t     *c_format;
  sox_signalinfo_t *c_info;

  Data_Get_Struct(self, sox_format_t, c_format);

  return Data_Wrap_Struct(RSoxSignal, 0, 0, &c_format->signal);
}

VALUE rsoxsignal_alloc(VALUE klass) {
	sox_signalinfo_t *c_signal = ALLOC(sox_signalinfo_t);
	memset(c_signal, 0, sizeof(sox_signalinfo_t));
	return Data_Wrap_Struct(klass, 0, free, c_signal);
}

VALUE rsoxsignal_rate(VALUE self) {
  sox_signalinfo_t *c;

  Data_Get_Struct(self, sox_signalinfo_t, c);

  return DBL2NUM(c->rate);
}

VALUE rsoxsignal_rate_set(VALUE self, VALUE rate) {
  sox_signalinfo_t *c;
  double val = NUM2DBL(rate);

  Data_Get_Struct(self, sox_signalinfo_t, c);
  c->rate = val;

  return rate;
}

VALUE rsoxsignal_channels(VALUE self) {
  sox_signalinfo_t     *c_signal;

  Data_Get_Struct(self, sox_signalinfo_t, c_signal);

  return UINT2NUM(c_signal->channels);
}

VALUE rsoxsignal_channels_set(VALUE self, VALUE channels) {
  sox_signalinfo_t     *c_signal;
  unsigned int val = NUM2UINT(channels);

  Data_Get_Struct(self, sox_signalinfo_t, c_signal);
  c_signal->channels = val;

  return channels;
}

VALUE rsoxsignal_bits(VALUE self) {
  sox_signalinfo_t     *c_signal;

  Data_Get_Struct(self, sox_signalinfo_t, c_signal);

  return UINT2NUM(c_signal->precision);
}

VALUE rsoxsignal_bits_set(VALUE self, VALUE bits) {
  sox_signalinfo_t     *c_signal;
  unsigned int val = NUM2UINT(bits);

  Data_Get_Struct(self, sox_signalinfo_t, c_signal);
  c_signal->precision = val;

  return bits;
}

VALUE rsoxsignal_length(VALUE self, VALUE bits) {
  sox_signalinfo_t     *c_signal;

  Data_Get_Struct(self, sox_signalinfo_t, c_signal);

  return UINT2NUM(c_signal->length);
}

VALUE rsoxsignal_length_set(VALUE self, VALUE length) {
  sox_signalinfo_t     *c_signal;
  size_t val = (size_t)NUM2UINT(length);

  Data_Get_Struct(self, sox_signalinfo_t, c_signal);
  c_signal->length = val;

  return length;
}

VALUE rsoxformat_encoding(VALUE self) {
  sox_format_t *c_format;

  Data_Get_Struct(self, sox_format_t, c_format);

  return Data_Wrap_Struct(RSoxEncoding, 0, 0, &c_format->encoding);
}

VALUE rsoxformat_filename(VALUE self) {
  sox_format_t *c_format;

  Data_Get_Struct(self, sox_format_t, c_format);

  return rb_str_new2(c_format->filename);
}

VALUE rsoxformat_read(VALUE self, VALUE buffer) {
  sox_format_t *c_format;
  sox_sample_t *c_buffer;

  Data_Get_Struct(self,   sox_format_t, c_format);
  Data_Get_Struct(rb_iv_get(buffer, "@buffer"), sox_sample_t, c_buffer);

  return INT2NUM(sox_read(c_format, c_buffer, NUM2INT(rb_iv_get(buffer, "@length"))));
}

VALUE rsoxformat_write(VALUE self, VALUE buffer, VALUE length) {
  sox_format_t *c_format;
  sox_sample_t *c_buffer;
  int write_len = NUM2INT(length == Qnil ? rb_iv_get(buffer, "@length") : length);

  Data_Get_Struct(self,   sox_format_t, c_format);
  Data_Get_Struct(rb_iv_get(buffer, "@buffer"), sox_sample_t, c_buffer);

  return INT2NUM(sox_write(c_format, c_buffer, write_len));
}

VALUE rsoxformat_seek(VALUE self, VALUE offset, VALUE whence){
  sox_format_t *c_format;

  Data_Get_Struct(self, sox_format_t, c_format);

  return INT2NUM(sox_seek(c_format, NUM2LONG(offset), NUM2INT(whence)));
}

typedef struct {/* rsox_block_with_id_t */
  VALUE block;
  ID func;
} rsox_block_with_id_t;

static int rsox_rubyblock_flow(sox_effect_t *effect, sox_sample_t const *ibuf, sox_sample_t *obuf UNUSED, size_t *isamp, size_t *osamp) {
  size_t i;
  rsox_block_with_id_t *param = (rsox_block_with_id_t *)effect->priv;
  VALUE buffer = Data_Wrap_Struct(RSoxBuffer, 0, 0, ibuf);
  rb_iv_set(buffer, "@length", INT2NUM(*isamp));

  if (*isamp > 0)
    rb_funcall(param->block, param->func, 1, buffer);

  *osamp = 0;

  return SOX_SUCCESS;
}

static sox_effect_handler_t const *rsox_rubyblock_handler(void) {
  static sox_effect_handler_t handler = {
    "block", NULL, SOX_EFF_MCHAN, NULL, NULL, rsox_rubyblock_flow, NULL, NULL, NULL, sizeof(rsox_block_with_id_t)
  };

  return &handler;
}

VALUE rsoxeffectschain_add(int argc, VALUE *argv, VALUE self) {
  sox_effects_chain_t *c_chain;
  sox_effect_t *c_effect;
  sox_format_t *c_input, *c_output, *c_tmp_format;
  VALUE name, options, tmp, input, output;
  sox_effect_handler_t const *c_handler;
  char *c_options[10], *c_name;
  int i, j, t;
  rsox_block_with_id_t *block_param;

  rb_scan_args(argc, argv, "1*", &name, &options);

  c_name = StringValuePtr(name);

  if (strncmp(c_name, "block", 5) == 0) {
    if (!rb_block_given_p())
      rb_raise(rb_eArgError, "no block given");

    c_handler = rsox_rubyblock_handler();
    c_effect = sox_create_effect(c_handler);

    block_param  = (rsox_block_with_id_t *)c_effect->priv;
    block_param->block = rb_block_proc();
    block_param->func  = rb_intern("call");
  } else {
    c_handler = sox_find_effect(StringValuePtr(name));
    if (c_handler == NULL)
      rb_raise(rb_eArgError, "no such effect: %s", StringValuePtr(name));
    c_effect = sox_create_effect(c_handler);

    for (i = j = 0; i < RARRAY_LEN(options); i++) {
      if (TYPE(RARRAY_PTR(options)[i]) == T_DATA) {
        Data_Get_Struct(RARRAY_PTR(options)[i], sox_format_t, c_tmp_format);
        c_options[j++] = (char *)c_tmp_format;
      } else {
        tmp = rb_check_string_type(RARRAY_PTR(options)[i]);
        c_options[j++] = NIL_P(tmp) ? NULL : RSTRING_PTR(tmp);
      }
    }

    i = sox_effect_options(c_effect, j, j > 0 ? c_options : NULL);
    if (i != SOX_SUCCESS)
      rb_raise(rb_eArgError, "wrong arguments (%d)", j);
  }

  Data_Get_Struct(self, sox_effects_chain_t, c_chain);
  Data_Get_Struct(rb_iv_get(self, "@input"),  sox_format_t, c_input);
  Data_Get_Struct(rb_iv_get(self, "@output"), sox_format_t, c_output);

  i = sox_add_effect(c_chain, c_effect, &c_input->signal, &c_output->signal);

  return INT2NUM(i);
}

VALUE rsoxbuffer_initialize(int argc, VALUE *argv, VALUE self) {
  sox_sample_t *buffer;
  VALUE length;
  int llen;

  rb_scan_args(argc, argv, "01", &length);
  llen = NIL_P(length) ? 2048 : NUM2INT(length);
  buffer = ALLOC_N(sox_sample_t, llen);

  rb_iv_set(self, "@buffer", Data_Wrap_Struct(RSoxBuffer, 0, free, buffer));
  rb_iv_set(self, "@length", INT2NUM(llen));

  return self;
}

VALUE rsoxbuffer_at(VALUE self, VALUE index) {
  sox_sample_t *c_buffer;

  if (index < rb_iv_get(self, "@length")) {
    Data_Get_Struct(self, sox_sample_t, c_buffer);
    return INT2NUM(c_buffer[NUM2INT(index)]);
  }

  return Qnil;
}

VALUE rsoxbuffer_length(VALUE self) {
  return rb_iv_get(self, "@length");
}

VALUE rsoxbuffer_buffer(VALUE self) {
  return rb_iv_get(self, "@buffer");
}

VALUE rsoxencoding_bps(VALUE self) {
  sox_encodinginfo_t *c_enc;

  Data_Get_Struct(self, sox_encodinginfo_t, c_enc);

  return UINT2NUM(c_enc->bits_per_sample);
}

VALUE rsoxencoding_bps_set(VALUE self, VALUE bps) {
  sox_encodinginfo_t *c_enc;
  unsigned int val = NUM2UINT(bps);

  Data_Get_Struct(self, sox_encodinginfo_t, c_enc);
  c_enc->bits_per_sample = val;

  return bps;
}

VALUE rsoxencoding_compression(VALUE self) {
  sox_encodinginfo_t *c_enc;

  Data_Get_Struct(self, sox_encodinginfo_t, c_enc);

  return DBL2NUM(c_enc->compression);
}

static void rsoxeffectschain_free(void *ptr) {
  sox_delete_effects_chain(ptr);
}

VALUE rsox_effectschain(VALUE self, VALUE input, VALUE output) {
  sox_format_t *c_input, *c_output;
  sox_effects_chain_t *c_chain;
  VALUE chain;

  Data_Get_Struct(input,  sox_format_t, c_input);
  Data_Get_Struct(output, sox_format_t, c_output);

  c_chain = sox_create_effects_chain(&c_input->encoding, &c_output->encoding);
  chain = Data_Wrap_Struct(RSoxEffectsChain, 0, rsoxeffectschain_free, c_chain);
  rb_iv_set(chain, "@input",  input);
  rb_iv_set(chain, "@output", output);

  rb_iv_set(self, "@chain", chain);

  return chain;
}

int rsoxeffectschain_flow_callback(sox_bool all_done, void *data) {
  return SOX_SUCCESS;
}

VALUE rsoxeffectschain_flow(int argc, VALUE *argv, VALUE self) {
  sox_effects_chain_t *c_chain;

  Data_Get_Struct(self, sox_effects_chain_t, c_chain);

  return INT2NUM(sox_flow_effects(c_chain, NULL, NULL));
}

void Init_rsox(void) {
  rb_define_global_const("SOX_SUCCESS", INT2NUM(0));
  rb_define_global_const("SOX_EOF",     INT2NUM(-1));

  RSox = rb_define_class("RSox", rb_cObject);
  rb_define_singleton_method(RSox, "new",   rsox_new,   0);
  rb_define_singleton_method(RSox, "count", rsox_count, 0);
  rb_define_method(RSox, "initialize",   rsox_initialize,   0);
  rb_define_method(RSox, "format_init",  rsox_format_init,  0);
  rb_define_method(RSox, "format_quit",  rsox_format_quit,  0);
  rb_define_method(RSox, "open_read",    rsox_open_read,   -1);
  rb_define_method(RSox, "open_write",   rsox_open_write,  -1);
  rb_define_method(RSox, "chain",        rsox_effectschain, 2);
  rb_define_method(RSox, "buffer_size",  rsox_get_bufsize,  0);
  rb_define_method(RSox, "buffer_size=", rsox_set_bufsize,  1);

  RSoxFormat = rb_define_class("RSoxFormat", rb_cObject);
  rb_define_method(RSoxFormat, "signal",   rsoxformat_signal,   0);
  rb_define_method(RSoxFormat, "encoding", rsoxformat_encoding, 0);
  rb_define_method(RSoxFormat, "filename", rsoxformat_filename, 0);
  rb_define_method(RSoxFormat, "read",     rsoxformat_read,     1);
  rb_define_method(RSoxFormat, "write",    rsoxformat_write,    2);
  rb_define_method(RSoxFormat, "seek",     rsoxformat_seek,     2);

  RSoxBuffer        = rb_define_class("RSoxBuffer", rb_cObject);
  rb_define_method(RSoxBuffer, "initialize", rsoxbuffer_initialize, -1);
  rb_define_method(RSoxBuffer, "length",     rsoxbuffer_length,      0);
  rb_define_method(RSoxBuffer, "size",       rsoxbuffer_length,      0);
  rb_define_method(RSoxBuffer, "buffer",     rsoxbuffer_buffer,      0);
  rb_define_method(RSoxBuffer, "[]",         rsoxbuffer_at,          1);
  rb_define_method(RSoxBuffer, "at",         rsoxbuffer_at,          1);

  RSoxSignal        = rb_define_class("RSoxSignal", rb_cObject);
  rb_define_alloc_func(RSoxSignal, rsoxsignal_alloc);
  rb_define_method(RSoxSignal, "rate",      rsoxsignal_rate,         0);
  rb_define_method(RSoxSignal, "channels",  rsoxsignal_channels,     0);
  rb_define_method(RSoxSignal, "bits",      rsoxsignal_bits,         0);
  rb_define_method(RSoxSignal, "length",    rsoxsignal_length,       0);
  rb_define_method(RSoxSignal, "rate=",     rsoxsignal_rate_set,     1);
  rb_define_method(RSoxSignal, "channels=", rsoxsignal_channels_set, 1);
  rb_define_method(RSoxSignal, "bits=",     rsoxsignal_bits_set,     1);
  rb_define_method(RSoxSignal, "length=",   rsoxsignal_length_set,   1);

  RSoxEncoding      = rb_define_class("RSoxEncoding", rb_cObject);
  rb_define_method(RSoxEncoding, "bps",  rsoxencoding_bps, 0);
  rb_define_method(RSoxEncoding, "bps=", rsoxencoding_bps, 1);
  rb_define_method(RSoxEncoding, "compression", rsoxencoding_compression, 0);

  RSoxEffect = rb_define_class("RSoxEffect", rb_cObject);

  RSoxEffectsChain  = rb_define_class("RSoxEffectsChain", rb_cObject);
  rb_define_method(RSoxEffectsChain, "add",  rsoxeffectschain_add,  -1);
  rb_define_method(RSoxEffectsChain, "flow", rsoxeffectschain_flow, -1);
}
