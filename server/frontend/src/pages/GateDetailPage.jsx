import React, { useState } from 'react';
import { useParams, useNavigate } from 'react-router-dom';
import {
    Select, MenuItem,
    Dialog, DialogTitle, DialogContent, DialogActions,
} from '@mui/material';
import ArrowBackIcon from '@mui/icons-material/ArrowBack';
import EditIcon from '@mui/icons-material/Edit';
import CheckIcon from '@mui/icons-material/Check';
import CloseIcon from '@mui/icons-material/Close';
import LockIcon from '@mui/icons-material/Lock';
import LockOpenIcon from '@mui/icons-material/LockOpen';
import AppLayout from '../features/shell/components/AppLayout';
import StatusChangedDialog from '../features/gates/components/StatusChangedDialog';
import ManualStatusDialog from '../features/gates/components/ManualStatusDialog';
import { useAppSelector } from '../app/store';
import {
    useGetGatesQuery,
    useGetActivitiesQuery,
    useDeleteGateMutation,
    useUpdateGatePriorityMutation,
    useUpdateHeightAboveNNMutation,
} from '../app/store/api/api';

/* ── helpers: map domain values → Hydro-Blue CSS classes ── */

const statusInfo = (status) => {
    switch (status) {
        case 'OPEN': return { cls: 'status-open', label: 'Open', icon: <LockOpenIcon fontSize="small" /> };
        case 'CLOSED': return { cls: 'status-closed', label: 'Closed', icon: <LockIcon fontSize="small" /> };
        default: return { cls: 'status-oos', label: 'Out of Service', icon: null };
    }
};

const requestedStatusInfo = (status) => {
    switch (status) {
        case 'REQUESTED_OPEN': return { cls: 'status-open', label: 'Open' };
        case 'REQUESTED_CLOSE': return { cls: 'status-closed', label: 'Close' };
        default: return { cls: 'status-none', label: 'None' };
    }
};

const pendingJobInfo = (status) => {
    switch (status) {
        case 'PENDING_OPEN': return { cls: 'status-open', label: 'Open' };
        case 'PENDING_CLOSE': return { cls: 'status-closed', label: 'Close' };
        default: return { cls: 'status-none', label: 'None' };
    }
};

const priorityClass = (level) => {
    switch (level) {
        case 0: return 'priority-low';
        case 1: return 'priority-medium';
        case 2: return 'priority-high';
        case 3: return 'priority-critical';
        default: return 'priority-low';
    }
};

const activityTypeInfo = (type) => {
    switch (type) {
        case 'SENSOR_NEW': return { dotClass: 'blue', icon: '🆕' };
        case 'SENSOR_VALUE_CHANGED': return { dotClass: 'amber', icon: '🔄' };
        case 'SENSOR_VALUE_KEEPALIVE': return { dotClass: 'blue', icon: '📡' };
        case 'SENSEMATE_WORKER_REPORT': return { dotClass: 'green', icon: '👷' };
        case 'TARGET_STATE_REQUEST': return { dotClass: 'amber', icon: '📋' };
        case 'MANUAL_STATUS_SET': return { dotClass: 'red', icon: '✋' };
        default: return { dotClass: 'blue', icon: '📊' };
    }
};

const stateConfirmationInfo = (sc) => {
    switch (sc) {
        case 'WORKER_CONFIRMED_SINGLE': return { cls: 'status-open', label: 'Confirmed (1 worker)' };
        case 'WORKER_CONFIRMED_MULTI': return { cls: 'status-closed', label: 'Confirmed (2+ workers)' };
        case 'WORKER_CONFIRMED_ALL': return { cls: 'status-closed', label: 'Confirmed (all workers)' };
        case 'WORKER_CONFLICT': return { cls: 'status-oos', label: 'Conflict' };
        case 'UNCONFIRMED': return { cls: 'status-none', label: 'Unconfirmed' };
        default: return { cls: 'status-none', label: sc || '—' };
    }
};

const formatFullTimestamp = (timestamp) => {
    if (!timestamp) return '—';
    return new Date(timestamp).toLocaleString();
};

