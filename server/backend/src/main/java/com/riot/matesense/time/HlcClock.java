package com.riot.matesense.time;

import org.springframework.stereotype.Component;


@Component
public class HlcClock {

    private long lastPhysical = 0;
    private long lastLogical = 0;

    /**
     * Generates a timestamp for a local event (or before sending a message).
     */
    public synchronized HlcTimestamp send() {

        long now = System.currentTimeMillis();

        if (now > lastPhysical) {
            lastPhysical = now;
            lastLogical = 0;
        } else {
            lastLogical++;
        }

        return new HlcTimestamp(lastPhysical, lastLogical);
    }

    /**
     * Updates the clock after receiving a remote timestamp.
     */
    public synchronized HlcTimestamp receive(
            long remotePhysical,
            long remoteLogical) {

        long now = System.currentTimeMillis();

        long maxPhysical =
                Math.max(Math.max(lastPhysical, remotePhysical), now);

        long logical;

        if (maxPhysical == lastPhysical &&
                maxPhysical == remotePhysical) {

            logical = Math.max(lastLogical, remoteLogical) + 1;

        } else if (maxPhysical == lastPhysical) {

            logical = lastLogical + 1;

        } else if (maxPhysical == remotePhysical) {

            logical = remoteLogical + 1;

        } else {

            logical = 0;
        }

        lastPhysical = maxPhysical;
        lastLogical = logical;

        return new HlcTimestamp(lastPhysical, lastLogical);
    }
}