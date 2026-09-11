#include <libheif/heif.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

static void check(heif_error error) {
  if (error.code != heif_error_Ok) {
    std::cerr << error.message << '\n';
    std::exit(1);
  }
}

static void roundtrip(heif_compression_format format, const char* decoder) {
  heif_context* context = heif_context_alloc();
  heif_encoder* encoder = nullptr;
  check(heif_context_get_encoder_for_format(context, format, &encoder));
  check(heif_encoder_set_lossy_quality(encoder, 90));
  heif_image* input = nullptr;
  check(heif_image_create(64, 64, heif_colorspace_YCbCr, heif_chroma_420, &input));
  const heif_channel channels[] = {heif_channel_Y, heif_channel_Cb, heif_channel_Cr};
  for (int p = 0; p < 3; ++p) {
    int size = p == 0 ? 64 : 32;
    check(heif_image_add_plane(input, channels[p], size, size, 8));
    int stride = 0;
    auto* plane = heif_image_get_plane(input, channels[p], &stride);
    if (!plane) std::exit(2);
    for (int y = 0; y < size; ++y) std::memset(plane + y * stride, p == 0 ? 16 : 128, size);
  }
  heif_image_handle* encoded = nullptr;
  check(heif_context_encode_image(context, input, encoder, nullptr, &encoded));
  const char* filename = "native-roundtrip.heif";
  check(heif_context_write_to_file(context, filename));
  heif_context* reader = heif_context_alloc();
  check(heif_context_read_from_file(reader, filename, nullptr));
  heif_image_handle* handle = nullptr;
  check(heif_context_get_primary_image_handle(reader, &handle));
  if (heif_image_handle_get_width(handle) != 64 || heif_image_handle_get_height(handle) != 64) std::exit(3);
  auto* options = heif_decoding_options_alloc();
  options->decoder_id = decoder;
  heif_image* output = nullptr;
  check(heif_decode_image(handle, &output, heif_colorspace_YCbCr, heif_chroma_420, options));
  for (int p = 0; p < 3; ++p) {
    int stride = 0, size = p == 0 ? 64 : 32;
    const auto* plane = heif_image_get_plane_readonly(output, channels[p], &stride);
    if (!plane) std::exit(4);
    for (int y = 0; y < size; ++y)
      for (int x = 0; x < size; ++x)
        if (std::abs(int(plane[y * stride + x]) - (p == 0 ? 16 : 128)) > 2) std::exit(5);
  }
  std::cout << "libheif " << heif_encoder_get_name(encoder) << " -> " << decoder << " pixel round trip passed\n";
  heif_image_release(output);
  heif_decoding_options_free(options);
  heif_image_handle_release(handle);
  heif_context_free(reader);
  heif_image_handle_release(encoded);
  heif_image_release(input);
  heif_encoder_release(encoder);
  heif_context_free(context);
  if (std::remove(filename) != 0) std::exit(6);
}

int main(int argc, char** argv) {
  const char* version = heif_get_version();
  if (!version || !version[0]) return 1;
  std::cout << "libheif " << version << '\n';
  if (argc == 1) return 0;
  check(heif_init(nullptr));
  if (!heif_have_decoder_for_format(heif_compression_HEVC)) return 7;
  roundtrip(heif_compression_AV1, "aom");
  roundtrip(heif_compression_AV1, "dav1d");
  bool gpl = std::strcmp(argv[1], "gpl") == 0;
  if (bool(heif_have_encoder_for_format(heif_compression_HEVC)) != gpl) return 8;
  if (gpl) roundtrip(heif_compression_HEVC, "libde265");
  heif_deinit();
  return 0;
}
