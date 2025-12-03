package com.riot.matesense.service;

import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.dataformat.cbor.CBORFactory;
import com.fasterxml.jackson.dataformat.cbor.CBORGenerator;
import org.springframework.stereotype.Component;
@Component
public class CborConverter {

    private final ObjectMapper cborMapper;

    public CborConverter() {
        CBORFactory cf = new CBORFactory();
        // TODO: check if there is a way to enforce integers in the range of simple values are automatically encoded as
        //       simple values. The setting below does not help.
        //cf.enable(CBORGenerator.Feature.WRITE_MINIMAL_INTS);
        this.cborMapper = new ObjectMapper(cf);
    }

    public byte[] toCbor(Object data) throws Exception {
        //TODO format new message type
        System.out.println("-------------toCbor():");
        System.out.println(data.toString());

        return cborMapper.writeValueAsBytes(data);
    }
}
