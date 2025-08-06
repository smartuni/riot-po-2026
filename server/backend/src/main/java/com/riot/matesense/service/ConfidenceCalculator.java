package com.riot.matesense.service;

import com.riot.matesense.entity.GateEntity;
import com.riot.matesense.enums.Status;
import com.riot.matesense.enums.ConfidenceQuality;
import com.riot.matesense.enums.MsgType;

public class ConfidenceCalculator
{
    public ConfidenceCalculator()
    {

    }

    public void changeConfidence(GateEntity entity, int passedConfidence, MsgType reportType) // redesigned: now uses a universal calculator, each gate stores its confidence
    {
        System.out.println("Passed confidence: " + passedConfidence);
        int confidence;
        Status gateStatus = entity.getStatus(); // retrieves status of the gate (already updated in MqttMessageHandler)

        Status[] gateArray = entity.getGateStatusArray() != null ? entity.getGateStatusArray() : new Status[0];
        Status[] workerArray = entity.getWorkerStatusArray() != null ? entity.getWorkerStatusArray() : new Status[0];

        if (gateStatus == Status.UNKNOWN || gateStatus == Status.NONE) { // if we don't know the status of the gate, we can be sure that we don't know, set confidence to 100
            confidence = 100;
            System.out.println("Gate status is unknown/none. Setting confidence to max (100).");
        }
        else
        {
            confidence = passedConfidence; // retrieve confidence from gate
            int iterations = Math.min(3, Math.min(gateArray.length, workerArray.length));
  
            for (int i = 0; i < iterations; i++) // iterate through reports
            {
                if(reportType == MsgType.IST_STATE) // if the report is from a gate sensor
                {
                    int delta = (int)Math.floor(10 / (i + 1)); // calculates how much consideration a report gets, newer reports get more sway
                    if (gateStatus == gateArray[i] && gateArray[i] != Status.NONE)
                    {
                        confidence += delta; // if new a report matches an older report, increase confidence
                    }
                    else if (gateArray[i] != Status.NONE)
                    {
                        confidence -= delta; // otherwise, decrease confidence
                    }
                }
                else if(reportType == MsgType.SEEN_TABLE_STATE) // if the report is from a worker
                {
                    int delta = (int)Math.floor(20 / (i + 1));
                    if (gateStatus == workerArray[i] && workerArray[i] != Status.NONE)
                    {
                        confidence += delta;
                    }
                    else if (workerArray[i] != Status.NONE)
                    {
                        confidence -= delta;
                    }
                }
                else // update not originating from a gate or worker
                {
                    break;
                }
            }
        }

        entity.shuffleReports(gateStatus, reportType); // give older reports less sway for future calculations

        confidence = Math.max(0, Math.min(100, confidence)); // normalize confidence (keep within 0-100)
        entity.setConfidence(confidence);
        System.out.println("Final normalized confidence: " + confidence);

        if (confidence >= 90){ // set a keyword depending on confidence level
            entity.setQuality(ConfidenceQuality.HIGH);
        }
        else if (confidence >= 80){
            entity.setQuality(ConfidenceQuality.MED_HIGH);
        }
        else if (confidence >= 70){
            entity.setQuality(ConfidenceQuality.MED);
        }
        else if (confidence >= 60){
            entity.setQuality(ConfidenceQuality.MED_LOW);
        }
        else {
            entity.setQuality(ConfidenceQuality.LOW);
        }

        System.out.println("Set quality: " + entity.getQuality());

        // Pending Job Check - only if confidenc is 100% set Pending to none!
        String pendingJob = entity.getPendingJob();
        if (entity.getQuality() == ConfidenceQuality.HIGH) {
            if (gateStatus == Status.OPENED && "PENDING_OPEN".equals(pendingJob)) {
                System.out.println("High confidence and gate is OPENED with PENDING_OPEN. Clearing pending job.");
                entity.setPendingJob("None");
            } else if (gateStatus == Status.CLOSED && "PENDING_CLOSE".equals(pendingJob)) {
                System.out.println("High confidence and gate is CLOSED with PENDING_CLOSE. Clearing pending job.");
                entity.setPendingJob("None");
            } else {
                System.out.println("High confidence but no matching pending job state. PendingJob: " + pendingJob + ", GateStatus: " + gateStatus);
            }
        } else {
            System.out.println("Confidence not high enough to modify pending job. Quality: " + entity.getQuality());
        }

        System.out.println("Final pending job state: " + entity.getPendingJob());
    }
}

/*
 * During the presentation, an error came up where the confidence continued to go up despite receiving conflicting reports.
 * This occurred as the GateMate and SenseMate repeatedly reported conflicting states (Gate kept saying it was closed, Sense kept saying it was open).
 * Why this was happening is beyond my understanding, but it revealed a flaw in my confidence calculator:
 * When it receives a gate report, it only compares it to other gate reports instead of both gate and sensor reports, and vice versa.
 * This means that the confidence will always go up in situations like that: because both types of reports only talk to themselves, they'll only see identical reports.
 * Thus, the confidence will always increase.
 * 
 * I won't change the code we presented, but if we had more time, I would change it so it compares reports to all previous reports, not just
 * previous reports of the same type.
 */