const GateDetailPage = () => {
    const { id } = useParams();
    const navigate = useNavigate();
    const user = useAppSelector((state) => state.auth.user);
    const isController = user?.role === 'controller';

    const { data: gates = [], isLoading: gatesLoading, error: gatesError } = useGetGatesQuery();
    const { data: activities = [], isLoading: activitiesLoading } = useGetActivitiesQuery();
    const [deleteGate] = useDeleteGateMutation();
    const [updateGatePriority] = useUpdateGatePriorityMutation();
    const [updateHeightAboveNN] = useUpdateHeightAboveNNMutation();

    const [statusDialogOpen, setStatusDialogOpen] = useState(false);
    const [manualDialogOpen, setManualDialogOpen] = useState(false);
    const [deleteDialogOpen, setDeleteDialogOpen] = useState(false);
    const [heightInput, setHeightInput] = useState('');
    const [editingHeight, setEditingHeight] = useState(false);

    const gate = gates.find((g) => g.id === parseInt(id));

    const gateActivities = activities
        .filter((a) => a.gateId === parseInt(id))
        .sort((a, b) => new Date(b.lastTimeStamp) - new Date(a.lastTimeStamp));

    /* ── loading state ── */
    if (gatesLoading || activitiesLoading) {
        return (
            <AppLayout>
                <div className="card" style={{ padding: '48px', textAlign: 'center', color: 'var(--text-secondary)' }}>
                    Loading...
                </div>
            </AppLayout>
        );
    }

    /* ── error / not found state ── */
    if (gatesError) {
        return (
            <AppLayout>
                <div className="card" style={{ padding: '16px' }}>
                    <div style={{ padding: '16px', background: 'rgba(239,68,68,0.1)', borderRadius: '8px', color: 'var(--red-600)' }}>
                        Failed to load gate data.
                    </div>
                </div>
            </AppLayout>
        );
    }

    if (!gate) {
        return (
            <AppLayout>
                <div className="card" style={{ padding: '16px' }}>
                    <div style={{ padding: '16px', background: 'rgba(239,68,68,0.1)', borderRadius: '8px', color: 'var(--red-600)' }}>
                        Gate with ID {id} was not found.
                    </div>
                </div>
            </AppLayout>
        );
    }

    const si = statusInfo(gate.status);
    const rsi = requestedStatusInfo(gate.requestedStatus);
    const pji = pendingJobInfo(gate.pendingJob);
    const sci = stateConfirmationInfo(gate.stateConfirmation);

    const handleDeleteGate = async () => {
        try {
            await deleteGate(gate.id).unwrap();
            navigate('/dashboard');
        } catch (error) {
            console.error('Failed to delete gate:', error);
            alert('Failed to delete gate.');
        }
    };

    const handlePriorityChange = async (newPriority) => {
        try {
            await updateGatePriority({ gateId: gate.id, priority: newPriority }).unwrap();
        } catch (error) {
            console.error('Failed to update priority:', error);
            alert('Failed to update priority.');
        }
    };

    const handleHeightSubmit = async () => {
        const parsed = parseFloat(heightInput);
        if (isNaN(parsed)) {
            alert('Please enter a valid number.');
            return;
        }
        try {
            await updateHeightAboveNN({ gateId: gate.id, heightAboveNN: parsed }).unwrap();
            setHeightInput('');
            setEditingHeight(false);
        } catch (error) {
            console.error('Failed to update height:', error);
            alert('Failed to update height.');
        }
    };

    const startEditingHeight = () => {
        setHeightInput(gate.heightAboveNN != null ? String(gate.heightAboveNN) : '');
        setEditingHeight(true);
    };

    const cancelEditingHeight = () => {
        setHeightInput('');
        setEditingHeight(false);
    };

    return (
        <AppLayout>
            <button
                className="btn btn-ghost"
                style={{ marginBottom: '16px' }}
                onClick={() => navigate('/dashboard')}
            >
                <ArrowBackIcon fontSize="small" /> Back to Dashboard
            </button>

            {/* ── Gate header (full width) ── */}
            <div className="card" style={{ marginBottom: '16px' }}>
                <div style={{ padding: '24px' }}>
                    <div style={{ display: 'flex', alignItems: 'center', gap: '16px', flexWrap: 'wrap' }}>
                        <span className="gate-id" style={{ fontSize: '20px' }}>G-{gate.id}</span>
                        <h1 style={{ fontSize: '24px', fontWeight: 700, margin: 0 }}>
                            {gate.location || 'Unknown Location'}
                        </h1>
                        <span className={`status-badge ${si.cls}`}>
                            <span className="status-dot" />
                            {si.icon} {si.label}
                        </span>
                    </div>
                    {gate.manualOverride && (
                        <div
                            style={{
                                marginTop: '16px',
                                padding: '12px 16px',
                                background: 'rgba(245,158,11,0.1)',
                                border: '1px solid var(--amber-500)',
                                borderRadius: '8px',
                                color: 'var(--text)',
                                fontSize: '14px',
                                fontWeight: 500,
                                display: 'flex',
                                alignItems: 'center',
                                gap: '8px',
                            }}
                        >
                            <span style={{ fontSize: '18px' }}>✋</span>
                            <span>Status was set manually by an operator</span>
                        </div>
                    )}
                </div>
            </div>

            {/* ── 2-column layout: info+activities (2/3) | actions (1/3) ── */}
            <div style={{ display: 'grid', gridTemplateColumns: '2fr 1fr', gap: '16px', alignItems: 'start' }}>

                {/* ── Left column: Gate info + Activities ── */}
                <div style={{ display: 'flex', flexDirection: 'column', gap: '16px' }}>
                    {/* Gate info card */}
                    <div className="card">
                        <div className="card-header">
                            <span className="card-title">Gate Information</span>
                        </div>
                        <div style={{
                            display: 'grid',
                            gridTemplateColumns: 'repeat(2, 1fr)',
                            gap: '16px',
                            padding: '16px 20px',
                        }}>
                            <div>
                                <span style={{ fontSize: '12px', color: 'var(--text-secondary)', fontWeight: 600 }}>Device ID</span>
                                <p className="mono" style={{ fontSize: '14px', color: 'var(--text)' }}>{gate.deviceId ?? '—'}</p>
                            </div>
                            <div>
                                <span style={{ fontSize: '12px', color: 'var(--text-secondary)', fontWeight: 600 }}>Confidence</span>
                                <p className="mono" style={{ fontSize: '14px', color: 'var(--text)' }}>
                                    {gate.confidence != null ? `${gate.confidence}%` : '—'}
                                    {gate.quality != null && (
                                        <span style={{ color: 'var(--text-secondary)', fontSize: '12px', marginLeft: '6px' }}>
                                            ({gate.quality})
                                        </span>
                                    )}
                                </p>
                            </div>
                            <div>
                                <span style={{ fontSize: '12px', color: 'var(--text-secondary)', fontWeight: 600 }}>Priority</span>
                                <div style={{ display: 'flex', alignItems: 'center', gap: '12px', marginTop: '2px' }}>
                                    <span className={priorityClass(gate.priority ?? 0)} style={{ display: 'inline-flex', alignItems: 'center' }}>
                                        <span className="priority-dot" />
                                        {gate.priority ?? 0}
                                    </span>
                                    {isController && (
                                        <Select
                                            value={gate.priority ?? 0}
                                            onChange={(e) => handlePriorityChange(parseInt(e.target.value))}
                                            variant="outlined"
                                            size="small"
                                            style={{ minWidth: 60 }}
                                        >
                                            {[0, 1, 2, 3].map((level) => (
                                                <MenuItem key={level} value={level}>
                                                    {level}
                                                </MenuItem>
                                            ))}
                                        </Select>
                                    )}
                                </div>
                            </div>
                            <div>
                                <span style={{ fontSize: '12px', color: 'var(--text-secondary)', fontWeight: 600 }}>Height above NN</span>
                                <div style={{ display: 'flex', alignItems: 'center', gap: '6px', marginTop: '2px' }}>
                                    {editingHeight && isController ? (
                                        <>
                                            <input
                                                className="form-input"
                                                type="number"
                                                step="any"
                                                min="0"
                                                value={heightInput}
                                                onChange={(e) => setHeightInput(e.target.value)}
                                                onKeyDown={(e) => {
                                                    if (e.key === 'Enter') handleHeightSubmit();
                                                    if (e.key === 'Escape') cancelEditingHeight();
                                                }}
                                                autoFocus
                                                style={{ width: '80px', fontSize: '13px', padding: '4px 8px' }}
                                            />
                                            <span style={{ fontSize: '13px', color: 'var(--text-secondary)' }}>m</span>
                                            <button
                                                className="action-link"
                                                onClick={handleHeightSubmit}
                                                style={{ color: 'var(--green-600)' }}
                                            >
                                                <CheckIcon fontSize="small" />
                                            </button>
                                            <button
                                                className="action-link"
                                                onClick={cancelEditingHeight}
                                                style={{ color: 'var(--text-secondary)' }}
                                            >
                                                <CloseIcon fontSize="small" />
                                            </button>
                                        </>
                                    ) : (
                                        <>
                                            <p className="mono" style={{ fontSize: '14px', color: 'var(--text)', margin: 0 }}>
                                                {gate.heightAboveNN != null ? `${gate.heightAboveNN} m` : '—'}
                                            </p>
                                            {isController && (
                                                <button
                                                    className="action-link"
                                                    onClick={startEditingHeight}
                                                    style={{ padding: '2px', display: 'inline-flex', alignItems: 'center' }}
                                                >
                                                    <EditIcon fontSize="small" style={{ fontSize: '16px' }} />
                                                </button>
                                            )}
                                        </>
                                    )}
                                </div>
                            </div>
                            <div>
                                <span style={{ fontSize: '12px', color: 'var(--text-secondary)', fontWeight: 600 }}>Requested Status</span>
                                <div style={{ marginTop: '4px' }}>
                                    <span className={`status-badge ${rsi.cls}`}>
                                        <span className="status-dot" />
                                        {rsi.label}
                                    </span>
                                </div>
                            </div>
                            <div>
                                <span style={{ fontSize: '12px', color: 'var(--text-secondary)', fontWeight: 600 }}>Pending Job</span>
                                <div style={{ marginTop: '4px' }}>
                                    <span className={`status-badge ${pji.cls}`}>
                                        <span className="status-dot" />
                                        {pji.label}
                                    </span>
                                </div>
                            </div>
                            <div>
                                <span style={{ fontSize: '12px', color: 'var(--text-secondary)', fontWeight: 600 }}>Last Update</span>
                                <p className="mono" style={{ fontSize: '13px', color: 'var(--text)' }}>
                                    {formatFullTimestamp(gate.lastTimeStamp)}
                                </p>
                            </div>
                            <div>
                                <span style={{ fontSize: '12px', color: 'var(--text-secondary)', fontWeight: 600 }}>Coordinates</span>
                                <p className="coords" style={{ fontSize: '13px' }}>
                                    {gate.latitude != null ? gate.latitude.toFixed(5) : '—'}, {gate.longitude != null ? gate.longitude.toFixed(5) : '—'}
                                </p>
                            </div>
                            <div>
                                <span style={{ fontSize: '12px', color: 'var(--text-secondary)', fontWeight: 600 }}>State Confirmation</span>
                                <div style={{ marginTop: '4px' }}>
                                    <span className={`status-badge ${sci.cls}`}>
                                        <span className="status-dot" />
                                        {sci.label}
                                    </span>
                                </div>
                            </div>
                        </div>
                    </div>

                    {/* Activities card */}
                    <div className="card">
                        <div className="card-header">
                            <span className="card-title">Activities</span>
                            <span style={{ fontSize: '12px', color: 'var(--text-secondary)' }}>
                                {gateActivities.length} total
                            </span>
                        </div>
                        <div className="activity-list">
                            {gateActivities.length === 0 ? (
                                <div style={{ textAlign: 'center', padding: '40px 20px', color: 'var(--text-secondary)', fontSize: '13px' }}>
                                    No activities available for this gate.
                                </div>
                            ) : (
                                gateActivities.map((activity, i) => {
                                    const ati = activityTypeInfo(activity.activityType);
                                    return (
                                        <div className="activity-item" key={activity.id || i}>
                                            <div className={`activity-dot ${ati.dotClass}`}>
                                                {ati.icon}
                                            </div>
                                            <div>
                                                <div className="activity-text">
                                                    <span style={{ fontSize: '11px', color: 'var(--text-secondary)', fontWeight: 600, marginRight: '6px' }}>
                                                        {activity.activityType || 'UNKNOWN'}
                                                    </span>
                                                    {activity.message || ''}
                                                </div>
                                                <div className="activity-time">
                                                    {formatFullTimestamp(activity.lastTimeStamp)}
                                                    {activity.workerId != null && ` · Worker: ${activity.workerId}`}
                                                    {activity.requestedStatus && ` · Requested: ${activity.requestedStatus}`}
                                                </div>
                                            </div>
                                        </div>
                                    );
                                })
                            )}
                        </div>
                    </div>
                </div>

                {/* ── Right column: Actions ── */}
                <div style={{ display: 'flex', flexDirection: 'column', gap: '16px' }}>
                    {isController && (
                        <div className="card">
                            <div className="card-header">
                                <span className="card-title">Actions</span>
                            </div>
                            <div style={{ padding: '16px 20px', display: 'flex', flexDirection: 'column', gap: '10px' }}>
                                <button
                                    className="btn btn-primary"
                                    onClick={() => setManualDialogOpen(true)}
                                    style={{ width: '100%', justifyContent: 'center' }}
                                >
                                    Set Status Manually
                                </button>
                                <button
                                    className="btn btn-outline"
                                    onClick={() => setStatusDialogOpen(true)}
                                    style={{ width: '100%', justifyContent: 'center' }}
                                >
                                    Request Change
                                </button>
                                <button
                                    className="btn btn-danger"
                                    onClick={() => setDeleteDialogOpen(true)}
                                    style={{ width: '100%', justifyContent: 'center' }}
                                >
                                    Delete Gate
                                </button>
                            </div>
                        </div>
                    )}

                    {/* Quick stats sidebar */}
                    <div className="card">
                        <div className="card-header">
                            <span className="card-title">Quick Stats</span>
                        </div>
                        <div style={{ padding: '16px 20px', display: 'flex', flexDirection: 'column', gap: '12px' }}>
                            <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
                                <span style={{ fontSize: '13px', color: 'var(--text-secondary)' }}>Confidence</span>
                                <span style={{ fontFamily: "'JetBrains Mono', monospace", fontSize: '14px', fontWeight: 600, color: gate.confidence >= 90 ? 'var(--green-600)' : gate.confidence >= 70 ? 'var(--amber-600)' : 'var(--red-600)' }}>
                                    {gate.confidence != null ? `${gate.confidence}%` : '—'}
                                </span>
                            </div>
                            <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
                                <span style={{ fontSize: '13px', color: 'var(--text-secondary)' }}>Priority</span>
                                <span style={{ display: 'inline-flex', alignItems: 'center', gap: '4px' }}>
                                    <span className="priority-dot" style={{ background: gate.priority >= 3 ? 'var(--red-600)' : gate.priority >= 2 ? '#f97316' : gate.priority >= 1 ? 'var(--amber-500)' : 'var(--slate-400)' }} />
                                    <span style={{ fontSize: '14px', fontWeight: 600 }}>{gate.priority ?? 0}</span>
                                </span>
                            </div>
                            <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
                                <span style={{ fontSize: '13px', color: 'var(--text-secondary)' }}>Height über NN</span>
                                <span style={{ fontFamily: "'JetBrains Mono', monospace", fontSize: '14px', fontWeight: 600 }}>
                                    {gate.heightAboveNN != null ? `${gate.heightAboveNN} m` : '—'}
                                </span>
                            </div>
                            <div style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
                                <span style={{ fontSize: '13px', color: 'var(--text-secondary)' }}>Activities</span>
                                <span style={{ fontSize: '14px', fontWeight: 600 }}>{gateActivities.length}</span>
                            </div>
                        </div>
                    </div>
                </div>
            </div>

            <ManualStatusDialog
                open={manualDialogOpen}
                gate={gate}
                onClose={() => setManualDialogOpen(false)}
            />

            <StatusChangedDialog
                open={statusDialogOpen}
                gate={gate}
                onClose={() => setStatusDialogOpen(false)}
            />

            <Dialog open={deleteDialogOpen} onClose={() => setDeleteDialogOpen(false)}>
                <DialogTitle>Confirm Deletion</DialogTitle>
                <DialogContent>
                    Are you sure you want to delete the gate with the ID: <strong>{gate.id}</strong>?
                </DialogContent>
                <DialogActions>
                    <button className="btn btn-ghost" onClick={() => setDeleteDialogOpen(false)}>Cancel</button>
                    <button className="btn btn-danger" onClick={handleDeleteGate}>Delete</button>
                </DialogActions>
            </Dialog>
        </AppLayout>
    );
};

export default GateDetailPage;
