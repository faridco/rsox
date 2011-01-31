#include <ruby.h>
#include <sox.h>

static VALUE RSox;

static VALUE RSoxFormat;
static VALUE RSoxFormatHandler;
static VALUE RSoxEffect;
static VALUE RSoxEffectHandler;
static VALUE RSoxEffectsChain;
static VALUE RSoxSignal;
static VALUE RSoxEncoding;
static VALUE RSoxBuffer;

static void rsox_destroy(void *instance) {/*{{{*/
    int *instance_count;

    instance_count = (int*)instance;
    *instance_count--;
    if (*instance_count == 0)
        sox_quit();
    rb_iv_set(RSox, "@instance_count", INT2NUM(*instance_count));
}/*}}}*/

VALUE rsox_new(VALUE class) {/*{{{*/
    static int rsox_instance_count = 0;
    VALUE instance_num;

    rsox_instance_count++;
    instance_num = Data_Wrap_Struct(RSox, 0, rsox_destroy, &rsox_instance_count);

    rb_iv_set(class, "@instance_count", INT2NUM(rsox_instance_count));
    rb_obj_call_init(instance_num, 0, 0);

    return instance_num;
}/*}}}*/

VALUE rsox_count(VALUE class) {/*{{{*/
    return rb_iv_get(class, "@count");
}/*}}}*/

VALUE rsox_initialize(VALUE self) {/*{{{*/
    int *instance_count;

    Data_Get_Struct(self, int, instance_count);
    if (*instance_count == 1)
        sox_init();

    return self;
}/*}}}*/

VALUE rsox_format_init(VALUE self) {/*{{{*/
    int i = sox_format_init();
    return INT2NUM(i);
}/*}}}*/

VALUE rsox_format_quit(VALUE self) {/*{{{*/
    sox_format_quit();
    return Qnil;
}/*}}}*/

VALUE rsox_open_read(int argc, VALUE *argv, VALUE self) {/*{{{*/
    VALUE path, signal, encoding, filetype;
    sox_signalinfo_t   *c_signal   = NULL;
    sox_encodinginfo_t *c_encoding = NULL;
    sox_format_t       *c_format;

    rb_scan_args(argc, argv, "13", &path, &signal, &encoding, &filetype);

    if (!NIL_P(signal))   Data_Get_Struct(signal,   sox_signalinfo_t,   c_signal);
    if (!NIL_P(encoding)) Data_Get_Struct(encoding, sox_encodinginfo_t, c_encoding);

    c_format = sox_open_read(StringValuePtr(path), c_signal, c_encoding, filetype == Qnil ? NULL : StringValuePtr(filetype));

    return Data_Wrap_Struct(RSoxFormat, 0, 0, c_format);
}/*}}}*/

sox_bool rsox_overwrite_callback(const char *filename) {/*{{{*/
    return sox_false;
}/*}}}*/

VALUE rsox_open_write(int argc, VALUE *argv, VALUE self) {/*{{{*/
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
}/*}}}*/

VALUE rsoxformat_signal(VALUE self) {/*{{{*/
    sox_format_t     *c_format;
    sox_signalinfo_t *c_info;

    Data_Get_Struct(self, sox_format_t, c_format);

    return Data_Wrap_Struct(RSoxSignal, 0, 0, &c_format->signal);
}/*}}}*/

VALUE rsoxsignal_rate(VALUE self) {/*{{{*/
    sox_signalinfo_t *c;

    Data_Get_Struct(self, sox_signalinfo_t, c);

    return DBL2NUM(c->rate);
}/*}}}*/

VALUE rsoxsignal_rate_set(VALUE self, VALUE rate) {/*{{{*/
    sox_signalinfo_t *c;
    double val = NUM2DBL(rate);

    Data_Get_Struct(self, sox_signalinfo_t, c);
    c->rate = val;

    return rate;
}/*}}}*/

VALUE rsoxsignal_channels(VALUE self) {/*{{{*/
    sox_signalinfo_t     *c_signal;

    Data_Get_Struct(self, sox_signalinfo_t, c_signal);

    return UINT2NUM(c_signal->channels);
}/*}}}*/

VALUE rsoxsignal_channels_set(VALUE self, VALUE channels) {/*{{{*/
    sox_signalinfo_t     *c_signal;
    unsigned int val = NUM2UINT(channels);

    Data_Get_Struct(self, sox_signalinfo_t, c_signal);
    c_signal->channels = val;

    return channels;
}/*}}}*/

VALUE rsoxsignal_bits(VALUE self) {/*{{{*/
    sox_signalinfo_t     *c_signal;

    Data_Get_Struct(self, sox_signalinfo_t, c_signal);

    return UINT2NUM(c_signal->precision);
}/*}}}*/

