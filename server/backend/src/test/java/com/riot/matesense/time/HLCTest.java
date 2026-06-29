package com.riot.matesense.time;

import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.*;

class HlcClockTest {

    private static int compare(HlcTimestamp a, HlcTimestamp b) {
        if (a.getPhysical() < b.getPhysical()) {
            return -1;
        }
        if (a.getPhysical() > b.getPhysical()) {
            return 1;
        }
        return Long.compare(a.getLogical(), b.getLogical());
    }

    @Test
    void testInit() {
        HlcClock clock = new HlcClock();
        assertNotNull(clock);
    }

    @Test
    void testGetCurrentTimestamp() {
        HlcClock clock = new HlcClock();

        HlcTimestamp ts0 = clock.send();
        HlcTimestamp ts1 = clock.send();

        assertEquals(1, compare(ts1, ts0));
        assertEquals(-1, compare(ts0, ts1));

        assertEquals(0, compare(ts0, ts0));
        assertEquals(0, compare(ts1, ts1));
    }

    @Test
    void testUpdateCurrentTimestampWithLater() {
        HlcClock clock = new HlcClock();

        HlcTimestamp ts0 = clock.send();

        HlcTimestamp updated = clock.receive(
                ts0.getPhysical() + 10,
                5);

        HlcTimestamp remote = new HlcTimestamp(
                ts0.getPhysical() + 10,
                5);

        assertEquals(1, compare(updated, remote));
    }

    @Test
    void testUpdateCurrentTimestampWithEarlier() {
        HlcClock clock = new HlcClock();

        HlcTimestamp ts0 = clock.send();

        HlcTimestamp remote = new HlcTimestamp(
                ts0.getPhysical() + 10,
                0);

        assertEquals(1, compare(remote, ts0));

        HlcTimestamp updated = clock.receive(
                remote.getPhysical(),
                remote.getLogical());

        assertEquals(1, compare(updated, remote));
        assertEquals(1, compare(updated, ts0));
    }

    @Test
    void testComplexTimeSeries() {

        HlcClock nodeA = new HlcClock();
        HlcClock nodeB = new HlcClock();

        // Events on node A
        HlcTimestamp a1 = nodeA.send();
        HlcTimestamp a2 = nodeA.send();

        // Events on node B
        HlcTimestamp b1 = nodeB.send();
        HlcTimestamp b2 = nodeB.send();

        // Node B receives a1
        HlcTimestamp afterA1 = nodeB.receive(
                a1.getPhysical(),
                a1.getLogical());

        assertEquals(1, compare(afterA1, a1));

        // Prevent warnings
        assertNotNull(a2);
        assertNotNull(b1);
        assertNotNull(b2);
    }

    @Test
    void testManyLocalEvents() {

        HlcClock clock = new HlcClock();

        HlcTimestamp previous = clock.send();

        for (int i = 0; i < 100; i++) {

            HlcTimestamp current = clock.send();

            assertEquals(1, compare(current, previous));

            previous = current;
        }
    }

    @Test
    void testReceiveEqualTimestamp() {

        HlcClock clock = new HlcClock();

        HlcTimestamp local = clock.send();

        HlcTimestamp updated = clock.receive(
                local.getPhysical(),
                local.getLogical());

        assertEquals(local.getPhysical(), updated.getPhysical());
        assertEquals(local.getLogical() + 1, updated.getLogical());
    }

    @Test
    void testReceiveOlderTimestamp() {

        HlcClock clock = new HlcClock();

        HlcTimestamp local = clock.send();

        HlcTimestamp updated = clock.receive(
                local.getPhysical() - 1000,
                0);

        assertEquals(1, compare(updated, local));
    }
}