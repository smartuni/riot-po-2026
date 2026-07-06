import React, { useState } from "react";
import {
    Dialog, DialogTitle, DialogContent, DialogActions,
} from "@mui/material";
import LockIcon from "@mui/icons-material/Lock";
import LockOpenIcon from "@mui/icons-material/LockOpen";
import { useAppSelector } from "../../../app/store";
import { useRequestGateStatusChangeMutation } from "../../../app/store/api/api";

const selectUser = (state) => state.auth.user;

const getStatusClass = (status) => {
    switch (status?.toUpperCase()) {
        case "OPEN": return "status-open";
        case "CLOSED": return "status-closed";
        case "OUT_OF_SERVICE": return "status-oos";
        default: return "status-none";
    }
};

const getRequestedStatusClass = (status) => {
    switch (status?.toUpperCase()) {
        case "REQUESTED_OPEN": return "status-open";
        case "REQUESTED_CLOSE": return "status-closed";
        case "REQUESTED_NONE": return "status-none";
        default: return "status-none";
    }
};

const getStatusDisplay = (status) => {
    switch (status?.toUpperCase()) {
        case "OPEN":
            return <><LockOpenIcon fontSize="small" /> OPEN</>;
        case "CLOSED":
            return <><LockIcon fontSize="small" /> CLOSED</>;
        default:
            return <>NONE</>;
    }
};

const getRequestedStatusDisplay = (status) => {
    switch (status?.toUpperCase()) {
        case "REQUESTED_OPEN":
            return <><LockOpenIcon fontSize="small" /> Open</>;
        case "REQUESTED_CLOSE":
            return <><LockIcon fontSize="small" /> Close</>;
        default:
            return <>None</>;
    }
};

