import React, { useState } from "react";
import axios from "axios";
import LockOpenIcon from '@mui/icons-material/LockOpen';
import LockIcon from '@mui/icons-material/Lock';
import CircleIcon from '@mui/icons-material/Circle';
import CloseIcon from '@mui/icons-material/Close';
import "../styles/FloodGatePopup.css";
import api from "../services/api";

const FloodGatePopup = ({ gate, onClose, onStatusChange }) => {
    const [loading, setLoading] = useState(false);
    const [requested, setRequested] = useState(false);

    const getStatusIcon = (status) => {
        switch (status?.toUpperCase()) {
            case "OPEN":
                return <LockOpenIcon fontSize="small" />;
            case "CLOSED":
                return <LockIcon fontSize="small" />;
            default:
                return <CircleIcon fontSize="small" />;
        }
    };

    const normalizedStatus = gate.status?.toLowerCase();
    const requestedStatus = gate.requestedStatus?.toLowerCase();
    const oppositeStatus = normalizedStatus === "requested_open" ? "requested_close" : "requested_open";
    const alreadyRequested =
        requestedStatus === oppositeStatus.toUpperCase() ||
        gate.status === oppositeStatus.replace("requested_", "").toUpperCase();


    const requestGateAction = async () => {
        if (loading || alreadyRequested) return;

        setLoading(true);
        try {
            // Fix 1: Use correct HTTP method (POST instead of PUT)
            // Fix 2: Send oppositeStatus instead of requestedStatus
            await axios.post(`/${gate.id}/request-status-change`, { requestedStatus: oppositeStatus }, // Fixed: use oppositeStatus
            );
            setRequested(true);
            if (onStatusChange) onStatusChange();
        } catch (error) {
            console.error("Fehler bei Statusanfrage:", error);
        } finally {
            setLoading(false);
        }
    };

    return (
        <div className="popup-overlay">
            <div className="popup-content">
                <button className="popup-close" onClick={onClose}><CloseIcon /></button>
                <h2>Request Status Change</h2>
                <p><strong>Gate:</strong> {gate.id}</p>
                <p><strong>Location:</strong> {gate.location}</p>
                <p>
                    <strong>Current Status:</strong>{" "}
                    <span className={`badge ${normalizedStatus}`}>
                        {getStatusIcon(gate.status)} {gate.status}
                    </span>
                </p>
                <p>
                    <strong>Requested Status:</strong>{" "}
                    <span className={`badge ${requestedStatus || "none"}`}>
                        {getStatusIcon(gate.requestedStatus)} {gate.requestedStatus || "none"}
                    </span>
                </p>

                <button
                    onClick={requestGateAction}
                    disabled={loading || alreadyRequested}
                    className={oppositeStatus === "requested_close" ? "close-button" : "open-button"}
                >
                    {loading ? "Senden..." : `Request ${oppositeStatus.toUpperCase()}`}
                </button>
            </div>
        </div>
    );
};

export default FloodGatePopup;