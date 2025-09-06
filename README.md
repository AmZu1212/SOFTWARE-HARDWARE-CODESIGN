# SOFTWARE-HARDWARE-CODESIGN
Software-Hardware co-design HW

# Python Performance Benchmarking Project

This project evaluates two Python benchmarks from the [pyperformance](https://github.com/python/pyperformance) suite:

- **`json_dumps`**: Compares the standard Python `json` library against the optimized [`orjson`](https://github.com/ijl/orjson) library.
- **`crypto_pyaes`**: Compares a pure-Python AES-CTR implementation (`pyaes`) with the [cryptography](https://cryptography.io/) library, which uses OpenSSL and hardware AES-NI instructions.

We used **Linux perf** to capture performance data and generate either:
- **FlameGraphs** for visual profiling  
- **Perf stats** for cycle counts, branch misses, cache misses, etc.  

---

## Repository Structure

```
.
├── Benchmark 1 - JSON_Dumps/
│   ├── runB1.sh                  # Script for json_dumps benchmark
│   ├── Helper Assets/            # Contains json.py and orjson.py benchmark variants
│   ├── Perf Outputs/             # perf.data, .perf, .folded
│   ├── FlameGraph Outputs/       # FlameGraph SVGs
│   └── Results/                  # Clean perf stats results (if SHOW_STATS=true)
│
├── Benchmark 2 - Crypto_PyAES/
│   ├── runB2.sh                  # Script for crypto_pyaes benchmark
│   ├── Helper Assets/            # Contains original_crypto_pyaes.py and new_crypto_pyaes.py
│   ├── Perf Outputs/
│   ├── FlameGraph Outputs/
│   └── Results/
```
---

## Usage

Each benchmark has its own folder:

- **Benchmark 1 (json_dumps)** → `Project Files/Benchmark 1 - JSON_Dumps/`
- **Benchmark 2 (crypto_pyaes)** → `Project Files/Benchmark 2 - Crypto_PyAES/`

### 1. Default mode: Generate FlameGraphs

```bash
cd "Project Files/Benchmark 1 - JSON_Dumps"
./runB1.sh

cd "Project Files/Benchmark 2 - Crypto_PyAES"
./runB2.sh
```

- Runs the benchmark under `perf record`
- Produces folded stack traces
- Generates SVG FlameGraphs in `FlameGraph Outputs/`

### 2. Stats mode: Collect clean perf stats

```bash
cd "Project Files/Benchmark 1 - JSON_Dumps"
SHOW_STATS=true ./runB1.sh

cd "Project Files/Benchmark 2 - Crypto_PyAES"
SHOW_STATS=true ./runB2.sh
```

- Runs the benchmark under `perf stat`
- Saves only the **mean runtime** and the **perf counter summary**
- Output written to `Results/` as:

  - `Normal Results.txt` → standard library / pure Python implementation  
  - `Optimized Results.txt` → optimized library (orjson / cryptography)  

---

## Examples

### json_dumps (standard vs orjson)
- `Normal Results.txt` → standard Python `json.dumps`
- `Optimized Results.txt` → `orjson.dumps` with SIMD acceleration (compiler)

### crypto_pyaes (pyaes vs cryptography)
- `Normal Results.txt` → pure Python AES (pyaes)
- `Optimized Results.txt` → OpenSSL AES-CTR (cryptography, uses AES-NI)

---

## Requirements

- Python 3.10+ with [pyperformance](https://github.com/python/pyperformance) installed
- [Linux perf](https://perf.wiki.kernel.org/)  
- [FlameGraph](https://github.com/brendangregg/FlameGraph) toolkit cloned and available

---

## Notes

- Benchmarks were run inside the `pyperformance` environment (`run_benchmark.py` files are overwritten temporarily by our scripts).  
- Scripts automatically restore the original benchmarks on exit.  
- **Important**: Run as root or with sufficient privileges to allow `perf` to collect profiling data.

---

## Authors

- Amir Zuabi  
- Nir Schif  