function StatusChangeDialog({ open, gate, onClose }) {
    const [requestedStatus, setRequestedStatus] = useState("");
    const [error, setError] = useState(null);
    const [requestGateStatusChange] = useRequestGateStatusChangeMutation();
    const userDetails = useAppSelector(selectUser);
    const workerId = userDetails?.workerId;
    const [prevGateId, setPrevGateId] = useState(gate?.id);

    if (gate?.id !== prevGateId) {
        setPrevGateId(gate?.id);
        setError(null);
    }

    if (!gate) return null;

    const handleSubmit = async () => {
        try {
            await requestGateStatusChange({
                gateId: gate.id,
                workerId,
                requestedStatus
            }).unwrap();
            onClose();
            setRequestedStatus("");
            setError(null);
        } catch (err) {
            console.error("Fehler beim Update:", err);
            setError("Failed to request status change. Please try again.");
        }
    };

    const handleClose = () => {
        setError(null);
        setRequestedStatus("");
        onClose();
    };

    return (
        <Dialog open={open} onClose={handleClose}>
            <DialogTitle style={{ fontWeight: 700 }}>Request Status Change</DialogTitle>
            <DialogContent>
                {/* ── Gate info ── */}
                <div style={{ marginBottom: '8px' }}>
                    <span style={{ fontSize: '13px', color: 'var(--text-secondary)' }}>Gate:</span>{' '}
                    <span className="gate-id">G-{gate.id}</span>
                </div>
                <div style={{ marginBottom: '8px' }}>
                    <span style={{ fontSize: '13px', color: 'var(--text-secondary)' }}>Location:</span>{' '}
                    <span style={{ fontSize: '14px', fontWeight: 500 }}>{gate.location}</span>
                </div>
                <div style={{ marginBottom: '8px' }}>
                    <span style={{ fontSize: '13px', color: 'var(--text-secondary)' }}>Current Status:</span>{' '}
                    <span className={`status-badge ${getStatusClass(gate.status)}`}>
                        <span className="status-dot" />
                        {getStatusDisplay(gate.status)}
                    </span>
                </div>
                <div style={{ marginBottom: '12px' }}>
                    <span style={{ fontSize: '13px', color: 'var(--text-secondary)' }}>Requested Status:</span>{' '}
                    <span className={`status-badge ${getRequestedStatusClass(gate.requestedStatus)}`}>
                        <span className="status-dot" />
                        {getRequestedStatusDisplay(gate.requestedStatus)}
                    </span>
                </div>

                {/* ── Info note ── */}
                <div style={{
                    marginTop: '12px',
                    padding: '10px 14px',
                    background: 'rgba(59,130,246,0.08)',
                    border: '1px solid var(--blue-300)',
                    borderRadius: '8px',
                    fontSize: '13px',
                    color: 'var(--text)',
                    display: 'flex',
                    gap: '8px',
                    alignItems: 'flex-start',
                }}>
                    <span style={{ fontSize: '16px', flexShrink: 0 }}>ℹ️</span>
                    <span>This sends a status request to the gate. The actual status changes when the device confirms.</span>
                </div>

                {/* ── New status selector ── */}
                <div style={{ marginTop: '12px', fontSize: '14px', fontWeight: 600, color: 'var(--text)' }}>
                    Select Requested Status:
                </div>
                <div style={{ display: 'flex', gap: '12px', marginTop: '12px' }}>
                    <button
                        type="button"
                        onClick={() => setRequestedStatus('REQUESTED_OPEN')}
                        style={{
                            flex: 1,
                            padding: '16px',
                            borderRadius: '10px',
                            border: requestedStatus === 'REQUESTED_OPEN' ? '2px solid var(--red-600)' : '1px solid var(--border)',
                            background: requestedStatus === 'REQUESTED_OPEN' ? 'rgba(239,68,68,0.06)' : 'var(--bg-card)',
                            cursor: 'pointer',
                            display: 'flex',
                            flexDirection: 'column',
                            alignItems: 'center',
                            gap: '8px',
                            transition: 'all 0.15s',
                        }}
                    >
                        <LockOpenIcon style={{ fontSize: '28px', color: requestedStatus === 'REQUESTED_OPEN' ? 'var(--red-600)' : 'var(--text-secondary)' }} />
                        <span style={{ fontSize: '14px', fontWeight: 600, color: requestedStatus === 'REQUESTED_OPEN' ? 'var(--red-600)' : 'var(--text)' }}>Open</span>
                    </button>
                    <button
                        type="button"
                        onClick={() => setRequestedStatus('REQUESTED_CLOSE')}
                        style={{
                            flex: 1,
                            padding: '16px',
                            borderRadius: '10px',
                            border: requestedStatus === 'REQUESTED_CLOSE' ? '2px solid var(--green-600)' : '1px solid var(--border)',
                            background: requestedStatus === 'REQUESTED_CLOSE' ? 'rgba(34,197,94,0.06)' : 'var(--bg-card)',
                            cursor: 'pointer',
                            display: 'flex',
                            flexDirection: 'column',
                            alignItems: 'center',
                            gap: '8px',
                            transition: 'all 0.15s',
                        }}
                    >
                        <LockIcon style={{ fontSize: '28px', color: requestedStatus === 'REQUESTED_CLOSE' ? 'var(--green-600)' : 'var(--text-secondary)' }} />
                        <span style={{ fontSize: '14px', fontWeight: 600, color: requestedStatus === 'REQUESTED_CLOSE' ? 'var(--green-600)' : 'var(--text)' }}>Closed</span>
                    </button>
                    <button
                        type="button"
                        onClick={() => setRequestedStatus('REQUESTED_NONE')}
                        style={{
                            flex: 1,
                            padding: '16px',
                            borderRadius: '10px',
                            border: requestedStatus === 'REQUESTED_NONE' ? '2px solid var(--slate-500)' : '1px solid var(--border)',
                            background: requestedStatus === 'REQUESTED_NONE' ? 'rgba(100,116,139,0.06)' : 'var(--bg-card)',
                            cursor: 'pointer',
                            display: 'flex',
                            flexDirection: 'column',
                            alignItems: 'center',
                            gap: '8px',
                            transition: 'all 0.15s',
                        }}
                    >
                        <span style={{ fontSize: '28px', color: requestedStatus === 'REQUESTED_NONE' ? 'var(--slate-500)' : 'var(--text-secondary)', lineHeight: 1 }}>∅</span>
                        <span style={{ fontSize: '14px', fontWeight: 600, color: requestedStatus === 'REQUESTED_NONE' ? 'var(--slate-500)' : 'var(--text)' }}>None</span>
                    </button>
                </div>

                {/* ── Error ── */}
                {error && (
                    <div style={{ marginTop: '8px', padding: '8px 12px', background: 'rgba(239,68,68,0.1)', borderRadius: '6px', fontSize: '13px', color: 'var(--red-600)' }}>
                        {error}
                    </div>
                )}
            </DialogContent>
            <DialogActions>
                <button className="btn btn-ghost" onClick={handleClose}>Cancel</button>
                <button
                    className="btn btn-primary"
                    onClick={handleSubmit}
                    disabled={!requestedStatus || !workerId}
                    style={{ opacity: (!requestedStatus || !workerId) ? 0.5 : 1 }}
                >
                    Request Change
                </button>
            </DialogActions>
        </Dialog>
    );
}

export default StatusChangeDialog;
