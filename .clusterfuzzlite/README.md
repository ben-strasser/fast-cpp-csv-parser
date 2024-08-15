# ClusterFuzzLite set up

This folder contains a fuzzing set for [ClusterFuzzLite](https://google.github.io/clusterfuzzlite).

## Running fuzzing locally

To reproduce this set up the way ClusterFuzzLite does it (by way of [OSS-Fuzz](https://github.com/google/oss-fuzz)) you can do:

```sh
git clone https://github.com/google/oss-fuzz
git clone https://github.com/ben-strasser/fast-cpp-csv-parser
cd fast-cpp-csv-parser

# Build the fuzzers in .clusterfuzzlite
python3 ../oss-fuzz/infra/helper.py build_fuzzers --external $PWD

# Run the fuzzer for 180 seconds
python3 ../oss-fuzz/infra/helper.py run_fuzzer --external $PWD parse_fuzzer-- -max_total_time=180
```
