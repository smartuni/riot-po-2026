import React, { useState } from "react";
import { useNavigate } from "react-router-dom";
import { useAppSelector } from "../../../app/store";
import {
    useGetGatesQuery,
    useGetActivitiesQuery,
    useGetDownlinkCounterQuery,
    useRequestGateStatusChangeMutation,
    useTryIncrementDownlinkCounterMutation,
    useCreateGateMutation,
    useSendDownlinkMutation,
    useResetDownlinkCounterMutation,
} from "../../../app/store/api/api";
import {
    Dialog,
    DialogTitle,
    DialogContent,
    DialogActions,
} from "@mui/material";
import ArrowForwardIcon from "@mui/icons-material/ArrowForward";
import DoneAllIcon from "@mui/icons-material/DoneAll";
import WarningIcon from "@mui/icons-material/Warning";
import StatusChangedDialog from "./StatusChangedDialog";

const statusInfo = (status) => {
    switch (status) {
        case "OPEN": return { cls: "status-open", label: "Open" };
        case "CLOSED": return { cls: "status-closed", label: "Closed" };
        default: return { cls: "status-oos", label: "Out of Service" };
    }
};

const confirmationIndicator = (stateConfirmation) => {
    switch (stateConfirmation) {
        case "WORKER_CONFLICT": return { label: "⚠", color: "var(--red-600)" };
        case "UNCONFIRMED": return null;
        case "WORKER_CONFIRMED_SINGLE": return { label: "✓", color: "var(--blue-600)" };
        case "WORKER_CONFIRMED_MULTI":
        case "WORKER_CONFIRMED_ALL": return { label: "✓✓", color: "var(--green-600)" };
        default: return null;
    }
};

function ConfidenceCell({ gate }) {
    const sc = gate.stateConfirmation;
    let icon = null;
    let title = "Unconfirmed";
    let color = "var(--text-secondary)";

    if (sc === "WORKER_CONFIRMED_SINGLE") {
        icon = <ArrowForwardIcon className="confidence-icon" style={{ color: "var(--blue-600)" }} />;
        title = "Confirmed by 1 worker";
        color = "var(--blue-600)";
    } else if (sc === "WORKER_CONFIRMED_MULTI" || sc === "WORKER_CONFIRMED_ALL") {
        icon = <DoneAllIcon className="confidence-icon" style={{ color: "var(--green-600)" }} />;
        title = "Confirmed by 2+ workers";
        color = "var(--green-600)";
    } else if (sc === "WORKER_CONFLICT") {
        icon = <WarningIcon className="confidence-icon" style={{ color: "var(--red-600)" }} />;
        title = "Conflict: workers disagree";
        color = "var(--red-600)";
    }

    const confidencePct = gate.confidence != null ? `${gate.confidence}%` : "—";

    return (
        <span className="confidence-indicator" title={title}>
            {icon}
            <span className="confidence-value" style={{ color }}>{confidencePct}</span>
        </span>
    );
}

const FILTER_TABS = [
    { label: "All", value: "" },
    { label: "Open", value: "OPEN" },
    { label: "Closed", value: "CLOSED" },
    { label: "OOS", value: "OUT_OF_SERVICE" },
];

