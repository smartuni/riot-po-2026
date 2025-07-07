import React, { useEffect, useState } from "react";
import { fetchActivities, fetchGates, loadWorkerId, requestGateStatusChange, updateGatePriority } from "../services/api";
import ExpandMoreIcon from '@mui/icons-material/ExpandMore';
import ExpandLessIcon from '@mui/icons-material/ExpandLess';
import api from "../services/api";
import CloseIcon from "@mui/icons-material/Close";
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
    Box, Tooltip, DialogActions, DialogContent, DialogTitle, Dialog,
} from "@mui/material";
import LockOpenIcon from '@mui/icons-material/LockOpen';
import LockIcon from '@mui/icons-material/Lock';
import CircleIcon from '@mui/icons-material/Circle';
import SyncAltIcon from '@mui/icons-material/SyncAlt';
import MapView from "../components/MapView";
import StatusChangedDialog from "../components/StatusChangedDialog";
import HelpOutlineIcon from '@mui/icons-material/HelpOutline';
import SockJS from 'sockjs-client';
import { Stomp } from '@stomp/stompjs';

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
    const [workerId, setWorkerId] = useState(null);
    const [gateToDelete, setGateToDelete] = useState(null);
    const [deleteDialogOpen, setDeleteDialogOpen] = useState(false);
    const [selectedPriorities, setSelectedPriorities] = useState({});
    const [createDialogOpen, setCreateDialogOpen] = useState(false);
    const [newGateData, setNewGateData] = useState({
        location: "",
        latitude: "",
        longitude: "",
        priority: 0,
        status: "CLOSED", // default value
    });




    useEffect(() => {
        const loadDetails = async () => {
            try {
                const response = await api.get('/auth/user-details');
                if (response.status !== 200) {
                    throw new Error('Request failed with status code ' + response.status);
                }
                setWorkerId(response.data.workerId);
            } catch (e) {
                console.error("Fehler beim Laden der User-Details:", e);
            }
        };

        loadDetails();
    }, []);


    /**
     * Lädt die Gates beim ersten Rendern der Komponente.
     */
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

    /**
     * Initialisiert die WebSocket-Verbindung und abonniert die relevanten Topics.
     */
    useEffect(() => {
        const socket = new SockJS('http://localhost:8080/ws');
        const stompClient = Stomp.over(socket);

        stompClient.connect({}, () => {

            stompClient.subscribe('/topic/gates/add', (message) => {
                const activity = JSON.parse(message.body);
                setGates(prev => [...prev, activity]);
            });

            stompClient.subscribe('/topic/gates/delete', (message) => {
                const id = parseInt(message.body);
                setGates(prev => prev.filter(a => a.id !== id));
            });

            stompClient.subscribe('/topic/gates/updates', (message) => {
                const updatedGate = JSON.parse(message.body);
                setGates(prevGates => {
                    const index = prevGates.findIndex(gate => gate.id === updatedGate.id);
                    if (index !== -1) {
                        // Gate exists: replace it
                        const newGates = [...prevGates];
                        newGates[index] = updatedGate;
                        return newGates;
                    }
                });
            });
            stompClient.subscribe('/topic/gate-activities', (message) => {
                const activity = JSON.parse(message.body);
                setActivities(prev => [...prev, activity]);
            });

            stompClient.subscribe('/topic/gate-activities/delete', (message) => {
                const id = parseInt(message.body);
                setActivities(prev => prev.filter(a => a.id !== id));
            });
        });

        return () => {
            stompClient.disconnect();
        };
    }, []);

    /**
     * For deleting a gate.
     * @returns {Promise<void>}
     */
    const handleDeleteGate = async () => {
        try {
            await api.delete(`/gates/${gateToDelete.id}`);
            setDeleteDialogOpen(false);
            setGateToDelete(null);
            // const updated = await fetchGates();
            // setGates(updated);
        } catch (error) {
            console.error("Fehler beim Löschen des Gates:", error);
            alert("Fehler beim Löschen des Gates.");
        }
    };


    /**
     * Just for the Last Updatet Time of the Gates.
     */
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
        // const intervalId = setInterval(() => {
        //     loadActivities();
        // }, 300);

        // return () => clearInterval(intervalId);
    }, []);




    /**
     * Schließt den Dialog und aktualisiert die Gates.
     * @returns {Promise<void>}
     */
    const handleClose = async () => {
        setDialogOpen(false);
        const updated = await fetchGates();
        setGates(updated);
    };

    /**
     * Verarbeitet die Massenänderung des angeforderten Status für die gefilterten Gates.
     * @returns {Promise<void>}
     */
    const handleBulkRequestedStatusChange = async () => {
        if (!bulkRequestedStatus) return;

        const statusToSend = bulkRequestedStatus === "NONE" ? null : bulkRequestedStatus;

        const promises = filteredGates.map(async (gate) => {
            try {
                await requestGateStatusChange(gate.id, workerId, bulkRequestedStatus);
            } catch (error) {
                console.error(`Fehler beim Aktualisieren von Gate ${gate.id}`, error);
            }
        });

        await Promise.all(promises);
        const updated = await fetchGates();
        setGates(updated);
    };





    /**
     * Rendert den angeforderten Status für ein Gate.
     * @param status
     * @returns {Element}
     */
    const renderRequestedStatus = (status) => {
        switch (status) {
            case "REQUESTED_OPEN":
                return <><LockOpenIcon fontSize="small" /> OPEN</>;
            case "REQUESTED_CLOSE":
                return <><LockIcon fontSize="small" /> CLOSE</>;
            default:
                return <><CircleIcon fontSize="small" /> NONE</>;
        }
    };

    const renderPendingJobs = (status) => {
        switch (status) {
            case "PENDING_OPEN":
                return <><LockOpenIcon fontSize="small" /> OPEN</>;
            case "PENDING_CLOSE":
                return <><LockIcon fontSize="small" /> CLOSE</>;
            default:
                return <><CircleIcon fontSize="small" /> NONE</>;
        }
    };

    /**
     * Filtert die Gates basierend auf der Suchanfrage und dem Statusfilter.
     * @type {*[]}
     */
    const filteredGates = gates.filter(gate =>
        (gate.id.toString().includes(search) || gate.location.toLowerCase().includes(search.toLowerCase())) &&
        (
            filter === "" ||
            gate.status === filter ||
            (gate.requestedStatus && gate.requestedStatus.toLowerCase().includes(filter.toLowerCase()))
        )
    );

    /**
     * Sendet eine manuelle Downlink-Anfrage für die gefilterten Gates.
     * @returns {Promise<void>}
     */
    const sendManualDownlink = async () => {
        const statusIntMap = {
            "REQUESTED_OPEN": 1,
            "REQUESTED_CLOSE": 0
        };

        const payload = [
            0, // Type
            Math.floor(Date.now() / 1000), // Unix Timestamp
            filteredGates
                .filter(g => g.requestedStatus in statusIntMap)
                .map(g => [g.id, statusIntMap[g.requestedStatus], g.priority ?? 0])
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

    const handlePriorityChange = async (gateId, newPriority) => {
        try {
            await updateGatePriority(gateId, newPriority);

            // Update die Anzeige sofort im Frontend
            setGates(prevGates =>
                prevGates.map(g =>
                    g.id === gateId ? { ...g, priority: newPriority } : g
                )
            );

            // Optional: selectedPriorities mitziehen, falls du das brauchst
            setSelectedPriorities(prev => ({
                ...prev,
                [gateId]: newPriority
            }));
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
            (newGateData.status === "OPENED" || newGateData.status === "CLOSED")
        );
    };

    /**
     * Berechnet die Zeit seit dem letzten Update eines Gates in einem lesbaren Format.
     * @param timestamp
     * @returns {string}
     */
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
                        style={{ marginRight: "1rem" }}
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
                style={{ marginTop: "1rem", marginBottom: "1rem" }}
            >
                <Tab label="List View" value="list" />
                <Tab label="Map View" value="map" />
            </Tabs>

            {view === "list" ? (
                <>
                    <Box className={"button-box"} gap={2} mb={2}>
                        {/* Bulk Select */}
                        <FormControl size="small">
                            <InputLabel>Bulk Requested Status</InputLabel>
                            <Select
                                value={bulkRequestedStatus}
                                label="Bulk Requested Status"
                                onChange={(e) => setBulkRequestedStatus(e.target.value)}
                                style={{ minWidth: 160 }}
                            >
                                <MenuItem value="">None</MenuItem>
                                <MenuItem value="REQUESTED_OPEN">Request Open</MenuItem>
                                <MenuItem value="REQUESTED_CLOSE">Request Close</MenuItem>
                                <MenuItem value="REQUESTED_NONE">Clear All Requests</MenuItem>
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
                        <Button
                            variant="contained"
                            color="success"
                            onClick={() => setCreateDialogOpen(true)}
                        >
                            Create Gate
                        </Button>

                    </Box>

                    <table className="status-table">
                        <thead>
                            <tr>
                                <th>Gate ID</th>
                                <th>Location</th>
                                <th>Status</th>
                                <th>Requested Status</th>
                                <th>Pending Jobs</th>
                                <th>Priority</th>
                                <th>Last Update</th>
                                <th>Confidence</th>
                                <th>Actions</th>
                                <th>Activities</th>
                                <th>Delete</th>
                            </tr>
                        </thead>
                        <tbody>
                            {filteredGates.map((gate) => (
                                <React.Fragment key={gate.id}>
                                    <tr>
                                        <td data-label="Gate ID">{gate.id}</td>
                                        <td data-label="Location">
                                            {gate.location}<br />
                                            <span className="coords">{gate.latitude}, {gate.longitude}</span>
                                        </td>
                                        <td data-label="Status">
                                            <span className={`badge ${gate.status.toLowerCase()}`}>
                                                {gate.status === "OPENED"
                                                    ? <LockOpenIcon fontSize="small" />
                                                    : <LockIcon fontSize="small" />
                                                } {gate.status}
                                            </span>
                                        </td>
                                        <td data-label="Requested Status">
                                            <span className={`badge ${gate.requestedStatus ? gate.requestedStatus.toLowerCase() : 'none'}`}>
                                                {renderRequestedStatus(gate.requestedStatus)}
                                            </span>
                                        </td>
                                        <td data-label="Pending Jobs">
                                            <span className={`badge ${gate.pendingJob ? gate.pendingJob.toLowerCase() : 'none'}`}>
                                                {renderPendingJobs(gate.pendingJob)}
                                            </span>
                                        </td>
                                        <td data-label="Priority">
                                            <FormControl size="small" variant="outlined">
                                                <Select
                                                    value={gate.priority ?? 0}
                                                    onChange={(e) => {
                                                        const newPriority = parseInt(e.target.value);
                                                        handlePriorityChange(gate.id, newPriority);
                                                    }}
                                                    variant="outlined">
                                                    {[0, 1, 2, 3].map((level) => (
                                                        <MenuItem key={level} value={level}>
                                                            {level}
                                                        </MenuItem>
                                                    ))}
                                                </Select>
                                            </FormControl>

                                        </td>
                                        <td data-label="Last Update">
                                            <div>{getTimeAgo(gate.lastTimeStamp)}</div>
                                            <div className="date">{new Date(gate.lastTimeStamp).toLocaleString()}</div>
                                        </td>
                                        <td data-label="Confidence">
                                            {gate.confidence}
                                            <br />
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
                                        <td data-label="Actions">
                                            <IconButton
                                                color="warning"
                                                size="small"
                                                onClick={(e) => {
                                                    e.stopPropagation();
                                                    setSelectedGate(gate);
                                                    setDialogOpen(true);
                                                }}
                                            >
                                                <SyncAltIcon />
                                            </IconButton>
                                        </td>
                                        <td data-label="Activities">
                                            <IconButton
                                                onClick={() =>
                                                    setExpandedGateId(expandedGateId === gate.id ? null : gate.id)
                                                }
                                                size="small"
                                                aria-label="expand row"
                                            >
                                                {expandedGateId === gate.id ? <ExpandLessIcon /> : <ExpandMoreIcon />}
                                            </IconButton>
                                        </td>
                                        <td data-label="Delete">
                                            <IconButton
                                                color="error"
                                                size="small"
                                                onClick={() => {
                                                    setGateToDelete(gate);
                                                    setDeleteDialogOpen(true);
                                                }}
                                            >
                                                <CloseIcon />
                                            </IconButton>
                                        </td>
                                    </tr>
                                    {expandedGateId === gate.id && (
                                        <tr className="expanded-row">
                                            <td colSpan={11} style={{ backgroundColor: "#f9f9f9" }}>
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
                            ))}
                        </tbody>
                    </table>
                </>
            ) : (
                <MapView search={search} statusFilter={filter} />
            )}

            <StatusChangedDialog
                open={dialogOpen}
                gate={selectedGate}
                onClose={() => handleClose()}
            />
            <Dialog open={deleteDialogOpen} onClose={() => setDeleteDialogOpen(false)}>
                <DialogTitle>Confirm Deletion</DialogTitle>
                <DialogContent>
                    Are you sure you want to delete the gate with the ID: <strong>{gateToDelete?.id}</strong>?
                </DialogContent>
                <DialogActions>
                    <Button onClick={() => setDeleteDialogOpen(false)}>Cancel</Button>
                    <Button color="error" onClick={handleDeleteGate} variant="contained">
                        Delete
                    </Button>
                </DialogActions>
            </Dialog>

            <Dialog open={createDialogOpen} onClose={() => setCreateDialogOpen(false)}>
                <DialogTitle>Create New Gate</DialogTitle>
                <DialogContent>
                    <TextField
                        fullWidth
                        required
                        margin="dense"
                        label="Location"
                        value={newGateData.location}
                        onChange={(e) => setNewGateData({ ...newGateData, location: e.target.value })}
                    />

                    <TextField
                        fullWidth
                        required
                        margin="dense"
                        label="Latitude"
                        type="number"
                        inputProps={{ step: "any" }}
                        value={newGateData.latitude}
                        onChange={(e) => setNewGateData({ ...newGateData, latitude: e.target.value })}
                    />

                    <TextField
                        fullWidth
                        required
                        margin="dense"
                        label="Longitude"
                        type="number"
                        inputProps={{ step: "any" }}
                        value={newGateData.longitude}
                        onChange={(e) => setNewGateData({ ...newGateData, longitude: e.target.value })}
                    />
                    <TextField
                        fullWidth
                        margin="dense"
                        label="Priority"
                        type="number"
                        inputProps={{ min: 0, max: 3 }}
                        value={newGateData.priority}
                        onChange={(e) => setNewGateData({ ...newGateData, priority: parseInt(e.target.value) })}
                    />
                    <TextField
                        fullWidth
                        required
                        select
                        margin="dense"
                        label="Status"
                        value={newGateData.status}
                        onChange={(e) => setNewGateData({ ...newGateData, status: e.target.value })}
                    >
                        <MenuItem value="OPENED">OPENED</MenuItem>
                        <MenuItem value="CLOSED">CLOSED</MenuItem>
                    </TextField>
                </DialogContent>
                <DialogActions>
                    <Button onClick={() => setCreateDialogOpen(false)}>Cancel</Button>
                    <Button
                        variant="contained"
                        color="primary"
                        disabled={!isFormValid()}
                        onClick={async () => {
                            try {
                                await api.post("/add-gate-ui", newGateData);
                                // const updated = await fetchGates();
                                // setGates(updated);
                                setCreateDialogOpen(false);
                                setNewGateData({ location: "", latitude: "", longitude: "", priority: 0 });
                            } catch (error) {
                                console.error("Fehler beim Erstellen:", error);
                                alert("Fehler beim Erstellen des Gates.");
                            }
                        }}
                    >
                        Create
                    </Button>
                </DialogActions>
            </Dialog>

        </div>
    );
}
/*
 * Test for Websockets
 */

export default StatusTables;
