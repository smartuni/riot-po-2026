package com.riot.matesense.datatypes;

import java.sql.Timestamp;

public class Status {
    Timestamp timestamp;
    Boolean isOpen;
    Long deviceId;
    public Status(){
        this.timestamp = new Timestamp(System.currentTimeMillis());
    }

    public static void main(String[] args) {
       Status status = new Status();
        System.out.println(status.timestamp);
    }
}
