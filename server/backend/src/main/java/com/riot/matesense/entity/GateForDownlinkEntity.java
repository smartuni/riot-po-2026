package com.riot.matesense.entity;


import com.riot.matesense.enums.Status;
import jakarta.persistence.*;
import lombok.Getter;
import lombok.Setter;

import java.sql.Timestamp;

//Das Node team shcickt sowas hier: [//    2, # 2 is an example value for the message type//    [  # The list with the "Ist Status" entries//        [ # This is a "Ist Status" entry//            187, # GateID//            0,   # Ist Status//            247  # Timestamp//        ],//        [ # 2nd "Ist Status" entry//            69,  # GateID//            1,   # Ist Status//            333  # Timestamp//        ]//        # ... More entries//    ]//]
@Getter
@Setter
@Table(name = "gates_for_downlink")
@Entity
public class GateForDownlinkEntity {
    @Id
    private int id;
    @Column(name = "requested_status")
    private int requestedStatus;

    public GateForDownlinkEntity(int id, int requestedStatus) {
        this.id = id;
        this.requestedStatus = requestedStatus;
    }

    public GateForDownlinkEntity() {
    }
}