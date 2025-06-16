import InfoBoxes from './InfoBoxes';
import HeaderBar from './HeaderBar';
import Sidebar from './Sidebar';
import '../styles/Gates.css';
import { fetchGates } from '../services/api';
import { useEffect, useState } from 'react';

import sensorGreen from '../assets/img/png/sensor_green.png';
import sensorRed from '../assets/img/png/sensor_red.png';
import workerGreen from '../assets/img/png/worker_green.png';
import workerRed from '../assets/img/png/worker_red.png';
import api from '../services/api'
import axios from "axios";

const STATUS_CLASSES = {
    open: 'status-open',
    closed: 'status-closed',
    unknown: 'status-unknown',
};



const getSensorIcon = (sensorConfidence) => (sensorConfidence ? sensorGreen : sensorRed);
const getWorkerIcon = (workerConfidence) => (workerConfidence ? workerGreen : workerRed);

const getStatusLabel = (status) => {
    const normalized = normalizeStatus(status);
    const cls = STATUS_CLASSES[normalized] || 'status-unknown';
    return <span className={cls}>{normalized}</span>;
};


const normalizeStatus = (status) => {
    if (!status) return 'unknown';
    const s = status.toLowerCase();
    return s === 'close' ? 'closed' : s;
};


const GatesPage = () => {
    const [gates, setGates] = useState([]);
    const [loadingGateIds, setLoadingGateIds] = useState([]);

    const isGateLoading = (gateId) => loadingGateIds.includes(gateId);

    const addLoadingGate = (gateId) =>
        setLoadingGateIds((prev) => [...new Set([...prev, gateId])]);

    const removeLoadingGate = (gateId) =>
        setLoadingGateIds((prev) => prev.filter((id) => id !== gateId));

    const requestGateAction = async (gateId, requestedStatus) => {
        addLoadingGate(gateId);
        try {
            const response = await axios.post(`/${gateId}/request-status-change`, { requestedStatus: requestedStatus });

            if (!response.ok) {
                throw new Error('Fehler beim Senden der Statusänderung');
            }

            console.log(`Statusänderung für Gate ${gateId} zu "${requestedStatus}" angefragt.`);
        } catch (error) {
            console.error('Fehler bei Statusanfrage:', error);
        } finally {
            removeLoadingGate(gateId);
        }
    };

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
        const interval = setInterval(loadGates, 10000);
        return () => clearInterval(interval);
    }, []);

    const getActionButton = (gate) => {
        const gateId = gate.id;
        const isLoading = isGateLoading(gateId);

        const currentStatus = gate.status?.toLowerCase();
        const requestedStatus = gate.requestedStatus?.toLowerCase();

        if (currentStatus === 'unknown') {
            return (
                <button
                    className="support-button"
                    onClick={() => requestGateAction(gateId, 'support')}
                    disabled={isLoading}
                >
                    {isLoading ? 'Senden...' : 'support'}
                </button>
            );
        }

        const oppositeStatus = currentStatus === 'open' ? 'close' : 'open';
        const isAlreadyRequested = requestedStatus === oppositeStatus;

        return (
            <button
                className={oppositeStatus === 'close' ? 'close-button' : 'open-button'}
                onClick={() => requestGateAction(gateId, oppositeStatus)}
                disabled={isLoading || isAlreadyRequested}
            >
                {isLoading ? 'Senden...' : oppositeStatus}
            </button>
        );
    };

    return (
        <div>
            <HeaderBar />
            <div className="container">
                <Sidebar />
                <div className="main">
                    <InfoBoxes />
                    <h1>Flood Gates (worker):</h1>
                    <div className="gate-boxes">
                        {gates.map((gate) => (
                            <div key={gate.id} className="gate-box">
                                <div className="floodgate-card">
                                    <h2 className="floodgate-title">Flood Gate: {gate.id}</h2>

                                    {/* First Row */}
                                    <div className="floodgate-row">
                                        <div>{gate.location}</div>
                                        <div>
                                            <img src={getWorkerIcon(gate.workerConfidence)} alt="worker status" />
                                        </div>
                                        <div>
                                            <img src={getSensorIcon(gate.sensorConfidence)} alt="sensor status" />
                                        </div>
                                    </div>
                                    <div className="floodgate-header">
                                        <div>Location</div>
                                        <div>Worker-Gate-Status</div>
                                        <div>Sensor-Gate-Status</div>
                                    </div>

                                    {/* Second Row */}
                                    <div className="floodgate-header">
                                        <div>Current Status</div>
                                        <div>Requested Status</div>
                                        <div>Action</div>
                                    </div>
                                    <div className="floodgate-row">
                                        <div>{getStatusLabel(gate.status?.toLowerCase())}</div>
                                        <div>
                                            {gate.requestedStatus
                                                ? getStatusLabel(gate.requestedStatus?.toLowerCase())
                                                : <span className="status-unknown">none</span>}
                                        </div>
                                        <div>{getActionButton(gate)}</div>
                                    </div>
                                </div>
                            </div>
                        ))}
                    </div>
                </div>
            </div>
        </div>
    );
};

export default GatesPage;
