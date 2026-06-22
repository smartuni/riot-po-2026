package com.riot.matesense.time;

public class HlcDemoTest {

    public static void main(String[] args) {

        HlcClock backendClock = new HlcClock();

        System.out.println("=== EVENT 1 ===");

        long devicePhy1 = 1740000000000L;
        long deviceLog1 = 0L;

        HlcTimestamp t1 =
                backendClock.receive(devicePhy1, deviceLog1);

        fakeDbInsert(
                1,
                devicePhy1,
                deviceLog1,
                t1
        );

        System.out.println();

        System.out.println("=== EVENT 2 (same device time) ===");

        long devicePhy2 = 1740000000000L;
        long deviceLog2 = 0L;

        HlcTimestamp t2 =
                backendClock.receive(devicePhy2, deviceLog2);

        fakeDbInsert(
                2,
                devicePhy2,
                deviceLog2,
                t2
        );

        System.out.println();

        System.out.println("=== EVENT 3 (older device clock) ===");

        long devicePhy3 = 1739999999000L;
        long deviceLog3 = 0L;

        HlcTimestamp t3 =
                backendClock.receive(devicePhy3, deviceLog3);

        fakeDbInsert(
                3,
                devicePhy3,
                deviceLog3,
                t3
        );

        System.out.println();

        System.out.println("=== EVENT 4 (future device clock) ===");

        long devicePhy4 = 1740000010000L;
        long deviceLog4 = 0L;

        HlcTimestamp t4 =
                backendClock.receive(devicePhy4, deviceLog4);

        fakeDbInsert(
                4,
                devicePhy4,
                deviceLog4,
                t4
        );
    }

    private static void fakeDbInsert(
            long gateId,
            long incomingPhy,
            long incomingLog,
            HlcTimestamp merged) {

        System.out.println("DEVICE SENT:");
        System.out.println("gateId=" + gateId);
        System.out.println("incomingPhy=" + incomingPhy);
        System.out.println("incomingLog=" + incomingLog);

        System.out.println();

        System.out.println("DB INSERT:");
        System.out.println("gateId=" + gateId);
        System.out.println("hlcPhysical=" + merged.getPhysical());
        System.out.println("hlcLogical=" + merged.getLogical());
    }
}