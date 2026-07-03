import React, { useState } from 'react';
import { useParams, useNavigate } from 'react-router-dom';
import {
    Select, MenuItem,
    Dialog, DialogTitle, DialogContent, DialogActions,
    Button, Typography,
} from '@mui/material';
import ArrowBackIcon from '@mui/icons-material/ArrowBack';
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

const activityTypeClass = (type) => {
    switch (type) {
        case 'status_change':
        case 'STATUS_CHANGE':
            return 'status-closed';
        case 'error':
        case 'ERROR':
        case 'alert':
        case 'ALERT':
            return 'status-open';
        default:
            return 'status-oos';
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

    const [statusDialogOpen, setStatusDialogOpen] = useState(false);
    const [manualDialogOpen, setManualDialogOpen] = useState(false);
    const [deleteDialogOpen, setDeleteDialogOpen] = useState(false);

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
                    <div style={{ padding: '16px', background: 'var(--red-100)', borderRadius: '8px', color: 'var(--red-600)' }}>
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
                    <div style={{ padding: '16px', background: 'var(--red-100)', borderRadius: '8px', color: 'var(--red-600)' }}>
                        Gate with ID {id} was not found.
                    </div>
                </div>
            </AppLayout>
        );
    }

    const si = statusInfo(gate.status);
    const rsi = requestedStatusInfo(gate.requestedStatus);
    const pji = pendingJobInfo(gate.pendingJob);

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

    return (
        <AppLayout>
            {/* ── Back button ── */}
            <button
                className="btn btn-ghost"
                style={{ marginBottom: '16px' }}
                onClick={() => navigate('/dashboard')}
            >
                <ArrowBackIcon fontSize="small" /> Back to Dashboard
            </button>

            {/* ── Gate header ── */}
            <div className="card" style={{ marginBottom: '16px' }}>
                <div style={{ padding: '24px 24px 0' }}>
                    <div style={{ display: 'flex', alignItems: 'center', gap: '16px', flexWrap: 'wrap' }}>
                        <span className="gate-id" style={{ fontSize: '20px' }}>G-{gate.id}</span>
                        <Typography variant="h5" component="h1" sx={{ fontWeight: 700 }}>
                            {gate.location || 'Unknown Location'}
                        </Typography>
                        <span className={`status-badge ${si.cls}`}>
                            <span className="status-dot" />
                            {si.icon} {si.label}
                        </span>
                    </div>
                </div>

                {/* ── Manual override banner ── */}
                {gate.manualOverride && (
                    <div
                        style={{
                            margin: '16px 24px',
                            padding: '12px 16px',
                            background: 'var(--amber-100)',
                            border: '1px solid var(--amber-500)',
                            borderRadius: '8px',
                            color: 'var(--amber-600)',
                            fontSize: '14px',
                            fontWeight: 600,
                        }}
                    >
                        Status was set manually
                    </div>
                )}

                {/* ── Gate info grid ── */}
                <div style={{
                    display: 'grid',
                    gridTemplateColumns: 'repeat(2, 1fr)',
                    gap: '16px',
                    padding: '16px 24px 24px',
                }}>
                    <div>
                        <Typography variant="caption" color="textSecondary">Device ID</Typography>
                        <Typography className="mono">{gate.deviceId ?? '—'}</Typography>
                    </div>
                    <div>
                        <Typography variant="caption" color="textSecondary">Confidence</Typography>
                        <Typography className="mono">
                            {gate.confidence != null ? `${gate.confidence}%` : '—'}
                            {gate.quality != null && (
                                <span style={{ color: 'var(--text-secondary)', fontSize: '12px', marginLeft: '6px' }}>
                                    ({gate.quality})
                                </span>
                            )}
                        </Typography>
                    </div>
                    <div>
                        <Typography variant="caption" color="textSecondary">Priority</Typography>
                        <div style={{ display: 'flex', alignItems: 'center', gap: '12px' }}>
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
                        <Typography variant="caption" color="textSecondary">Requested Status</Typography>
                        <div>
                            <span className={`status-badge ${rsi.cls}`}>
                                <span className="status-dot" />
                                {rsi.label}
                            </span>
                        </div>
                    </div>
                    <div>
                        <Typography variant="caption" color="textSecondary">Pending Job</Typography>
                        <div>
                            <span className={`status-badge ${pji.cls}`}>
                                <span className="status-dot" />
                                {pji.label}
                            </span>
                        </div>
                    </div>
                    <div>
                        <Typography variant="caption" color="textSecondary">Last Update</Typography>
                        <Typography className="mono" style={{ fontSize: '13px' }}>
                            {formatFullTimestamp(gate.lastTimeStamp)}
                        </Typography>
                    </div>
                    <div>
                        <Typography variant="caption" color="textSecondary">Coordinates</Typography>
                        <Typography className="coords" style={{ fontSize: '13px' }}>
                            {gate.latitude != null ? gate.latitude.toFixed(5) : '—'}, {gate.longitude != null ? gate.longitude.toFixed(5) : '—'}
                        </Typography>
                    </div>
                    <div>
                        <Typography variant="caption" color="textSecondary">State Confirmation</Typography>
                        <Typography style={{ fontSize: '13px' }}>
                            {gate.stateConfirmation || '—'}
                        </Typography>
                    </div>
                </div>
            </div>

            {/* ── Actions (controller only) ── */}
            {isController && (
                <div className="card" style={{ marginBottom: '16px' }}>
                    <div className="card-header">
                        <span className="card-title">Actions</span>
                    </div>
                    <div style={{ padding: '16px 20px', display: 'flex', gap: '12px', flexWrap: 'wrap' }}>
                        <button
                            className="btn btn-primary"
                            onClick={() => setManualDialogOpen(true)}
                        >
                            Set Status Manually
                        </button>
                        <button
                            className="btn btn-outline"
                            onClick={() => setStatusDialogOpen(true)}
                        >
                            Request Change
                        </button>
                        <button
                            className="btn btn-danger"
                            onClick={() => setDeleteDialogOpen(true)}
                        >
                            Delete Gate
                        </button>
                    </div>
                </div>
            )}

            {/* ── Activities section ── */}
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
                        gateActivities.map((activity, i) => (
                            <div className="activity-item" key={activity.id || i}>
                                <div className={`activity-dot blue`}>
                                    📊
                                </div>
                                <div>
                                    <div className="activity-text">
                                        <span className={`status-badge ${activityTypeClass(activity.activityType)}`} style={{ marginRight: '8px' }}>
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
                        ))
                    )}
                </div>
            </div>

            {/* ── Dialogs ── */}
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
