#include <nanoarrow/nanoarrow.hpp>
#include <nanobind/nanobind.h>

namespace nb = nanobind;

auto CheckData(nb::object obj) {
  const auto maybe_capsule = nb::getattr(obj, "__arrow_c_stream__")();
  const auto capsule = nb::cast<nb::capsule>(maybe_capsule);

  auto c_stream = static_cast<ArrowArrayStream *>(
      PyCapsule_GetPointer(capsule.ptr(), "arrow_array_stream"));
  if (c_stream == nullptr) {
    throw nb::value_error("Invalid PyCapsule provided");
  }
  nanoarrow::UniqueArrayStream stream{c_stream};

  // Get schema from stream
  nanoarrow::UniqueSchema schema;
  ArrowError error;
  NANOARROW_THROW_NOT_OK(
      ArrowArrayStreamGetSchema(stream.get(), schema.get(), &error));

  // Get the schema view for our child, which we assume is decimal
  ArrowSchemaView schema_view;
  NANOARROW_THROW_NOT_OK(
      ArrowSchemaViewInit(&schema_view, schema->children[0], &error));

  ArrowErrorCode code;
  nanoarrow::ViewArrayStream array_stream{stream.get(), &code, &error};

  for (const auto &chunk : array_stream) {
    nanoarrow::UniqueArrayView array_view;
    ArrowArrayViewInitFromSchema(array_view.get(), schema->children[0], &error);
    NANOARROW_THROW_NOT_OK(
        ArrowArrayViewSetArray(array_view.get(), chunk.children[0], &error));

    struct ArrowBuffer buffer;
    ArrowBufferInit(&buffer);

    struct ArrowDecimal decimal;
    ArrowDecimalInit(&decimal, schema_view.decimal_bitwidth,
                     schema_view.decimal_precision, schema_view.decimal_scale);

    ArrowArrayViewGetDecimalUnsafe(array_view.get(), 0, &decimal);
    NANOARROW_THROW_NOT_OK(ArrowDecimalAppendDigitsToBuffer(&decimal, &buffer));
    const char first_value[] = "12345678901234567890";
    if (strncmp(reinterpret_cast<const char *>(buffer.data), first_value,
                buffer.size_bytes)) {
      throw nb::value_error(
          "first record did not match expectation - expected: " +
          std::string(first_value) + " got: " +
          std::string{reinterpret_cast<const char *>(buffer.data),
                      buffer.size_bytes});
    }
    ArrowBufferReset(&buffer);

    ArrowArrayViewGetDecimalUnsafe(array_view.get(), 1, &decimal);
    NANOARROW_THROW_NOT_OK(ArrowDecimalAppendDigitsToBuffer(&decimal, &buffer));
    const char first_value[] = "-998765432109876543210";
    if (strncmp(reinterpret_cast<const char *>(buffer.data), second_value,
                buffer.size_bytes)) {
      throw nb::value_error(
          "second record did not match expectation - expected: " +
          std::string(second_value) + " got: " +
          std::string{reinterpret_cast<const char *>(buffer.data),
                      buffer.size_bytes});
    }
    ArrowBufferReset(&buffer);
  }
}

NB_MODULE(capsule_bug, m) { m.def("check", &CheckData); }
