package com.riot.matesense.service;

import com.riot.matesense.entity.GateEntity;
import com.riot.matesense.enums.Status;
import com.riot.matesense.repository.GateRepository;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.autoconfigure.orm.jpa.DataJpaTest;
import org.springframework.test.context.ActiveProfiles;

import static org.assertj.core.api.Assertions.assertThat;

@DataJpaTest
@ActiveProfiles("test")
class SequenceCounterServiceTest {

    @Autowired
    private GateRepository gateRepository;

    private SequenceCounterService seqCounterService;
    private Long gateId;

    @BeforeEach
    void setUp() {
        seqCounterService = new SequenceCounterService(gateRepository);

        GateEntity gate = new GateEntity();
        gate.setId(999L);
        gate.setStatus(Status.CLOSED);
        gate.setLastSeqTx(0);
        gateRepository.save(gate);
        gateId = gate.getId();
    }

    @Test
    void getNextSeq_startsAtZero() {
        assertThat(seqCounterService.getCurrentSeq(gateId)).isEqualTo(0);
    }

    @Test
    void getNextSeq_incrementsByOne() {
        int first = seqCounterService.getNextSeq(gateId);
        assertThat(first).isEqualTo(1);
        assertThat(seqCounterService.getCurrentSeq(gateId)).isEqualTo(1);
    }

    @Test
    void getNextSeq_returnsSequentialValues() {
        assertThat(seqCounterService.getNextSeq(gateId)).isEqualTo(1);
        assertThat(seqCounterService.getNextSeq(gateId)).isEqualTo(2);
        assertThat(seqCounterService.getNextSeq(gateId)).isEqualTo(3);
    }

    @Test
    void getNextSeq_wrapsAt65535() {
        seqCounterService.setSeq(gateId, 0xFFFF);
        assertThat(seqCounterService.getNextSeq(gateId)).isEqualTo(0);
    }

    @Test
    void setSeq_persistsValue() {
        seqCounterService.setSeq(gateId, 42);
        assertThat(seqCounterService.getCurrentSeq(gateId)).isEqualTo(42);
    }

    @Test
    void counterIsPersistedAcrossSaves() {
        seqCounterService.setSeq(gateId, 100);
        GateEntity reloaded = gateRepository.findById(gateId).orElseThrow();
        assertThat(reloaded.getLastSeqTx()).isEqualTo(100);
    }
}
