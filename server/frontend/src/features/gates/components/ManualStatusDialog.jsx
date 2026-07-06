import React, { useState } from "react";
import {
    Dialog, DialogTitle, DialogContent, DialogActions,
} from "@mui/material";
import LockIcon from "@mui/icons-material/Lock";
import LockOpenIcon from "@mui/icons-material/LockOpen";
import { useAppSelector } from "../../../app/store";
import { useSetGateStatusManuallyMutation } from "../../../app/store/api/api";

const selectUser = (state) => state.auth.user;

const getStatusClass = (status) => {
    switch (status?.toUpperCase()) {
        case "OPEN": return "status-open";
        case "CLOSED": return "status-closed";
        case "OUT_OF_SERVICE": return "status-oos";
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

function ManualStatusDialog({ open, gate, onClose }) {
    const [newStatus, setNewStatus] = useState("");
    const [error, setError] = useState(null);
    const [setGateStatusManually] = useSetGateStatusManuallyMutation();
    const userDetails = useAppSelector(selectUser);
    const workerId = userDetails?.workerId;
    const [prevGateId, setPrevGateId] = useState(gate?.id);

    // Reset error when switching gates
    if (gate?.id !== prevGateId) {
        setPrevGateId(gate?.id);
        setError(null);
    }

    if (!gate) return null;

    const handleSubmit = async () => {
        try {
            await setGateStatusManually({
                gateId: gate.id,
                workerId,
                status: newStatus,
            }).unwrap();
            onClose();
            setNewStatus("");
            setError(null);
        } catch (err) {
            console.error("Failed to set gate status manually:", err);
            setError("Failed to set gate status. Please try again.");
        }
    };

    const handleClose = () => {
        setError(null);
        setNewStatus("");
        onClose();
    };

    return (
        <Dialog open={open} onClose={handleClose}>
            <DialogTitle style={{ fontWeight: 700 }}>Set Gate Status Manually</DialogTitle>
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
                <div style={{ marginBottom: '12px' }}>
                    <span style={{ fontSize: '13px', color: 'var(--text-secondary)' }}>Current Status:</span>{' '}
                    <span className={`status-badge ${getStatusClass(gate.status)}`}>
                        <span className="status-dot" />
                        {getStatusDisplay(gate.status)}
                    </span>
                </div>

                {/* ── Manual override note (info) ── */}
                {gate.manualOverride && (
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
                        <span>Status was previously set manually.</span>
                    </div>
                )}

                {/* ── Warning ── */}
                <div style={{
                    marginTop: '12px',
                    padding: '10px 14px',
                    background: 'rgba(245,158,11,0.08)',
                    border: '1px solid var(--amber-500)',
                    borderRadius: '8px',
                    fontSize: '13px',
                    color: 'var(--text)',
                    display: 'flex',
                    gap: '8px',
                    alignItems: 'flex-start',
                }}>
                    <span style={{ fontSize: '16px', flexShrink: 0 }}>⚠️</span>
                    <span><strong>Warning:</strong> This will immediately override the gate status. Use with caution.</span>
                </div>

                {/* ── New status selector ── */}
                <div style={{ marginTop: '12px', fontSize: '14px', fontWeight: 600, color: 'var(--text)' }}>
                    Select New Status:
                </div>
                <div style={{ display: 'flex', gap: '12px', marginTop: '12px' }}>
                    <button
                        type="button"
                        onClick={() => setNewStatus('OPEN')}
                        style={{
                            flex: 1,
                            padding: '16px',
                            borderRadius: '10px',
                            border: newStatus === 'OPEN' ? '2px solid var(--red-600)' : '1px solid var(--border)',
                            background: newStatus === 'OPEN' ? 'rgba(239,68,68,0.06)' : 'var(--bg-card)',
                            cursor: 'pointer',
                            display: 'flex',
                            flexDirection: 'column',
                            alignItems: 'center',
                            gap: '8px',
                            transition: 'all 0.15s',
                        }}
                    >
                        <LockOpenIcon style={{ fontSize: '28px', color: newStatus === 'OPEN' ? 'var(--red-600)' : 'var(--text-secondary)' }} />
                        <span style={{ fontSize: '14px', fontWeight: 600, color: newStatus === 'OPEN' ? 'var(--red-600)' : 'var(--text)' }}>Open</span>
                    </button>
                    <button
                        type="button"
                        onClick={() => setNewStatus('CLOSED')}
                        style={{
                            flex: 1,
                            padding: '16px',
                            borderRadius: '10px',
                            border: newStatus === 'CLOSED' ? '2px solid var(--green-600)' : '1px solid var(--border)',
                            background: newStatus === 'CLOSED' ? 'rgba(34,197,94,0.06)' : 'var(--bg-card)',
                            cursor: 'pointer',
                            display: 'flex',
                            flexDirection: 'column',
                            alignItems: 'center',
                            gap: '8px',
                            transition: 'all 0.15s',
                        }}
                    >
                        <LockIcon style={{ fontSize: '28px', color: newStatus === 'CLOSED' ? 'var(--green-600)' : 'var(--text-secondary)' }} />
                        <span style={{ fontSize: '14px', fontWeight: 600, color: newStatus === 'CLOSED' ? 'var(--green-600)' : 'var(--text)' }}>Closed</span>
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
                    disabled={!newStatus || !workerId}
                    style={{ opacity: (!newStatus || !workerId) ? 0.5 : 1 }}
                >
                    Set Status
                </button>
            </DialogActions>
        </Dialog>
    );
}

export default ManualStatusDialog;
