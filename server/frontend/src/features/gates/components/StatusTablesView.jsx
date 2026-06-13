import React, { useState } from "react";
import { useGetGatesQuery, useGetActivitiesQuery } from "../../../app/store/api/api";
import { MapView } from "../../map";

/* ── helpers ── */

const statusInfo = (status) => {
    switch (status) {
        case "OPEN": return { cls: "status-open", label: "Open" };
        case "CLOSED": return { cls: "status-closed", label: "Closed" };
        default: return { cls: "status-oos", label: "Out of Service" };
    }
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

/* ── filter‑tab definitions ── */

const FILTER_TABS = [
    { label: "All", value: "" },
    { label: "Open", value: "OPEN" },
    { label: "Closed", value: "CLOSED" },
    { label: "OOS", value: "OUT_OF_SERVICE" },
];

function StatusTablesView() {
    const [search, setSearch] = useState("");
    const [filter, setFilter] = useState("");
    const [view, setView] = useState("list");
    const [expandedGateId, setExpandedGateId] = useState(null);

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
                <div style={{ padding: '16px', background: 'var(--red-100)', borderRadius: '8px', color: 'var(--red-600)' }}>
                    Failed to load gates data: {error.toString()}
                </div>
            </div>
        );
    }

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
                <table className="gate-table">
                    <thead>
                        <tr>
                            <th>Gate ID</th>
                            <th>Location</th>
                            <th>Status</th>
                            <th>Last Update</th>
                            <th>Confidence</th>
                            <th>Activities</th>
                        </tr>
                    </thead>
                    <tbody>
                        {filteredGates.map((gate) => {
                            const si = statusInfo(gate.status);

                            return (
                                <React.Fragment key={gate.id}>
                                    <tr>
                                        <td data-label="Gate ID">
                                            <span className="gate-id">G-{gate.id}</span>
                                        </td>
                                        <td data-label="Location">
                                            {gate.location}<br />
                                            <span className="coords" style={{ fontSize: '12px', color: 'var(--text-secondary)' }}>
                                                {gate.latitude}, {gate.longitude}
                                            </span>
                                        </td>
                                        <td data-label="Status">
                                            <span className={`status-badge ${si.cls}`}>
                                                <span className="status-dot" />
                                                {si.label}
                                            </span>
                                        </td>
                                        <td data-label="Last Update">
                                            <span className="last-update">{getTimeAgo(gate.lastTimeStamp)}</span>
                                            <div style={{ fontSize: '12px', color: 'var(--text-secondary)' }}>
                                                {new Date(gate.lastTimeStamp).toLocaleString()}
                                            </div>
                                        </td>
                                        <td data-label="Confidence">
                                            100%
                                            <span
                                                title="Confidence reflects agreement between sensor and worker. 100% means both match."
                                                style={{ marginLeft: 4, cursor: "help", verticalAlign: "middle", color: "var(--text-secondary)" }}
                                            >
                                                &#9432;
                                            </span>
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
                                    </tr>

                                    {expandedGateId === gate.id && (
                                        <tr className="expanded-row">
                                            <td colSpan={6}>
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
            ) : (
                <MapView search={search} statusFilter={filter} />
            )}
        </div>
    );
}

export default StatusTablesView;
