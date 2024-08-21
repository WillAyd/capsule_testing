import pyarrow as pa

from capsule_bug import capsule_bug

def test_bug():
    schema = pa.schema([("col", pa.decimal128(38, 10))])
    tbl = pa.Table.from_arrays([
        pa.array(["1234567890.123456789", "-99876543210.987654321", None])
    ], schema=schema)

    capsule_bug.check(tbl)
