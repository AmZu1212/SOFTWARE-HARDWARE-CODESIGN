#!/usr/bin/env python
# AES-128 CTR benchmark with toggle between pyaes (pure Python) and cryptography (AES-NI)

import pyperf

# Toggle between implementations:
USE_CRYPTOGRAPHY_LIB = True

# 23,000 bytes (unchanged)
CLEARTEXT = b"This is a test. What could possibly go wrong? " * 500

# 128-bit key (16 bytes)
KEY = b'\xa1\xf6%\x8c\x87}_\xcd\x89dHE8\xbf\xc9,'

# Fixed IV (16 bytes) for reproducibility
IV = b"\x00" * 16


if USE_CRYPTOGRAPHY_LIB:
    from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes

    def bench_aes(loops):
        range_it = range(loops)
        t0 = pyperf.perf_counter()

        for _ in range_it:
            # Encrypt
            cipher = Cipher(algorithms.AES(KEY), modes.CTR(IV))
            enc = cipher.encryptor()
            ciphertext = enc.update(CLEARTEXT) + enc.finalize()

            # Decrypt
            cipher = Cipher(algorithms.AES(KEY), modes.CTR(IV))
            dec = cipher.decryptor()
            plaintext = dec.update(ciphertext) + dec.finalize()

        dt = pyperf.perf_counter() - t0
        if plaintext != CLEARTEXT:
            raise Exception("decrypt error!")
        return dt

else:
    import pyaes

    def bench_aes(loops):
        range_it = range(loops)
        t0 = pyperf.perf_counter()

        for _ in range_it:
            aes = pyaes.AESModeOfOperationCTR(KEY)
            ciphertext = aes.encrypt(CLEARTEXT)

            aes = pyaes.AESModeOfOperationCTR(KEY)
            plaintext = aes.decrypt(ciphertext)

        dt = pyperf.perf_counter() - t0
        if plaintext != CLEARTEXT:
            raise Exception("decrypt error!")
        return dt


if __name__ == "__main__":
    runner = pyperf.Runner()
    runner.metadata["description"] = (
        "AES-128 CTR benchmark (backend: "
        + ("cryptography/OpenSSL" if USE_CRYPTOGRAPHY_LIB else "pyaes pure Python")
        + ")"
    )
    runner.bench_time_func("crypto_pyaes", bench_aes)
