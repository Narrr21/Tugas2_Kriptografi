import ctypes
from pathlib import Path

MODES = ["ECB", "CBC", "CFB", "OFB", "CTR"]
BLOCK_SIZE = 16

_DLL_PATH = Path(__file__).with_name("cipher.dll")


class CipherError(Exception):
    pass


def _load() -> ctypes.CDLL:
    if not _DLL_PATH.exists():
        raise FileNotFoundError(
            f"{_DLL_PATH} belum ada. Jalankan src\\gui\\build.ps1 terlebih dahulu."
        )
    lib = ctypes.CDLL(str(_DLL_PATH))
    lib.kc_last_error.restype = ctypes.c_char_p
    lib.kc_process.restype = ctypes.c_int
    lib.kc_process.argtypes = [
        ctypes.c_int,
        ctypes.c_char_p, ctypes.c_size_t,
        ctypes.c_char_p,
        ctypes.c_int,
        ctypes.c_char_p,
        ctypes.c_char_p, ctypes.c_size_t, ctypes.POINTER(ctypes.c_size_t),
    ]
    return lib


_lib = _load()


def _check_len(name: str, value: bytes) -> None:
    if len(value) != BLOCK_SIZE:
        raise ValueError(f"{name} harus tepat {BLOCK_SIZE} byte, didapat {len(value)} byte")


def _process(encrypt: bool, data: bytes, key: bytes, mode: str, iv: bytes) -> bytes:
    _check_len("Kunci", key)
    _check_len("IV", iv)
    capacity = len(data) + BLOCK_SIZE
    out = ctypes.create_string_buffer(capacity)
    out_len = ctypes.c_size_t(0)
    status = _lib.kc_process(
        int(encrypt), data, len(data), key, MODES.index(mode), iv,
        out, capacity, ctypes.byref(out_len),
    )
    if status != 0:
        raise CipherError(_lib.kc_last_error().decode("utf-8", errors="replace"))
    return out.raw[: out_len.value]


def encrypt(data: bytes, key: bytes, mode: str, iv: bytes) -> bytes:
    return _process(True, data, key, mode, iv)


def decrypt(data: bytes, key: bytes, mode: str, iv: bytes) -> bytes:
    return _process(False, data, key, mode, iv)
