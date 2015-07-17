#!/bin/sh

python - <<END
import riakclient
riakclient.set_config_string('protocol=pbc;host=127.0.0.1')
json = {"id": 1234, "format": 0, "size": 8589934592, "chunk_size": 8388608, "replicas": 3, "name": "example_name"}
riakclient.create_volume(0x1234, json)
print riakclient.open_volume(0x1234, 0)
riakclient.close_volume(0x1234, 'True')
END
