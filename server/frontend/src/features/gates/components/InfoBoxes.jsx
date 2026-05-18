import React from "react";
import { useGetGatesQuery } from "../../../app/store/api/api";
import { CircularProgress, Alert } from "@mui/material";

function totalGates(gates) {
    return gates.length;
}

function opengates(gates) {
    let tmp = 0;
    for (const gate of gates) {
        if (enumToJson(gate.status) === "opened") {
            tmp++;
        }
    }
    return tmp;
}

function closedGates(gates) {
    let tmp = 0;
    for (const gate of gates) {
        if (enumToJson(gate.status) === "closed") {
            tmp++;
        }
    }
    return tmp;
}

function gatesOutOfService(gates) {
    let tmp = 0;
    for (const gate of gates) {
        if (enumToJson(gate.status) === "unknown") {
            tmp++;
        }
    }
    return tmp;
}

function enumToJson(inString) {
    if (inString === "OPENED") {
        return "opened"
    } else if (inString === "CLOSED") {
        return "closed"
    } else if (inString === "UNKNOWN") {
        return "unknown"
    }
    return "unknown"
}

function InfoBoxes() {
    const { data: gates = [], isLoading, error } = useGetGatesQuery();

    if (isLoading) {
        return (
            <div className="info-boxes" style={{ display: 'flex', justifyContent: 'center', alignItems: 'center', minHeight: '200px' }}>
                <CircularProgress />
            </div>
        );
    }

    if (error) {
        return (
            <div className="info-boxes" style={{ padding: '2rem' }}>
                <Alert severity="error">Failed to load gates data: {error.toString()}</Alert>
            </div>
        );
    }

    return (
        <div className="info-boxes">
            <div className="box">
                <h5>
                    Total Gate
                </h5>
                <h1>
                    {totalGates(gates)}
                </h1>
            </div>
            <div className="box"><h5>
                Open
            </h5>
                <h1>{opengates(gates)}</h1>
            </div>
            <div className="box"><h5>
                Closed
            </h5>
                <h1>
                    {closedGates(gates)}
                </h1>
            </div>
            <div className="box">
                <h5>
                    Out of Service
                </h5>
                <h1>{gatesOutOfService(gates)}</h1>
            </div>
        </div>
    );
}

export default InfoBoxes;