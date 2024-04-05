#include <dragonruby.h>
#include <features.h>
#include <mruby.h>
#include <stdint.h>
#include <zlib.h>

static inline uint64_t host_to_big(uint64_t a) {
  volatile uint64_t chk = 1;
  if (*(uint8_t *)&chk == 1) {
    return __builtin_bswap64(a);
  } else {
    return a;
  }
}

#if defined(__linux__) || defined(__APPLE__)
#define DL(NAME) NAME
#else
#define DL(NAME) drb->NAME
#endif

struct drb_api_t *drb;

mrb_value rz_compress_string(mrb_state *mrb, mrb_value self) {
  struct RString *rstr = RSTRING(self);
  const char *str = RSTR_PTR(rstr);
  const size_t len = (size_t)RSTR_LEN(rstr);

  mrb_int level = Z_DEFAULT_COMPRESSION;
  DL(mrb_get_args)(mrb, "|i", &level);

  if (level < -1 || level > 9) {
    DL(mrb_raisef)
    (mrb, E_ARGUMENT_ERROR, "expected `level` to be in the range 0..9, got %d",
     level);
  }

  size_t res_len = compressBound(len) + sizeof(size_t);
  mrb_value res_str = DL(mrb_str_new_capa)(mrb, res_len);

  int ec = compress2((Bytef *)RSTRING_PTR(res_str) + sizeof(size_t), &res_len,
                     (Bytef *)str, len, level);

  if (ec != Z_OK) {
    DL(mrb_raise)(mrb, mrb->eStandardError_class, "couldn't compress string");
  }

  struct RString *res_rstr = RSTRING(res_str);
  _Static_assert(sizeof(size_t) == 8);
  *(size_t *)RSTR_PTR(res_rstr) = host_to_big(len);
  RSTR_SET_LEN(res_rstr, res_len + sizeof(size_t));

  return res_str;
}

mrb_value rz_decompress_string(mrb_state *mrb, mrb_value self) {
  struct RString *rstr = RSTRING(self);
  const char *str = RSTR_PTR(rstr);
  const size_t len = (size_t)RSTR_LEN(rstr);

  if (len < 8) {
    DL(mrb_raise)
    (mrb, mrb->eStandardError_class,
     "tried to decompress a string that definitely does not have a "
     "length tag");
  }

  size_t size = host_to_big(*(size_t *)str);
  size_t src_size = size;
  size_t expected_size = size;
  str += sizeof(size_t);
  mrb_value res_str = DL(mrb_str_new_capa)(mrb, size);

  struct RString *res_rstr = RSTRING(res_str);

  int ec =
      uncompress2((Bytef *)RSTR_PTR(res_rstr), &size, (Bytef *)str, &src_size);

  if (ec != Z_OK) {
    if (ec == Z_DATA_ERROR || ec == Z_BUF_ERROR) {
      DL(mrb_raise)
      (mrb, mrb->eStandardError_class,
       "failed to decompress string, corrupted data");
    } else {
      DL(mrb_raise)
      (mrb, mrb->eStandardError_class,
       "failed to decompress string, memory alloc failiure");
    }
  }

  if (size != expected_size) {
    DL(mrb_raisef)
    (mrb, mrb_exc_get_id(mrb, mrb_intern_lit(mrb, "IndexError")),
     "size mismatch (data corrupted) (expected %d, got %d)", expected_size,
     size);
  }

  RSTR_SET_LEN(res_rstr, size);

  return res_str;
}

void drb_register_c_extensions_with_api(mrb_state *mrb, struct drb_api_t *api) {
  drb = api;
  struct RClass *str = mrb->string_class;
  DL(mrb_define_method_id)
  (mrb, str, mrb_intern_lit(mrb, "compress"), rz_compress_string,
   MRB_ARGS_OPT(1));
  DL(mrb_define_method_id)
  (mrb, str, mrb_intern_lit(mrb, "decompress"), rz_decompress_string,
   MRB_ARGS_NONE());
}
