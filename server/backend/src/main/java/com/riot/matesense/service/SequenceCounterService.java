package com.riot.matesense.service;

import com.riot.matesense.entity.GateEntity;
import com.riot.matesense.repository.GateRepository;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

@Service
public class SequenceCounterService {

    private static final int MAX_SEQ = 0xFFFF;

    private final GateRepository gateRepository;

    public SequenceCounterService(GateRepository gateRepository) {
        this.gateRepository = gateRepository;
    }

    @Transactional
    public int getNextSeq(Long gateId) {
        GateEntity gate = gateRepository.findById(gateId)
                .orElseThrow(() -> new IllegalArgumentException("Gate not found: " + gateId));

        int current = gate.getLastSeqTx();
        int next = (current + 1) & MAX_SEQ;

        gate.setLastSeqTx(next);
        gateRepository.save(gate);

        return next;
    }

    public int getCurrentSeq(Long gateId) {
        GateEntity gate = gateRepository.findById(gateId)
                .orElseThrow(() -> new IllegalArgumentException("Gate not found: " + gateId));
        return gate.getLastSeqTx();
    }

    @Transactional
    public void setSeq(Long gateId, int seq) {
        if (seq < 0 || seq > MAX_SEQ) {
            throw new IllegalArgumentException("Sequence counter out of range: " + seq);
        }
        GateEntity gate = gateRepository.findById(gateId)
                .orElseThrow(() -> new IllegalArgumentException("Gate not found: " + gateId));
        gate.setLastSeqTx(seq);
        gateRepository.save(gate);
    }
}
