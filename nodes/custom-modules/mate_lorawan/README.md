Notizen 02.06.2025:
- Base64 -> Hex -> Cbor
- No payload formatter
- 1) cbor_encoder_init - probably only once at start, creates basic structure (not visible) 2) cbor_encoder_create_array - creates array 3) cbor_encoder_create_array - reference to previous encoder, puts array in array
- Basic example message: 
[
[1, 0, 4567], 
[1, 1, 3456],
[1, 1, 2345],
[1, 0, 1234],
]
