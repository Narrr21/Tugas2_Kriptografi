import streamlit as st

import cipher

st.title("Cipher Blok 128-bit")

mode = st.selectbox("Mode", cipher.MODES, index=1)
key_hex = st.text_input("Kunci (32 hex)", "0123456789ABCDEFFEDCBA9876543210")
iv_hex = st.text_input("IV (32 hex)", "A5A5A5A5A5A5A5A55A5A5A5A5A5A5A5A", disabled=mode == "ECB")
op = st.radio("Operasi", ["Enkripsi", "Dekripsi"], horizontal=True)

if op == "Enkripsi":
    text = st.text_area("Plaintext (teks)")
else:
    text = st.text_area("Ciphertext (hex)")
uploaded = st.file_uploader("Atau pilih file")

if st.button(op, type="primary"):
    try:
        key = bytes.fromhex(key_hex)
        iv = bytes.fromhex(iv_hex)
        if uploaded is not None:
            data = uploaded.getvalue()
        elif op == "Enkripsi":
            data = text.encode("utf-8")
        else:
            data = bytes.fromhex(text)

        if op == "Enkripsi":
            result = cipher.encrypt(data, key, mode, iv)
        else:
            result = cipher.decrypt(data, key, mode, iv)
    except ValueError as e:
        st.error(f"Input tidak valid: {e}")
    except cipher.CipherError as e:
        st.error(f"Dekripsi gagal ({e}). Cek kunci, IV, dan mode.")
    else:
        st.subheader("Hasil")
        if op == "Dekripsi":
            try:
                st.code(result.decode("utf-8"), language=None)
            except UnicodeDecodeError:
                st.code(result.hex().upper(), language=None)
        else:
            st.code(result.hex().upper(), language=None)
        st.download_button("Simpan hasil", result, file_name="hasil.bin", on_click="ignore")
