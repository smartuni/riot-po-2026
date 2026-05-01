import React, {useEffect, useState} from "react";
import {fetchGates} from "../services/api";
import SockJS from "sockjs-client";
import {Stomp} from "@stomp/stompjs";

function totalGates(gates) {
    let total = 0;
    let gate;
    for (gate in gates){
        total++;
    }
    return total;
}

function opengates(gates) {
    let tmp = 0;
    let gate;
    for (gate of gates) {
        if (enumToJson(gate.status) === "opened") {
            tmp++;
        }
    }
    return tmp;
}

function closedGates(gates) {
    let tmp = 0;
    let gate;
    for (gate of gates) {
        if (enumToJson(gate.status) === "closed") {
            tmp++;
        }
    }
    return tmp;
}

function gatesOutOfService(gates) {
    let tmp = 0;
    let gate;
    for (gate of gates) {
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
}

function InfoBoxes() {
    const [gates, setGates] = useState([]);

    useEffect(() => {
        const loadGates = async () => {
            try {
                const data = await fetchGates();
                setGates(data);
            } catch (error) {
                console.error('Fehler beim Laden der Gates', error);
            }
        };

        loadGates();
    }, []);

    /**
     * Initialisiert die WebSocket-Verbindung und abonniert die relevanten Topics.
     */
    useEffect(() => {
        const socket = new SockJS('http://localhost:8080/ws');
        const stompClient = Stomp.over(socket);

        stompClient.connect({}, () => {

            stompClient.subscribe('/topic/gates/add', (message) => {
                const activity = JSON.parse(message.body);
                setGates(prev => [...prev, activity]);
            });

            stompClient.subscribe('/topic/gates/delete', (message) => {
                const id = parseInt(message.body);
                setGates(prev => prev.filter(a => a.id !== id));
            });

            stompClient.subscribe('/topic/gates/updates', (message) => {
                const updatedGate = JSON.parse(message.body);
                setGates(prevGates => {
                    const index = prevGates.findIndex(gate => gate.id === updatedGate.id);
                    if (index !== -1) {
                        // Gate exists: replace it
                        const newGates = [...prevGates];
                        newGates[index] = updatedGate;
                        return newGates;
                    }
                });
            });
        });

        return () => {
            stompClient.disconnect();
        };
    }, []);

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