function StatusTables({ filter: filterProp, setFilter: setFilterProp }) {
    const navigate = useNavigate();

    const { data: gates = [], isLoading: gatesLoading, error: gatesError } = useGetGatesQuery();
    const { data: activities = [], isLoading: activitiesLoading, error: activitiesError } = useGetActivitiesQuery();
    const { data: downlinkCounterData } = useGetDownlinkCounterQuery();
    const userDetails = useAppSelector((state) => state.auth.user);

    const downlinkCount = downlinkCounterData ?? 0;
    const workerId = userDetails?.workerId ?? null;

    const [requestGateStatusChange] = useRequestGateStatusChangeMutation();
    const [tryIncrementDownlinkCounter] = useTryIncrementDownlinkCounterMutation();
    const [createGate] = useCreateGateMutation();
    const [sendDownlink] = useSendDownlinkMutation();
    const [resetDownlinkCounter] = useResetDownlinkCounterMutation();

    const [search, setSearch] = useState("");
    const [filterLocal, setFilterLocal] = useState("");
    const [selectedGate, setSelectedGate] = useState(null);
    const [dialogOpen, setDialogOpen] = useState(false);
    const [bulkRequestedStatus, setBulkRequestedStatus] = useState("");
    const [expandedGateId, setExpandedGateId] = useState(null);
    const [createDialogOpen, setCreateDialogOpen] = useState(false);
    const [resetDialogOpen, setResetDialogOpen] = useState(false);
    const [newGateData, setNewGateData] = useState({
        location: "",
        latitude: "",
        longitude: "",
        priority: 0,
        status: "CLOSED",
    });

    const [selectedGateIds, setSelectedGateIds] = useState(new Set());

    const filter = filterProp !== undefined ? filterProp : filterLocal;
    const setFilter = setFilterProp || setFilterLocal;

    const handleResetCounter = async () => {
        try {
            await resetDownlinkCounter().unwrap();
            alert("Counter reset successfully.");
            setResetDialogOpen(false);
        } catch (error) {
            console.error("Failed to reset counter:", error);
            alert("Reset failed. You may not have permission.");
        }
    };

    const handleClose = async () => {
        setDialogOpen(false);
    };

    const handleBulkRequestedStatusChange = async () => {
        if (!bulkRequestedStatus) return;

        const promises = filteredGates.map(async (gate) => {
            try {
                await requestGateStatusChange({ gateId: gate.id, workerId, requestedStatus: bulkRequestedStatus }).unwrap();
            } catch (error) {
                console.error(`Fehler beim Aktualisieren von Gate ${gate.id}`, error);
            }
        });

        await Promise.all(promises);
    };

    const filteredGates = gates.filter(gate =>
        (gate.id.toString().includes(search) || (gate.location?.toLowerCase() || '').includes(search.toLowerCase())) &&
        (
            filter === "" ||
            gate.status === filter ||
            (gate.requestedStatus && gate.requestedStatus.toLowerCase().includes(filter.toLowerCase()))
        )
    );

    const sendManualDownlink = async () => {
        const getStatusCode = (requestedStatus) => {
            if (requestedStatus === null || requestedStatus === undefined || requestedStatus === "REQUESTED_NONE") return 2;
            if (requestedStatus === "REQUESTED_OPEN") return 1;
            if (requestedStatus === "REQUESTED_CLOSE") return 0;
            return null;
        };

        const payload = [
            0,
            Math.floor(Date.now() / 1000),
            filteredGates
                .filter(g => getStatusCode(g.requestedStatus) !== null)
                .map(g => [g.id, getStatusCode(g.requestedStatus), g.priority ?? 0])
        ];

        if (payload[2].length === 0) {
            alert("No gates with requested state. Downlink won't be sent.");
            return;
        }

        try {
            await tryIncrementDownlinkCounter().unwrap();
            await sendDownlink(payload).unwrap();
            alert("Downlink sent.");
        } catch (error) {
            console.error("Error sending downlink:", error);
            alert("Failed to send downlink.");
        }
    };

    const isValidFloat = (value) => !isNaN(value) && parseFloat(value) === Number(value);

    const isFormValid = () => {
        return (
            newGateData.location.trim() !== "" &&
            isValidFloat(newGateData.latitude) &&
            isValidFloat(newGateData.longitude) &&
            (newGateData.status === "OPEN" || newGateData.status === "CLOSED")
        );
    };

    function getTimeAgo(timestamp) {
        const date = new Date(timestamp);
        const now = new Date();
        const secondsAgo = Math.floor((now - date) / 1000);

        if (secondsAgo < 60) return `${secondsAgo} seconds ago`;
        const minutes = Math.floor(secondsAgo / 60);
        if (minutes < 60) return `${minutes} minute${minutes !== 1 ? 's' : ''} ago`;
        const hours = Math.floor(minutes / 60);
        if (hours < 24) return `${hours} hour${hours !== 1 ? 's' : ''} ago`;
        const days = Math.floor(hours / 24);
        if (days === 1) return "yesterday";
        if (days < 7) return `${days} days ago`;
        return date.toLocaleDateString();
    }

    const toggleGateSelection = (gateId) => {
        setSelectedGateIds(prev => {
            const next = new Set(prev);
            if (next.has(gateId)) next.delete(gateId);
            else next.add(gateId);
            return next;
        });
    };

    const toggleSelectAll = () => {
        if (selectedGateIds.size === filteredGates.length && filteredGates.length > 0) {
            setSelectedGateIds(new Set());
        } else {
            setSelectedGateIds(new Set(filteredGates.map(g => g.id)));
        }
    };

    if (gatesLoading || activitiesLoading) {
        return (
            <div className="card" style={{ padding: '48px', textAlign: 'center', color: 'var(--text-secondary)' }}>
                Loading gates…
            </div>
        );
    }

    if (gatesError || activitiesError) {
        return (
            <div className="card" style={{ padding: '16px' }}>
                <div style={{ padding: '16px', background: 'rgba(239,68,68,0.1)', borderRadius: '8px', color: 'var(--red-600)' }}>
                    {gatesError ? 'Failed to load gates data. ' : ''}
                    {activitiesError ? 'Failed to load activities data. ' : ''}
                </div>
            </div>
        );
    }

    return (
        <div className="card">
            <div className="table-toolbar">
                <h2 style={{ fontSize: '16px', fontWeight: 600, margin: 0, marginRight: '12px' }}>Flood Gates</h2>

                <input
                    className="search-input table-search"
                    aria-label="Search gates"
                    placeholder="Search gates…"
                    value={search}
                    onChange={(e) => setSearch(e.target.value)}
                />

                <div className="filter-tabs">
                    {FILTER_TABS.map(tab => (
                        <button
                            key={tab.value}
                            className={`filter-tab${filter === tab.value ? ' active' : ''}`}
                            data-filter={tab.value}
                            onClick={() => setFilter(tab.value)}
                        >
                            {tab.label}
                        </button>
                    ))}
                </div>

                <div style={{ flex: 1 }} />
            </div>

            <>
                    <div className={`bulk-bar${filteredGates.length > 0 ? ' visible' : ''}`}>
                        <select
                            className="form-input"
                            value={bulkRequestedStatus}
                            onChange={(e) => setBulkRequestedStatus(e.target.value)}
                            style={{ width: 'auto', minWidth: '160px' }}
                        >
                            <option value="">None</option>
                            <option value="REQUESTED_OPEN">Request Open</option>
                            <option value="REQUESTED_CLOSE">Request Close</option>
                            <option value="REQUESTED_NONE">Clear All Requests</option>
                        </select>

                        <button
                            className="btn btn-primary"
                            onClick={handleBulkRequestedStatusChange}
                            disabled={!bulkRequestedStatus}
                        >
                            Apply to Filtered
                        </button>

                        <button
                            className="btn btn-primary"
                            onClick={sendManualDownlink}
                            disabled={downlinkCount >= 10}
                            title={downlinkCount >= 10
                                ? "Downlink limit reached (10). Please wait or reset."
                                : `You have sent ${downlinkCount} downlinks. You can send up to 10 downlinks.`}
                        >
                            Send Downlink
                        </button>

                        <button
                            className="btn btn-danger"
                            disabled={userDetails?.role !== 'controller'}
                            onClick={() => setResetDialogOpen(true)}
                        >
                            Reset Downlink Counter
                        </button>

                        <button
                            className="btn btn-ghost"
                            onClick={() => setCreateDialogOpen(true)}
                        >
                            Create Gate
                        </button>

                        {selectedGateIds.size > 0 && (
                            <span className="bulk-count">{selectedGateIds.size} selected</span>
                        )}
                    </div>

                    <table className="gate-table">
                        <thead>
                            <tr>
                                <th>
                                    <input
                                        type="checkbox"
                                        className="select-all"
                                        aria-label="Select all gates"
                                        checked={filteredGates.length > 0 && selectedGateIds.size === filteredGates.length}
                                        onChange={toggleSelectAll}
                                    />
                                </th>
                                <th>Gate ID</th>
                                <th>Location</th>
                                <th>Status</th>
                                <th>Confidence</th>
                                <th>Last Update</th>
                                <th>Actions</th>
                                <th>Activities</th>
                            </tr>
                        </thead>
                        <tbody>
                            {filteredGates.map((gate) => {
                                const si = statusInfo(gate.status);
                                const ci = confirmationIndicator(gate.stateConfirmation);

                                return (
                                    <React.Fragment key={gate.id}>
                                        <tr onClick={() => navigate(`/gates/${gate.id}`)}>
                                            <td onClick={(e) => e.stopPropagation()}>
                                                <input
                                                    type="checkbox"
                                                    className="row-check"
                                                    aria-label={`Select gate ${gate.id}`}
                                                    checked={selectedGateIds.has(gate.id)}
                                                    onChange={() => toggleGateSelection(gate.id)}
                                                />
                                            </td>
                                            <td data-label="Gate ID">
                                                <span
                                                    className="gate-id"
                                                    style={{ cursor: 'pointer' }}
                                                    onClick={(e) => {
                                                        e.stopPropagation();
                                                        navigate(`/gates/${gate.id}`);
                                                    }}
                                                >
                                                    G-{gate.id}
                                                </span>
                                            </td>
                                            <td data-label="Location">
                                                <span
                                                    style={{ cursor: 'pointer', color: 'var(--accent)' }}
                                                    title="Click to view on map"
                                                    onClick={(e) => {
                                                        e.stopPropagation();
                                                        navigate('/map');
                                                    }}
                                                >
                                                    {gate.location}
                                                </span>
                                                <br />
                                                <span className="coords" style={{ fontSize: '12px', color: 'var(--text-secondary)' }}>
                                                    {gate.latitude != null ? gate.latitude.toFixed(5) : '—'}, {gate.longitude != null ? gate.longitude.toFixed(5) : '—'}
                                                </span>
                                            </td>
                                            <td data-label="Status">
                                                <span style={{ position: 'relative', display: 'inline-flex' }}>
                                                    <span className={`status-badge ${si.cls}`}>
                                                        <span className="status-dot" />
                                                        {si.label}
                                                    </span>
                                                    {ci && (
                                                        <span style={{
                                                            position: 'absolute',
                                                            top: '-4px',
                                                            right: '-4px',
                                                            width: '14px',
                                                            height: '14px',
                                                            borderRadius: '50%',
                                                            background: ci.color,
                                                            color: '#fff',
                                                            fontSize: '8px',
                                                            display: 'flex',
                                                            alignItems: 'center',
                                                            justifyContent: 'center',
                                                            fontWeight: 700,
                                                            lineHeight: 1,
                                                        }}>
                                                            {ci.label}
                                                        </span>
                                                    )}
                                                </span>
                                            </td>
                                            <td data-label="Confidence">
                                                <ConfidenceCell gate={gate} />
                                            </td>
                                            <td data-label="Last Update">
                                                {gate.lastTimeStamp ? (
                                                    <span
                                                        className="last-update"
                                                        title={new Date(gate.lastTimeStamp).toLocaleString()}
                                                    >
                                                        {getTimeAgo(gate.lastTimeStamp)}
                                                    </span>
                                                ) : '—'}
                                            </td>
                                            <td data-label="Actions" onClick={(e) => e.stopPropagation()}>
                                                <button
                                                    type="button"
                                                    className="action-link"
                                                    onClick={(e) => {
                                                        e.stopPropagation();
                                                        setSelectedGate(gate);
                                                        setDialogOpen(true);
                                                    }}
                                                >
                                                    Request Change
                                                </button>
                                            </td>
                                            <td data-label="Activities" onClick={(e) => e.stopPropagation()}>
                                                <button
                                                    className="action-link"
                                                    aria-label="expand row"
                                                    onClick={(e) => {
                                                        e.stopPropagation();
                                                        setExpandedGateId(expandedGateId === gate.id ? null : gate.id);
                                                    }}
                                                >
                                                    {expandedGateId === gate.id ? '▼' : '▶'} Activities
                                                </button>
                                            </td>
                                        </tr>
                                        {expandedGateId === gate.id && (
                                            <tr className="expanded-row" onClick={() => navigate(`/gates/${gate.id}`)}>
                                                <td colSpan={8}>
                                                    <div>
                                                        <strong>Activities</strong>
                                                        {activities
                                                            .filter(activity => activity.gateId === gate.id)
                                                            .slice(-4)
                                                            .map(activity => (
                                                                <p key={activity.id}>
                                                                    <strong>{activity.lastTimeStamp ? new Date(activity.lastTimeStamp).toLocaleString() : '—'}:</strong> {activity.message}
                                                                </p>
                                                            ))
                                                        }
                                                        {activities.filter(a => a.gateId === gate.id).length === 0 && (
                                                            <p>No activities available for this gate.</p>
                                                        )}
                                                    </div>
                                                </td>
                                            </tr>
                                        )}
                                    </React.Fragment>
                                );
                            })}
                        </tbody>
                    </table>
            </>

            <StatusChangedDialog
                open={dialogOpen}
                gate={selectedGate}
                onClose={() => handleClose()}
            />

            <Dialog open={createDialogOpen} onClose={() => setCreateDialogOpen(false)}>
                <DialogTitle>Create New Gate</DialogTitle>
                <DialogContent>
                    <div className="form-group">
                        <label className="form-label" htmlFor="create-gate-location">Location</label>
                        <input
                            id="create-gate-location"
                            className="form-input"
                            required
                            value={newGateData.location}
                            onChange={(e) => setNewGateData({ ...newGateData, location: e.target.value })}
                        />
                    </div>
                    <div className="form-group">
                        <label className="form-label" htmlFor="create-gate-latitude">Latitude</label>
                        <input
                            id="create-gate-latitude"
                            className="form-input"
                            required
                            type="number"
                            step="any"
                            value={newGateData.latitude}
                            onChange={(e) => setNewGateData({ ...newGateData, latitude: e.target.value })}
                        />
                    </div>
                    <div className="form-group">
                        <label className="form-label" htmlFor="create-gate-longitude">Longitude</label>
                        <input
                            id="create-gate-longitude"
                            className="form-input"
                            required
                            type="number"
                            step="any"
                            value={newGateData.longitude}
                            onChange={(e) => setNewGateData({ ...newGateData, longitude: e.target.value })}
                        />
                    </div>
                    <div className="form-group">
                        <label className="form-label" htmlFor="create-gate-priority">Priority</label>
                        <input
                            id="create-gate-priority"
                            className="form-input"
                            type="number"
                            min={0}
                            max={3}
                            value={newGateData.priority}
                            onChange={(e) => setNewGateData({ ...newGateData, priority: parseInt(e.target.value) })}
                        />
                    </div>
                    <div className="form-group">
                        <label className="form-label" htmlFor="create-gate-status">Status</label>
                        <select
                            id="create-gate-status"
                            className="form-input"
                            required
                            value={newGateData.status}
                            onChange={(e) => setNewGateData({ ...newGateData, status: e.target.value })}
                        >
                            <option value="OPEN">OPEN</option>
                            <option value="CLOSED">CLOSED</option>
                        </select>
                    </div>
                </DialogContent>
                <DialogActions>
                    <button className="btn btn-ghost" onClick={() => setCreateDialogOpen(false)}>Cancel</button>
                    <button
                        className="btn btn-primary"
                        disabled={!isFormValid()}
                        onClick={async () => {
                            try {
                                await createGate(newGateData).unwrap();
                                setCreateDialogOpen(false);
                                setNewGateData({ location: "", latitude: "", longitude: "", priority: 0, status: "CLOSED" });
                            } catch (error) {
                                console.error("Fehler beim Erstellen:", error);
                                alert("Fehler beim Erstellen des Gates.");
                            }
                        }}
                    >
                        Create
                    </button>
                </DialogActions>
            </Dialog>

            <Dialog open={resetDialogOpen} onClose={() => setResetDialogOpen(false)}>
                <DialogTitle>Reset Downlink Counter</DialogTitle>
                <DialogContent>
                    <p>Are you sure you want to reset the downlink counter?</p>
                </DialogContent>
                <DialogActions>
                    <button className="btn btn-ghost" onClick={() => setResetDialogOpen(false)}>Cancel</button>
                    <button className="btn btn-danger" onClick={handleResetCounter}>Reset</button>
                </DialogActions>
            </Dialog>
        </div>
    );
}

export default StatusTables;
