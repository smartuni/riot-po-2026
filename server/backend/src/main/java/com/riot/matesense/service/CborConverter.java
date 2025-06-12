package com.riot.matesense.service;

import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.dataformat.cbor.CBORFactory;
import org.springframework.stereotype.Component;

@Component
public class CborConverter {

    private final ObjectMapper cborMapper;

    public CborConverter() {
        this.cborMapper = new ObjectMapper(new CBORFactory());
    }

    public byte[] toCbor(Object data) throws Exception {
        return cborMapper.writeValueAsBytes(data);
    }
}
