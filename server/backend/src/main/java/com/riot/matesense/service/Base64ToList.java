package com.riot.matesense.service;

import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.dataformat.cbor.CBORFactory;
import org.springframework.stereotype.Service;

import java.util.Base64;
import java.util.List;

@Service
public class Base64ToList {

    public List<Object> decodeBase64ToList(String base64) throws Exception {
        byte[] cborBytes = Base64.getDecoder().decode(base64);
        ObjectMapper mapper = new ObjectMapper(new CBORFactory());
        return mapper.readValue(cborBytes, List.class);
    }
}
