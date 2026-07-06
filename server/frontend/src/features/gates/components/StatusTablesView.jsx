import React, { useState } from "react";
import { useNavigate } from "react-router-dom";
import { useGetGatesQuery, useGetActivitiesQuery } from "../../../app/store/api/api";
import ArrowForwardIcon from "@mui/icons-material/ArrowForward";
import DoneAllIcon from "@mui/icons-material/DoneAll";
import WarningIcon from "@mui/icons-material/Warning";

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

    if (sc === "WORKER_CONFIRMED_SINGLE") {
        icon = <ArrowForwardIcon className="confidence-icon" style={{ color: "var(--blue-600)" }} />;
        title = "Confirmed by 1 worker";
    } else if (sc === "WORKER_CONFIRMED_MULTI" || sc === "WORKER_CONFIRMED_ALL") {
        icon = <DoneAllIcon className="confidence-icon" style={{ color: "var(--green-600)" }} />;
        title = "Confirmed by 2+ workers";
    } else if (sc === "WORKER_CONFLICT") {
        icon = <WarningIcon className="confidence-icon" style={{ color: "var(--red-600)" }} />;
        title = "Conflict: workers disagree";
    }

    const confidencePct = gate.confidence != null ? `${gate.confidence}%` : "—";

    return (
        <span className="confidence-indicator" title={title}>
            {icon}
            <span className="confidence-value">{confidencePct}</span>
        </span>
    );
}

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

const FILTER_TABS = [
    { label: "All", value: "" },
    { label: "Open", value: "OPEN" },
    { label: "Closed", value: "CLOSED" },
    { label: "OOS", value: "OUT_OF_SERVICE" },
];

function StatusTablesView({ filter: filterProp, setFilter: setFilterProp }) {
    const navigate = useNavigate();

    const [search, setSearch] = useState("");
    const [filterLocal, setFilterLocal] = useState("");
    const [expandedGateId, setExpandedGateId] = useState(null);

    const filter = filterProp !== undefined ? filterProp : filterLocal;
    const setFilter = setFilterProp || setFilterLocal;

    const { data: gates = [], isLoading, error } = useGetGatesQuery();
    const { data: activities = [] } = useGetActivitiesQuery();

    const filteredGates = gates.filter(gate =>
        (gate.id.toString().includes(search) || (gate.location?.toLowerCase() || '').includes(search.toLowerCase())) &&
        (
            filter === "" ||
            gate.status === filter ||
            (gate.requestedStatus && gate.requestedStatus.toLowerCase().includes(filter.toLowerCase()))
        )
    );

    if (isLoading) {
        return (
            <div className="card" style={{ padding: '48px', textAlign: 'center', color: 'var(--text-secondary)' }}>
                Loading gates…
            </div>
        );
    }

    if (error) {
        return (
            <div className="card" style={{ padding: '16px' }}>
                <div style={{ padding: '16px', background: 'rgba(239,68,68,0.1)', borderRadius: '8px', color: 'var(--red-600)' }}>
                    Failed to load gates data. Please try again later.
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

            <table className="gate-table">
                    <thead>
                        <tr>
                            <th>Gate ID</th>
                            <th>Location</th>
                            <th>Status</th>
                            <th>Confidence</th>
                            <th>Last Update</th>
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
                                            <td colSpan={6}>
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
        </div>
    );
}

export default StatusTablesView;
