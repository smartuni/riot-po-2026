package com.riot.matesense.service;

record GateCommandRecord(
        int version,
        int messageType,
        int recordType,
        byte[] writerId,
        long sequence,
        long hlcPhysical,
        long hlcLogical,
        byte[] targetGateId,
        int targetState
) {}
