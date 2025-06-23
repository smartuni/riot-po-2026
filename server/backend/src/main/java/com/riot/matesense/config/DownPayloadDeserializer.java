//package com.riot.matesense.config;
//
//import com.fasterxml.jackson.core.JsonParser;
//import com.fasterxml.jackson.core.type.TypeReference;
//import com.fasterxml.jackson.databind.*;
//import com.riot.matesense.model.GateForDownlink;
//
//import java.io.IOException;
//import java.util.ArrayList;
//import java.util.List;
//import java.util.stream.Collectors;
//public class DownPayloadDeserializer extends JsonDeserializer<DownPayload> {
//    @Override
//    public DownPayload deserialize(JsonParser p, DeserializationContext ctxt)
//            throws IOException {
//        List<?> arr = p.readValueAs(new TypeReference<List<Object>>(){});
//        DownPayload dto = new DownPayload();
//
//        dto.setId(((Number) arr.get(0)).intValue());
//        dto.setTimestamp(((Number) arr.get(1)).longValue());
//
//        Object rawThird = arr.get(2);
//        List<List<GateForDownlink>> gates = new ArrayList<>();
//
//        if (rawThird instanceof List<?>) {
//            for (Object innerLevel : (List<?>) rawThird) {
//                if (innerLevel instanceof List<?>) {
//                    List<?> entry = (List<?>) innerLevel;
//                    if (entry.size() == 2) {
//                        Object first = entry.get(0);
//                        Object second = entry.get(1);
//                        if (first instanceof Number && second instanceof Number) {
//                            gates.add(List.of(
//                                    new GateForDownlink(((Number) first).intValue(),
//                                            ((Number) second).intValue())
//                            ));
//                        }
//                    }
//                }
//            }
//        }
//
//        dto.setGates(gates);
//        return dto;
//    }
//}
