import React, { useState } from "react";
import { useAppSelector } from "../../../app/store";
import {
    useGetGatesQuery,
    useGetActivitiesQuery,
    useGetDownlinkCounterQuery,
    useRequestGateStatusChangeMutation,
    useTryIncrementDownlinkCounterMutation,
    useUpdateGatePriorityMutation,
    useDeleteGateMutation,
    useCreateGateMutation,
    useSendDownlinkMutation,
    useResetDownlinkCounterMutation,
} from "../../../app/store/api/api";
import {
    Select,
    MenuItem,
    Dialog,
    DialogTitle,
    DialogContent,
    DialogActions,
} from "@mui/material";
import { MapView } from "../../map";
import StatusChangedDialog from "./StatusChangedDialog";

/* ── helpers: map domain values → Hydro‑Blue CSS classes ── */

const priorityClass = (level) => {
    switch (level) {
        case 0: return "priority-low";
        case 1: return "priority-medium";
        case 2: return "priority-high";
        case 3: return "priority-critical";
        default: return "priority-low";
    }
};

const statusInfo = (status) => {
    switch (status) {
        case "OPEN": return { cls: "status-open", label: "Open" };
        case "CLOSED": return { cls: "status-closed", label: "Closed" };
        default: return { cls: "status-oos", label: "Out of Service" };
    }
};

const requestedStatusInfo = (status) => {
    switch (status) {
        case "REQUESTED_OPEN": return { cls: "status-open", label: "Open" };
        case "REQUESTED_CLOSE": return { cls: "status-closed", label: "Close" };
        default: return { cls: "status-oos", label: "None" };
    }
};

const pendingJobInfo = (status) => {
    switch (status) {
        case "PENDING_OPEN": return { cls: "status-open", label: "Open" };
        case "PENDING_CLOSE": return { cls: "status-closed", label: "Close" };
        default: return { cls: "status-oos", label: "None" };
    }
};

const confirmationIndicator = (stateConfirmation) => {
    switch (stateConfirmation) {
        case "WORKER_CONFLICT": return { label: "⚠", color: "var(--red-600)" };
        case "UNCONFIRMED": return null;
        case "WORKER_CONFIRMED_SINGLE": return { label: "✓", color: "var(--blue-600)" };
        case "WORKER_CONFIRMED_MULTI":
        case "WORKER_CONFIRMED_ALL": return { label: "✓✓", color: "var(--green-600)" };
        default: return { label: "●", color: "var(--slate-400)" };
    }
};

/* ── filter‑tab definitions (maps tab → filter value) ── */

const FILTER_TABS = [
    { label: "All", value: "" },
    { label: "Open", value: "OPEN" },
    { label: "Closed", value: "CLOSED" },
    { label: "Req Open", value: "REQUESTED_OPEN" },
    { label: "Req Close", value: "REQUESTED_CLOSE" },
    { label: "No Request", value: "REQUESTED_NONE" },
];

/* ══════════════════════════════════════════════════════════ */

