# exocortex-wasm-runtime

A minimal C implementation of the TAP (Think-Act-Predict) protocol for WASM targets. Zero heap allocation, fixed-size buffers only.

## API

| Function | Signature | Description |
|---|---|---|
| `tap_sense` | `int tap_sense(int sensor_id)` | Read sensor value (0-1023 range) |
| `tap_recall` | `const char* tap_recall(const char* query)` | Lookup stored memory by key |
| `tap_predict` | `float tap_predict(int sensor_id)` | Predict next sensor value |

## Building

```bash
make          # Build library + tests
make test     # Run tests
make clean    # Clean build artifacts
```

## WASM Compilation (optional)

```bash
make wasm     # Build with wasi-sdk if available
```

## Design

- Fixed-size ring buffers (no malloc/free)
- Deterministic behavior suitable for WASM sandbox
- All state is static — no global heap usage

## License

MIT