VALUE rsoxsignal_bits_set(VALUE self, VALUE bits) {/*{{{*/
    sox_signalinfo_t     *c_signal;
    unsigned int val = NUM2UINT(bits);

    Data_Get_Struct(self, sox_signalinfo_t, c_signal);
    c_signal->precision = val;

    return bits;
}/*}}}*/

VALUE rsoxformat_encoding(VALUE self) {/*{{{*/
    sox_format_t *c_format;

    Data_Get_Struct(self, sox_format_t, c_format);

    return Data_Wrap_Struct(RSoxEncoding, 0, 0, &c_format->encoding);
}/*}}}*/

VALUE rsoxformat_filename(VALUE self) {/*{{{*/
    sox_format_t *c_format;

    Data_Get_Struct(self, sox_format_t, c_format);

    return rb_str_new2(c_format->filename);
}/*}}}*/

VALUE rsoxformat_read(VALUE self, VALUE buffer) { //, VALUE length) {/*{{{*/
    sox_format_t *c_format;
    sox_sample_t *c_buffer;

    Data_Get_Struct(self,   sox_format_t, c_format);
    Data_Get_Struct(rb_iv_get(buffer, "@buffer"), sox_sample_t, c_buffer);

    return INT2NUM(sox_read(c_format, c_buffer, NUM2INT(rb_iv_get(buffer, "@length"))));
}/*}}}*/

VALUE rsoxformat_write(VALUE self, VALUE buffer, VALUE length) {/*{{{*/
    sox_format_t *c_format;
    sox_sample_t *c_buffer;
    int write_len = NUM2INT(length == Qnil ? rb_iv_get(buffer, "@length") : length);

    Data_Get_Struct(self,   sox_format_t, c_format);
    Data_Get_Struct(rb_iv_get(buffer, "@buffer"), sox_sample_t, c_buffer);
    
    return INT2NUM(sox_write(c_format, c_buffer, write_len));
}/*}}}*/

VALUE rsoxformat_close(VALUE self) {/*{{{*/
    sox_format_t *c_format;

    Data_Get_Struct(self, sox_format_t, c_format);

    return INT2NUM(sox_close(c_format));
}/*}}}*/

VALUE rsoxformat_seek(VALUE self, VALUE offset, VALUE whence){/*{{{*/
    sox_format_t *c_format;

    Data_Get_Struct(self, sox_format_t, c_format);

    return INT2NUM(sox_seek(c_format, NUM2LONG(offset), NUM2INT(whence)));
}/*}}}*/

VALUE rsox_find_format(VALUE self, VALUE name, VALUE no_dev) {/*{{{*/
    sox_format_handler_t const *format_handler = sox_find_format(StringValuePtr(name), NUM2INT(no_dev));
    if (format_handler == NULL)
        return Qnil;
    else
        return Data_Wrap_Struct(RSoxFormatHandler, 0, 0, (sox_format_handler_t *)format_handler);
}/*}}}*/

VALUE rsox_create_effects_chain(VALUE self, VALUE in_enc, VALUE out_enc) {/*{{{*/
    sox_encodinginfo_t *c_in_enc, *c_out_enc;

    Data_Get_Struct(in_enc,  sox_encodinginfo_t, c_in_enc);
    Data_Get_Struct(out_enc, sox_encodinginfo_t, c_out_enc);

    return Data_Wrap_Struct(RSoxEffectsChain, 0, sox_delete_effects_chain, sox_create_effects_chain(c_in_enc, c_out_enc));
}/*}}}*/

VALUE rsox_delete_effects_chain(VALUE self, VALUE effects_chain) {/*{{{*/
    sox_effects_chain_t *c_effects_chain;

    Data_Get_Struct(effects_chain, sox_effects_chain_t, c_effects_chain);
    sox_delete_effects_chain(c_effects_chain);

    return Qnil;
}/*}}}*/

VALUE rsox_add_effect(VALUE self, VALUE chain, VALUE effect, VALUE in, VALUE out) {/*{{{*/
    sox_effects_chain_t *c_chain;
    sox_effect_t *c_effect;
    sox_signalinfo_t *c_in, *c_out;

    Data_Get_Struct(chain,  sox_effects_chain_t, c_chain);
    Data_Get_Struct(effect, sox_effect_t,        c_effect);
    Data_Get_Struct(in,     sox_signalinfo_t,    c_in);
    Data_Get_Struct(out,    sox_signalinfo_t,    c_out);

    return INT2NUM(sox_add_effect(c_chain, c_effect, c_in, c_out));
}/*}}}*/

VALUE rsox_find_effect(VALUE self, VALUE name) {/*{{{*/
    sox_effect_handler_t const *handler = sox_find_effect(StringValuePtr(name));
    if (handler == NULL)
        return Qnil;
    else
        return Data_Wrap_Struct(RSoxEffectHandler, 0, 0, (sox_effect_handler_t *)handler);
}/*}}}*/

VALUE rsoxeffects_options(int argc, VALUE *argv, VALUE self) {/*{{{*/
}/*}}}*/