function StatusTables() {
    /* ── RTK Query hooks (unchanged) ── */
    const { data: gates = [], isLoading: gatesLoading, error: gatesError } = useGetGatesQuery();
    const { data: activities = [], isLoading: activitiesLoading, error: activitiesError } = useGetActivitiesQuery();
    const { data: downlinkCounterData } = useGetDownlinkCounterQuery();
    const userDetails = useAppSelector((state) => state.auth.user);

    const downlinkCount = downlinkCounterData ?? 0;
    const workerId = userDetails?.workerId ?? null;

    const [requestGateStatusChange] = useRequestGateStatusChangeMutation();
    const [tryIncrementDownlinkCounter] = useTryIncrementDownlinkCounterMutation();
    const [updateGatePriority] = useUpdateGatePriorityMutation();
    const [deleteGate] = useDeleteGateMutation();
    const [createGate] = useCreateGateMutation();
    const [sendDownlink] = useSendDownlinkMutation();
    const [resetDownlinkCounter] = useResetDownlinkCounterMutation();

    /* ── state (unchanged) ── */
    const [search, setSearch] = useState("");
    const [filter, setFilter] = useState("");
    const [view, setView] = useState("list");
    const [selectedGate, setSelectedGate] = useState(null);
    const [dialogOpen, setDialogOpen] = useState(false);
    const [bulkRequestedStatus, setBulkRequestedStatus] = useState("");
    const [expandedGateId, setExpandedGateId] = useState(null);
    const [gateToDelete, setGateToDelete] = useState(null);
    const [deleteDialogOpen, setDeleteDialogOpen] = useState(false);
    const [createDialogOpen, setCreateDialogOpen] = useState(false);
    const [resetDialogOpen, setResetDialogOpen] = useState(false);
    const [newGateData, setNewGateData] = useState({
        location: "",
        latitude: "",
        longitude: "",
        priority: 0,
        status: "CLOSED",
    });

    /* ── checkbox selection state (new UI state) ── */
    const [selectedGateIds, setSelectedGateIds] = useState(new Set());

    /* ── business‑logic handlers (unchanged) ── */

    const handleDeleteGate = async () => {
        try {
            await deleteGate(gateToDelete.id).unwrap();
            setDeleteDialogOpen(false);
            setGateToDelete(null);
        } catch (error) {
            console.error("Fehler beim Löschen des Gates:", error);
            alert("Fehler beim Löschen des Gates.");
        }
    };

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
        const statusIntMap = {
            null: 2,
            "REQUESTED_OPEN": 1,
            "REQUESTED_CLOSE": 0
        };

        const payload = [
            0,
            Math.floor(Date.now() / 1000),
            filteredGates
                .filter(g => g.requestedStatus in statusIntMap)
                .map(g => [g.id, statusIntMap[g.requestedStatus], g.priority ?? 0])
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

    const handlePriorityChange = async (gateId, newPriority) => {
        try {
            await updateGatePriority({ gateId, priority: newPriority }).unwrap();
        } catch (error) {
            console.error("Fehler beim Aktualisieren der Priorität:", error);
            alert("Fehler beim Aktualisieren der Priorität.");
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

    /* ── checkbox helpers (new UI, not business logic) ── */

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

    /* ── loading / error states (plain HTML) ── */

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
                <div style={{ padding: '16px', background: 'var(--red-100)', borderRadius: '8px', color: 'var(--red-600)' }}>
                    {gatesError ? 'Failed to load gates data. ' : ''}
                    {activitiesError ? 'Failed to load activities data. ' : ''}
                </div>
            </div>
        );
    }

    /* ── main render ── */

    return (
        <div className="card">
            {/* ── toolbar: title, search, filter tabs, view toggle ── */}
            <div className="table-toolbar">
                <h2 style={{ fontSize: '16px', fontWeight: 600, margin: 0, marginRight: '12px' }}>Flood Gates</h2>

                <input
                    className="search-input table-search"
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

                <div className="filter-tabs" role="tablist">
                    <button
                        role="tab"
                        aria-selected={view === 'list'}
                        className={`filter-tab${view === 'list' ? ' active' : ''}`}
                        onClick={() => setView("list")}
                    >
                        List View
                    </button>
                    <button
                        role="tab"
                        aria-selected={view === 'map'}
                        className={`filter-tab${view === 'map' ? ' active' : ''}`}
                        onClick={() => setView("map")}
                    >
                        Map View
                    </button>
                </div>
            </div>

            {view === "list" ? (
                <>
                    {/* ── bulk‑action bar ── */}
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

                    {/* ── gate table ── */}
                    <table className="gate-table">
                        <thead>
                            <tr>
                                <th>
                                    <input
                                        type="checkbox"
                                        className="select-all"
                                        checked={filteredGates.length > 0 && selectedGateIds.size === filteredGates.length}
                                        onChange={toggleSelectAll}
                                    />
                                </th>
                                <th>Gate ID</th>
                                <th>Location</th>
                                <th>Status</th>
                                <th>Requested Status</th>
                                <th>Pending Jobs</th>
                                <th>Priority</th>
                                <th>Last Update</th>
                                <th>Actions</th>
                                <th>Activities</th>
                                <th>Delete</th>
                            </tr>
                        </thead>
                        <tbody>
                            {filteredGates.map((gate) => {
                                const si = statusInfo(gate.status);
                                const ci = confirmationIndicator(gate.stateConfirmation);
                                const rsi = requestedStatusInfo(gate.requestedStatus);
                                const pji = pendingJobInfo(gate.pendingJob);

                                return (
                                    <React.Fragment key={gate.id}>
                                        <tr>
                                            <td>
                                                <input
                                                    type="checkbox"
                                                    className="row-check"
                                                    checked={selectedGateIds.has(gate.id)}
                                                    onChange={() => toggleGateSelection(gate.id)}
                                                />
                                            </td>
                                            <td data-label="Gate ID">
                                                <span className="gate-id">G-{gate.id}</span>
                                            </td>
                                            <td data-label="Location">
                                                {gate.location}<br />
                                                <span className="coords" style={{ fontSize: '12px', color: 'var(--text-secondary)' }}>
                                                    {gate.latitude.toFixed(5)}, {gate.longitude.toFixed(5)}
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
                                            <td data-label="Requested Status">
                                                <span className={`status-badge ${rsi.cls}`}>
                                                    <span className="status-dot" />
                                                    {rsi.label}
                                                </span>
                                            </td>
                                            <td data-label="Pending Jobs">
                                                <span className={`status-badge ${pji.cls}`}>
                                                    <span className="status-dot" />
                                                    {pji.label}
                                                </span>
                                            </td>
                                            <td data-label="Priority">
                                                <span className={priorityClass(gate.priority ?? 0)} style={{ display: 'inline-flex', alignItems: 'center', marginRight: '4px' }}>
                                                    <span className="priority-dot" />
                                                    {gate.priority ?? 0}
                                                </span>
                                                <Select
                                                    value={gate.priority ?? 0}
                                                    onChange={(e) => {
                                                        const newPriority = parseInt(e.target.value);
                                                        handlePriorityChange(gate.id, newPriority);
                                                    }}
                                                    variant="outlined"
                                                    size="small"
                                                    style={{ minWidth: 60, verticalAlign: 'middle' }}
                                                >
                                                    {[0, 1, 2, 3].map((level) => (
                                                        <MenuItem key={level} value={level}>
                                                            {level}
                                                        </MenuItem>
                                                    ))}
                                                </Select>
                                            </td>
                                            <td data-label="Last Update">
                                                <span className="last-update">{getTimeAgo(gate.lastTimeStamp)}</span>
                                                <div style={{ fontSize: '12px', color: 'var(--text-secondary)' }}>
                                                    {new Date(gate.lastTimeStamp).toLocaleString()}
                                                </div>
                                            </td>
                                            <td data-label="Actions">
                                                <a
                                                    className="action-link"
                                                    onClick={(e) => {
                                                        e.stopPropagation();
                                                        setSelectedGate(gate);
                                                        setDialogOpen(true);
                                                    }}
                                                >
                                                    Request Change
                                                </a>
                                            </td>
                                            <td data-label="Activities">
                                                <button
                                                    className="action-link"
                                                    aria-label="expand row"
                                                    onClick={() =>
                                                        setExpandedGateId(expandedGateId === gate.id ? null : gate.id)
                                                    }
                                                >
                                                    {expandedGateId === gate.id ? '▼' : '▶'} Activities
                                                </button>
                                            </td>
                                            <td data-label="Delete">
                                                <a
                                                    className="action-link"
                                                    style={{ color: 'var(--red-600)' }}
                                                    onClick={() => {
                                                        setGateToDelete(gate);
                                                        setDeleteDialogOpen(true);
                                                    }}
                                                >
                                                    Delete
                                                </a>
                                            </td>
                                        </tr>
                                        {expandedGateId === gate.id && (
                                            <tr className="expanded-row">
                                                <td colSpan={11}>
                                                    <div>
                                                        <strong>Activities</strong>
                                                        {activities
                                                            .filter(activity => activity.gateId === gate.id)
                                                            .slice(-4)
                                                            .map(activity => (
                                                                <p key={activity.id}>
                                                                    <strong>{new Date(activity.lastTimeStamp).toLocaleString()}:</strong> {activity.message}
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
            ) : (
                <MapView search={search} statusFilter={filter} />
            )}

            {/* ── Status‑Change dialog (unchanged component) ── */}
            <StatusChangedDialog
                open={dialogOpen}
                gate={selectedGate}
                onClose={() => handleClose()}
            />

            {/* ── Delete‑Gate dialog (MUI Dialog kept) ── */}
            <Dialog open={deleteDialogOpen} onClose={() => setDeleteDialogOpen(false)}>
                <DialogTitle>Confirm Deletion</DialogTitle>
                <DialogContent>
                    Are you sure you want to delete the gate with the ID: <strong>{gateToDelete?.id}</strong>?
                </DialogContent>
                <DialogActions>
                    <button className="btn btn-ghost" onClick={() => setDeleteDialogOpen(false)}>Cancel</button>
                    <button className="btn btn-danger" onClick={handleDeleteGate}>Delete</button>
                </DialogActions>
            </Dialog>

            {/* ── Create‑Gate dialog (MUI Dialog kept) ── */}
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

            {/* ── Reset‑Counter dialog (MUI Dialog kept) ── */}
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
