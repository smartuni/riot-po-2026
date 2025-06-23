import React, {useEffect, useState} from "react";
import {fetchActivities, fetchGates, requestGateStatusChange} from "../services/api";
import axios from "axios";
import api from "../services/api";
import {
    TextField,
    MenuItem,
    IconButton,
    Tabs,
    Tab,
    Button,
    Select,
    FormControl,
    InputLabel,
    Box, Tooltip,
} from "@mui/material";
import LockOpenIcon from '@mui/icons-material/LockOpen';
import LockIcon from '@mui/icons-material/Lock';
import CircleIcon from '@mui/icons-material/Circle';
import SyncAltIcon from '@mui/icons-material/SyncAlt';
import MapView from "../components/MapView";
import StatusChangedDialog from "../components/StatusChangedDialog";
import HelpOutlineIcon from '@mui/icons-material/HelpOutline';

function StatusTables() {
    const [gates, setGates] = useState([]);
    const [search, setSearch] = useState("");
    const [filter, setFilter] = useState("");
    const [view, setView] = useState("list");
    const [selectedGate, setSelectedGate] = useState(null);
    const [dialogOpen, setDialogOpen] = useState(false);
    const [bulkRequestedStatus, setBulkRequestedStatus] = useState("");
    const [expandedGateId, setExpandedGateId] = useState(null);
    const [activities, setActivities] = useState([]);


    useEffect(() => {
        const loadGates = async () => {
            try {
                const data = await fetchGates();
                setGates(data);
            } catch (error) {
                console.error('Fehler beim Laden der Gates', error);
            }
        };
        loadGates();
    }, []);

    useEffect(() => {
        const loadActivities = async () => {
            try {
                const data = await fetchActivities();
                setActivities(data);
            } catch (error) {
                console.error('Fehler beim Laden der Aktivitäten', error);
            }
        };
        loadActivities();
    }, []);

    // Funktion zum Abrufen der Gates
    const handleClose = async () => {
        setDialogOpen(false);
        const updated = await fetchGates();
        setGates(updated);
    };

    // Funktion zum Ändern des angeforderten Status für mehrere Gates
    const handleBulkRequestedStatusChange = async () => {
        if (!bulkRequestedStatus) return;

        const statusToSend = bulkRequestedStatus === "NONE" ? null : bulkRequestedStatus;

        const gatesToUpdate = filteredGates.filter(gate => {
            const currentRequested = gate.requestedStatus || null;
            const currentStatus = gate.status;

            // Gleicher Request? Ignorieren
            if (currentRequested === statusToSend) return false;

            // Status passt schon zum Ziel? Ignorieren
            return !((statusToSend === "REQUESTED_OPEN" && currentStatus === "OPENED") ||
                (statusToSend === "REQUESTED_CLOSE" && currentStatus === "CLOSED"));


        });

        const promises = gatesToUpdate.map(async (gate) => {
            try {
                await requestGateStatusChange(gate.id, statusToSend);
            } catch (error) {
                console.error(`Fehler beim Aktualisieren von Gate ${gate.id}`, error);
            }
        });

        await Promise.all(promises);
        const updated = await fetchGates();
        setGates(updated);
    };

    // Funktion zum Rendern des angeforderten Status
    const renderRequestedStatus = (status) => {
        switch (status) {
            case "REQUESTED_OPEN":
                return <><LockOpenIcon fontSize="small"/> OPEN</>;
            case "REQUESTED_CLOSE":
                return <><LockIcon fontSize="small"/> CLOSE</>;
            default:
                return <><CircleIcon fontSize="small"/> NONE</>;
        }
    };

    // Filtere die Gates basierend auf der Suche und dem Statusfilter
    const filteredGates = gates.filter(gate =>
        (gate.id.toString().includes(search) || gate.location.toLowerCase().includes(search.toLowerCase())) &&
        (
            filter === "" ||
            gate.status === filter ||
            (gate.requestedStatus && gate.requestedStatus.toLowerCase().includes(filter.toLowerCase()))
        )
    );

    const sendManualDownlink = async () => {
        const statusIntMap = {
            "REQUESTED_OPEN": 1,
            "REQUESTED_CLOSE": 0
        };

        const payload = [
            1, // Type
            Math.floor(Date.now() / 1000), // Unix Timestamp
            filteredGates
                .filter(g => g.requestedStatus in statusIntMap)
                .map(g => [g.id, statusIntMap[g.requestedStatus]])
        ];

        if (payload[2].length === 0) {
            alert("No gates with requested OPEN or CLOSE status.");
            return;
        }

        try {
            const response = await api.post("api/downlink", JSON.stringify(payload));
            alert("Downlink sent.");
        } catch (error) {
            console.error("Error sending downlink:", error);
            alert("Failed to send downlink.");
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
        return date.toLocaleDateString(); // fallback to a readable date
    }


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
                        style={{marginRight: "1rem"}}
                    />
                    <TextField
                        size="small"
                        select
                        value={filter}
                        onChange={(e) => setFilter(e.target.value)}
                    >
                        <MenuItem value="">All Status</MenuItem>
                        <MenuItem value="OPENED">Open</MenuItem>
                        <MenuItem value="CLOSED">Closed</MenuItem>
                        <MenuItem value="REQUESTED_CLOSE">Requested Close</MenuItem>
                        <MenuItem value="REQUESTED_OPEN">Requested Open</MenuItem>
                        <MenuItem value="REQUESTED_NONE">No Requested Status</MenuItem>
                    </TextField>
                </div>
            </div>

            <Tabs
                value={view}
                onChange={(e, newValue) => setView(newValue)}
                style={{marginTop: "1rem", marginBottom: "1rem"}}
            >
                <Tab label="List View" value="list"/>
                <Tab label="Map View" value="map"/>
            </Tabs>

            {view === "list" ? (
                <>
                    <Box display="flex" alignItems="center" gap={2} mb={2}>
                        {/* Bulk Select */}
                        <FormControl size="small">
                            <InputLabel>Bulk Requested Status</InputLabel>
                            <Select
                                value={bulkRequestedStatus}
                                label="Bulk Requested Status"
                                onChange={(e) => setBulkRequestedStatus(e.target.value)}
                                style={{minWidth: 160}}
                            >
                                <MenuItem value="">None</MenuItem>
                                <MenuItem value="REQUESTED_OPEN">Request Open</MenuItem>
                                <MenuItem value="REQUESTED_CLOSE">Request Close</MenuItem>
                                <MenuItem value="REQUESTED_NONE">Clear Request</MenuItem>
                            </Select>
                        </FormControl>

                        <Button
                            variant="contained"
                            color="primary"
                            onClick={handleBulkRequestedStatusChange}
                            disabled={!bulkRequestedStatus}
                        >
                            Apply to Filtered
                        </Button>

                        <Button
                            variant="contained"
                            color="secondary"
                            onClick={sendManualDownlink}
                        >
                            Send Downlink
                        </Button>
                    </Box>

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
                            <React.Fragment key={gate.id}>
                                <tr
                                    onClick={() => setExpandedGateId(expandedGateId === gate.id ? null : gate.id)}
                                    style={{cursor: "pointer"}}
                                >
                                    <td>{gate.id}</td>
                                    <td>
                                        {gate.location}<br/>
                                        <span className="coords">{gate.latitude}, {gate.longitude}</span>
                                    </td>
                                    <td>
                                        <span className={`badge ${gate.status.toLowerCase()}`}>
                                            {gate.status === "OPENED"
                                                ? <LockOpenIcon fontSize="small"/>
                                                : <LockIcon fontSize="small"/>
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
                                        <div>{getTimeAgo(gate.lastTimeStamp)}</div>
                                        <div className="date">{gate.lastUpdate}</div>
                                    </td>
                                    <td>
                                        {gate.confidence}
                                        <br/>
                                        <Tooltip
                                            title="Confidence reflects agreement between sensor and worker. 100% means both match.">
                                            <HelpOutlineIcon
                                                fontSize="small"
                                                className="help-icon"
                                                style={{
                                                    marginLeft: 4,
                                                    cursor: "help",
                                                    verticalAlign: "middle",
                                                    color: "#888"
                                                }}
                                            />
                                        </Tooltip>
                                    </td>
                                    <td>
                                        <IconButton
                                            color="warning"
                                            size="small"
                                            onClick={(e) => {
                                                e.stopPropagation();  // verhindert dass Zeilen-Click auch getriggert wird
                                                setSelectedGate(gate);
                                                setDialogOpen(true);
                                            }}
                                        >
                                            <SyncAltIcon/>
                                        </IconButton>
                                    </td>
                                </tr>
                                {expandedGateId === gate.id && (
                                    <tr className="expanded-row">
                                        <td colSpan={8} style={{ backgroundColor: "#f9f9f9" }}>
                                            <div>
                                                <strong>Activities</strong>
                                                {activities
                                                    .filter(activity => activity.gateId === gate.id)
                                                    .slice(-4) // Optional: nur die letzten 4 zeigen
                                                    .map((activity, index) => (
                                                        <p key={activity.id}>
                                                            <strong>{activity.lastTimeStamp}:</strong> {activity.message}
                                                        </p>
                                                    ))}
                                                {activities.filter(a => a.gateId === gate.id).length === 0 && (
                                                    <p>No activities available for this gate.</p>
                                                )}
                                            </div>
                                        </td>
                                    </tr>
                                )}
                            </React.Fragment>
                        ))}
                        </tbody>
                    </table>
                </>
            ) : (
                <MapView search={search} statusFilter={filter}/>
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
