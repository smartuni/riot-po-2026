package com.riot.matesense.time;

import org.springframework.stereotype.Component;
import java.util.function.LongSupplier;

@Component

public class HlcClock {

    private final LongSupplier now;

    private long lastPhysical;
    private long lastLogical;

    public HlcClock() {
        this(System::currentTimeMillis);
    }

    public HlcClock(LongSupplier now) {
        this.now = now;
        this.lastPhysical = 0;
        this.lastLogical = 0;
    }

    /**
     * Generates a timestamp for a local event (or before sending a message).
     */

    public  synchronized HlcTimestamp send() {
        long current = now.getAsLong();

        if (current > lastPhysical) {
            lastPhysical = current;
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

        long current = now.getAsLong() ;

        long maxPhysical =  Math.max(current, Math.max(lastPhysical, remotePhysical));;

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