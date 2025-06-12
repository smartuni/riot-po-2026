import React, { useEffect, useState } from "react";
import { fetchGates, requestGateStatusChange, updateGate } from "../services/api";
import { TextField, MenuItem, IconButton, Tabs, Tab } from "@mui/material";
import RefreshIcon from '@mui/icons-material/Sync';
import DeleteIcon from '@mui/icons-material/Delete';
import LockOpenIcon from '@mui/icons-material/LockOpen';
import LockIcon from '@mui/icons-material/Lock';
import CircleIcon from '@mui/icons-material/Circle';
import SyncAltIcon from '@mui/icons-material/SyncAlt';
import MapView from "../components/MapView"; // Wichtig: Importiere deine MapView-Komponente
import StatusChangedDialog from "../components/StatusChangedDialog";

function StatusTables() {
    const [gates, setGates] = useState([]);
    const [search, setSearch] = useState("");
    const [filter, setFilter] = useState("");
    const [view, setView] = useState("list");
    const [selectedGate, setSelectedGate] = useState(null);
    const [dialogOpen, setDialogOpen] = useState(false);

    useEffect(() => {
        const loadGates = async () => {
            try {
                const data = await fetchGates();
                console.log(data)
                setGates(data);
            } catch (error) {
                console.error('Fehler beim Laden der Gates', error);
            }
        };

        loadGates();
    }, []);

    const handleClose = async () => {
        setDialogOpen(false)
        const updated = await fetchGates(); // Reload data
        console.log(updated)
        setGates(updated);
    }

    const STATUS_CLASSES = {
        open: 'status-open',
        closed: 'status-closed',
        unknown: 'status-unknown',
    };

    const renderRequestedStatus = (status) => {
        switch (status) {
            case "OPEN":
                return <><LockOpenIcon fontSize="small"/> OPEN</>;
            case "CLOSED":
                return <><LockIcon fontSize="small"/> CLOSE</>;
            default:
                return <><CircleIcon fontSize="small"/> NONE</>;
        }
    };

    const normalizeStatus = (status) => {
        if (!status) return 'unknown';
        const s = status.toLowerCase();
        return s === 'close' ? 'closed' : s;
    };

    const filteredGates = gates.filter(gate =>
        (gate.id.toString().includes(search) || gate.location.toLowerCase().includes(search.toLowerCase())) &&
        (filter === "" || gate.status === filter)
    );

    return (
        <div className="gate-status-container">
            <div className="gate-status-header">
                <h2>Flood Gates</h2>
                <div className="gate-controls">
                    <TextField
                        size="small"
                        placeholder="Search gates..."
                        value={search}
                        onChange={(e) => setSearch(e.target.value)}
                        style={{ marginRight: "1rem" }}
                    />
                    <TextField
                        size="small"
                        select
                        value={filter}
                        onChange={(e) => setFilter(e.target.value)}
                    >
                        <MenuItem value="">All Status</MenuItem>
                        <MenuItem value="OPEN">Open</MenuItem>
                        <MenuItem value="CLOSED">Closed</MenuItem>
                    </TextField>
                </div>
            </div>

            <Tabs
                value={view}
                onChange={(e, newValue) => setView(newValue)}
                style={{ marginTop: "1rem", marginBottom: "1rem" }}
            >
                <Tab label="List View" value="list" />
                <Tab label="Map View" value="map" />
            </Tabs>

            {view === "list" ? (
                <table className="status-table">
                    <thead>
                    <tr>
                        <th>Gate ID</th>
                        <th>Location</th>
                        <th>Status</th>
                        <th>Requested Status</th>
                        <th>Device ID</th>
                        <th>Last Update</th>
                        <th>Confidence</th>
                        <th>Actions</th>
                    </tr>
                    </thead>
                    <tbody>
                    {filteredGates.map((gate) => (
                        <tr key={gate.id}>
                            <td>{gate.id}</td>
                            <td>
                                {gate.location}<br/>
                                <span className="coords">{gate.latitude}, {gate.longitude}</span>
                            </td>
                            <td>
                                    <span className={`badge ${gate.status.toLowerCase()}`}>
                                        {gate.status === "OPEN"
                                            ? <LockOpenIcon fontSize="small" />
                                            : <LockIcon fontSize="small" />
                                        } {gate.status}
                                    </span>
                            </td>
                            <td>
                                    <span className={`badge ${gate.requestedStatus ? gate.requestedStatus.toLowerCase() : 'none'}`}>
                                        {renderRequestedStatus(gate.requestedStatus)}
                                    </span>
                            </td>
                            <td>{gate.deviceId}</td>
                            <td>
                                <div>about 1 hour ago</div>
                                <div className="date">{gate.lastUpdate}</div>
                            </td>
                            <td>
                                👤 {gate.workerConfidence ? "Yes" : "No"} <br/>
                                📡 {gate.sensorConfidence ? "Yes" : "No"}
                            </td>
                            <td>
                                <IconButton
                                    color="warning"
                                    size="small"
                                    onClick={() => {
                                        setSelectedGate(gate);
                                        setDialogOpen(true);
                                    }}
                                >
                                    <SyncAltIcon />
                                </IconButton>
                            </td>
                        </tr>
                    ))}
                    </tbody>
                </table>
            ) : (
                <MapView search={search} statusFilter={filter} />
            )}
            <StatusChangedDialog
                open={dialogOpen}
                gate={selectedGate}
                onClose={() => handleClose()}
            />
        </div>
    );
}

export default StatusTables;
