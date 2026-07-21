# Build for coverage
cmake -S . -B build-coverage \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS="-O0 -g -fprofile-instr-generate -fcoverage-mapping" \
  -DCMAKE_EXE_LINKER_FLAGS="-fprofile-instr-generate"

cmake --build build-coverage

mkdir -p build-coverage/profiles
LLVM_PROFILE_FILE="build-coverage/profiles/%p.profraw" \
  build-coverage/Tests

xcrun llvm-profdata merge -sparse \
  build-coverage/profiles/*.profraw \
  -o build-coverage/coverage.profdata

# Generate terminal summary
xcrun llvm-cov report build-coverage/Tests \
  -instr-profile=build-coverage/coverage.profdata \
  -ignore-filename-regex='(tests|benchmark|_deps)/'

# Generate HTML report
xcrun llvm-cov show build-coverage/Tests \
  -instr-profile=build-coverage/coverage.profdata \
  -format=html \
  -output-dir=build-coverage/coverage-html \
  -ignore-filename-regex='(tests|benchmark|_deps)/'

# Open report
open build-coverage/coverage-html/index.html