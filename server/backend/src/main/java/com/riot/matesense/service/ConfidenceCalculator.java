package com.riot.matesense.service;

import org.springframework.scheduling.annotation.Scheduled;
import org.springframework.stereotype.Component;

import com.riot.matesense.enums.Status;
import com.riot.matesense.enums.ConfidenceQuality;
import lombok.Getter;

@Component
public class ConfidenceCalculator //retooled to work within existing framework
{
    @Getter
    int confidence;
    ConfidenceQuality quality;
    boolean ignoreGate;
    boolean gateDetector;
    Status[] gateStatusArray = new Status[5];
    Status[] workerStatusArray = new Status[5];

    public ConfidenceCalculator() //initialize an empty list of reports and set confidence to max
    {
        this.confidence = 100;
        this.quality = determineQuality();
        this.ignoreGate = false;
        this.gateDetector = false;
        for(int i = 0; i < 5; i++)
        {
            this.gateStatusArray[i] = Status.NONE;
            this.workerStatusArray[i] = Status.NONE;
        }
    }

    public void shuffleConfidence(Status status) // will need to update once we know the origin of a report
    {
        for(int i = 1; i < 5; i++)
        {
            gateStatusArray[i] = gateStatusArray[i-1]; // push older reports to the back of the array
            workerStatusArray[i] = workerStatusArray[i-1];
        }

        gateStatusArray[0] = status; // insert most recent report to the front of the array
        workerStatusArray[0] = status;
    }

    public ConfidenceQuality updateConfidence(Status status)
    {
        if (status == Status.UNKNOWN || status == Status.NONE) //set confidence to max if status doesn't exist or is reported as unknown
        {
            confidence = 100; //(if we don't know, we're sure that we don't know)
        }
        else
        {
            for(int i = 0; i < 5; i++)
            {
                int gateDelta = 10 - (2 * i); // if gate is flagged as faulty, ignore its reports
                int workerDelta = 20 - (4 * i);

                if (!ignoreGate) 
                {
                    if (status == gateStatusArray[i] && gateStatusArray[i] != Status.NONE)
                    {
                        confidence += gateDelta; // if new a report matches an older report, increase confidence
                    }
                    else
                    {
                        confidence -= gateDelta; // otherwise, decrease confidence
                    }
                }

                if (status == workerStatusArray[i] && workerStatusArray[i] != Status.NONE)
                {
                    confidence += workerDelta;
                }
                else
                {
                    confidence -= workerDelta;
                }
            }
        }

        shuffleConfidence(status);

        confidence = Math.max(0, confidence); // normalize confidence, between 0 and 100
        confidence = Math.min(100, confidence);

        return determineQuality();
    }

    public void subtractConfidence()
    {
        confidence -= 20; //subtract five percent every six hours
    }

    public ConfidenceQuality determineQuality() { // given a confidence number, report a keyword for the confidence (Qualitative, not just a number)
        
        if (confidence >= 90){
            return ConfidenceQuality.HIGH;
        }

        else if (confidence >= 80 && confidence < 90){
            return ConfidenceQuality.MED_HIGH;
        }

        else if (confidence >= 70 && confidence < 80){
            return ConfidenceQuality.MED;
        }

        else if (confidence >= 60 && confidence < 70){
            return ConfidenceQuality.MED_LOW;
        }

        else {
            return ConfidenceQuality.LOW;
        }

    }

    public boolean isGateFaulty () 
    {
        if (gateStatusArray[0] == workerStatusArray [0] && gateStatusArray [1] == workerStatusArray [0])
        {
            gateDetector = false;
        }

        else 
        {
             gateDetector = true;
        }
        return gateDetector;
    }

    public void receiveReports (boolean command) 
    {
        ignoreGate = command;
    }

}




