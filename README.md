# SOFTWARE-HARDWARE-CODESIGN

*Software-Hardware co-design HW*  
*Note: The `N-Body Files` folder is a midterm warm-up project and can be ignored.*


# Python Performance Benchmarking Project

This project evaluates two Python benchmarks from the [pyperformance](https://github.com/python/pyperformance).

Please refer to the to the `Report.pdf` in the repository to see the thoughtprocess & explanations.

Suite:

- **`json_dumps`**: Compares the standard Python `json` library against the optimized [`orjson`](https://github.com/ijl/orjson) library.
- **`crypto_pyaes`**: Compares a pure-Python AES-CTR implementation (`pyaes`) with the [cryptography](https://cryptography.io/) library, which uses OpenSSL and hardware AES-NI instructions.

We used **Linux perf** to capture performance data and generate either:
- **FlameGraphs** for visual profiling  
- **Perf stats** for cycle counts, branch misses, cache misses, etc.  

---

## ⚠️ Disclaimer

These scripts are designed for use with the **pyperformance** benchmark suite.  
Before running them, make sure that:

- You have installed all requirements (Python 3.10+, pyperformance, Linux `perf`, FlameGraph).  
- You double-check the **paths to the benchmark files** (e.g., `run_benchmark.py` inside pyperformance).  
- You update the paths if your system uses a different Python version or installation directory.  
- Running as root or with elevated privileges is **your responsibility** — required for `perf` to access low-level CPU counters.

---

## Requirements

- Python 3.10+ with [pyperformance](https://github.com/python/pyperformance) installed
- [Linux perf](https://perf.wiki.kernel.org/)  
- [FlameGraph](https://github.com/brendangregg/FlameGraph) toolkit cloned and available

---

## Repository Structure

```
Project Files/
├── Benchmark 1 - JSON_Dumps/
│   ├── runB1.sh                  # Script for json_dumps benchmark
│   ├── Helper Assets/            # Contains json.py and orjson.py benchmark variants
│   ├── Perf Outputs/             # perf.data, .perf, .folded
│   ├── FlameGraph Outputs/       # FlameGraph SVGs
│   └── Results/                  # Clean perf stats results (if SHOW_STATS=true)
│
└── Benchmark 2 - Crypto_PyAES/
    ├── runB2.sh                  # Script for crypto_pyaes benchmark
    ├── Helper Assets/            # Contains original_crypto_pyaes.py and new_crypto_pyaes.py
    ├── Perf Outputs/
    ├── FlameGraph Outputs/
    └── Results/
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

## Results

- **json_dumps**  
  - The optimized library **orjson** showed significantly lower execution time compared to Python’s built-in `json`.  
  - Gains are due to **SIMD vectorization** and low-level C optimizations.  

- **crypto_pyaes**  
  - The pure-Python AES implementation (**pyaes**) averaged ~600 ms per run.  
  - The optimized version using **cryptography** (OpenSSL + AES-NI) averaged ~100 µs per run.  
  - This demonstrates the dramatic performance benefit of using **hardware acceleration (AES-NI)**.  
  - For extremely high-throughput scenarios, a dedicated accelerator (e.g., **FPGA**) could be used, but CPUs with AES-NI already provide excellent **latency**.  

**Conclusion:** Optimized libraries that leverage native code and hardware features (SIMD, AES-NI) offer orders of magnitude performance improvements over pure-Python implementations.

## Notes

- Benchmarks were run inside the `pyperformance` environment (`run_benchmark.py` files are overwritten temporarily by our scripts).  
- Scripts automatically restore the original benchmarks on exit.  
- **Important**: Run as root or with sufficient privileges to allow `perf` to collect profiling data.

---

## Authors

- Amir Zuabi  
- Nir Schif  