VALUE rsoxbuffer_initialize(int argc, VALUE *argv, VALUE self) {/*{{{*/
    sox_sample_t *buffer;
    VALUE length;
    int llen;

    rb_scan_args(argc, argv, "01", &length);
    llen = NIL_P(length) ? 2048 : NUM2INT(length);
    buffer = ALLOC_N(sox_sample_t, llen);

    rb_iv_set(self, "@buffer", Data_Wrap_Struct(RSoxBuffer, 0, free, buffer));
    rb_iv_set(self, "@length", INT2NUM(llen));

    return self;
}/*}}}*/

VALUE rsoxbuffer_length(VALUE self) {/*{{{*/
    return rb_iv_get(self, "@length");
}/*}}}*/

VALUE rsoxbuffer_buffer(VALUE self) {/*{{{*/
    return rb_iv_get(self, "@buffer");
}/*}}}*/

VALUE rsoxencoding_bps(VALUE self) {/*{{{*/
  sox_encodinginfo_t *c_enc;

  Data_Get_Struct(self, sox_encodinginfo_t, c_enc);

  return UINT2NUM(c_enc->bits_per_sample);
}/*}}}*/

VALUE rsoxencoding_bps_set(VALUE self, VALUE bps) {/*{{{*/
  sox_encodinginfo_t *c_enc;
  unsigned int val = NUM2UINT(bps);

  Data_Get_Struct(self, sox_encodinginfo_t, c_enc);
  c_enc->bits_per_sample = val;

  return bps;
}/*}}}*/

void Init_rsox(void) {/*{{{*/
    RSox = rb_define_class("RSox", rb_cObject);
    rb_define_singleton_method(RSox, "new",   rsox_new,   0);
    rb_define_singleton_method(RSox, "count", rsox_count, 0);
    rb_define_method(RSox, "initialize",  rsox_initialize,  0);
    rb_define_method(RSox, "format_init", rsox_format_init, 0);
    rb_define_method(RSox, "format_quit", rsox_format_quit, 0);
    rb_define_method(RSox, "open_read",   rsox_open_read,   -1);
    rb_define_method(RSox, "open_write",  rsox_open_write,  -1);
    rb_define_method(RSox, "find_format", rsox_find_format, 2);
    rb_define_method(RSox, "add_effect",  rsox_add_effect,  4);
    rb_define_method(RSox, "create_effects_chain", rsox_create_effects_chain, 2);
    rb_define_method(RSox, "delete_effects_chain", rsox_delete_effects_chain, 1);

    RSoxFormat = rb_define_class("RSoxFormat", rb_cObject);
    rb_define_method(RSoxFormat, "signal",   rsoxformat_signal,   0);
    rb_define_method(RSoxFormat, "encoding", rsoxformat_encoding, 0);
    rb_define_method(RSoxFormat, "filename", rsoxformat_filename, 0);
    rb_define_method(RSoxFormat, "read",     rsoxformat_read,     1);
    rb_define_method(RSoxFormat, "write",    rsoxformat_write,    2);
    rb_define_method(RSoxFormat, "close",    rsoxformat_close,    0);
    rb_define_method(RSoxFormat, "seek",     rsoxformat_seek,     2);

    RSoxFormatHandler = rb_define_class("RSoxFormatHandler", rb_cObject);
    RSoxEffectsChain  = rb_define_class("RSoxEffectsChain", rb_cObject);
    RSoxEffect        = rb_define_class("RSoxEffect", rb_cObject);
    RSoxEffectHandler = rb_define_class("RSoxEffectHandler", rb_cObject);

    RSoxBuffer        = rb_define_class("RSoxBuffer", rb_cObject);
    rb_define_method(RSoxBuffer, "initialize", rsoxbuffer_initialize, -1);
    rb_define_method(RSoxBuffer, "length",     rsoxbuffer_length,      0);
    rb_define_method(RSoxBuffer, "buffer",     rsoxbuffer_buffer,      0);

    RSoxSignal        = rb_define_class("RSoxSignal", rb_cObject);
    rb_define_method(RSoxSignal, "rate",      rsoxsignal_rate,         0);
    rb_define_method(RSoxSignal, "channels",  rsoxsignal_channels,     0);
    rb_define_method(RSoxSignal, "bits",      rsoxsignal_bits,         0);
    rb_define_method(RSoxSignal, "rate=",     rsoxsignal_rate_set,     1);
    rb_define_method(RSoxSignal, "channels=", rsoxsignal_channels_set, 1);
    rb_define_method(RSoxSignal, "bits=",     rsoxsignal_bits_set,     1);

    RSoxEncoding      = rb_define_class("RSoxEncoding", rb_cObject);
    rb_define_method(RSoxEncoding, "bps",  rsoxencoding_bps, 0);
    rb_define_method(RSoxEncoding, "bps=", rsoxencoding_bps, 1);
}/*}}}*/
