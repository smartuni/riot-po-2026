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
import ExpandMoreIcon from '@mui/icons-material/ExpandMore';
import ExpandLessIcon from '@mui/icons-material/ExpandLess';
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
    CircularProgress, Alert,
} from "@mui/material";
import LockOpenIcon from '@mui/icons-material/LockOpen';
import LockIcon from '@mui/icons-material/Lock';
import CircleIcon from '@mui/icons-material/Circle';
import SyncAltIcon from '@mui/icons-material/SyncAlt';
import QuestionMarkIcon from '@mui/icons-material/QuestionMark';
import Badge from '@mui/material/Badge';
import CheckIcon from '@mui/icons-material/Check';
import DoneAllIcon from '@mui/icons-material/DoneAll';
import PriorityHighIcon from '@mui/icons-material/PriorityHigh';
import { MapView } from "../../map";
import StatusChangedDialog from "./StatusChangedDialog";


function StatusTables() {
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
        status: "CLOSED", // default value
    });

    /**
     * For deleting a gate.
     * @returns {Promise<void>}
     */
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

    /**
     * Schließt den Dialog und aktualisiert die Gates.
     * @returns {Promise<void>}
     */
    const handleClose = async () => {
        setDialogOpen(false);
    };

    /**
     * Verarbeitet die Massenänderung des angeforderten Status für die gefilterten Gates.
     * @returns {Promise<void>}
     */
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

    const confirmationBadgeContent = (status) => {
        switch (status) {
            case "WORKER_CONFLICT":
                return <><PriorityHighIcon fontSize="small" /></>;
            case "UNCONFIRMED":
                return <> - </>;
            case "WORKER_CONFIRMED_SINGLE":
                return <><CheckIcon fontSize="small" /></>;
            case "WORKER_CONFIRMED_MULTI":
                return <><DoneAllIcon fontSize="small" /></>;
            case "WORKER_CONFIRMED_ALL":
                return <><DoneAllIcon fontSize="small" /></>;
            default:
                return <><CircleIcon fontSize="small" /></>;
        }
    };

    const confirmationBadgeColor = (status) => {
        switch (status) {
            case "WORKER_CONFLICT":
                return "error";
            case "WORKER_CONFIRMED_SINGLE":
                return "info";
            case "WORKER_CONFIRMED_MULTI":
            case "WORKER_CONFIRMED_ALL":
                return "success";
            default:
                return "error";
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

    if (gatesLoading || activitiesLoading) {
        return (
            <div className="gate-status-container" style={{ display: 'flex', justifyContent: 'center', alignItems: 'center', minHeight: '400px' }}>
                <CircularProgress />
            </div>
        );
    }

    if (gatesError || activitiesError) {
        return (
            <div className="gate-status-container" style={{ padding: '2rem' }}>
                <Alert severity="error">
                    {gatesError ? 'Failed to load gates data. ' : ''}
                    {activitiesError ? 'Failed to load activities data. ' : ''}
                </Alert>
            </div>
        );
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
                        <Tooltip
                            title={
                                downlinkCount >= 10
                                    ? "Downlink limit reached (10). Please wait or reset."
                                    : "You have sent " + downlinkCount + " downlinks. You can send up to 10 downlinks."
                            }
                        >
                            <span>
                                <Button
                                    variant="contained"
                                    color="secondary"
                                    onClick={sendManualDownlink}
                                    disabled={downlinkCount >= 10}
                                >
                                    Send Downlink
                                </Button>
                            </span>
                        </Tooltip>
                        <Button
                            variant="outlined"
                            color="error"
                            disabled={userDetails?.role !== 'controller'}
                            onClick={() => setResetDialogOpen(true)}
                        >
                            Reset Downlink Counter
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
                                            <span className="coords">{gate.latitude.toFixed(5)}, {gate.longitude.toFixed(5)}</span>
                                        </td>
                                        <td data-label="Status">
                                            <Badge
                                                color={confirmationBadgeColor(gate.stateConfirmation)}
                                                invisible={`${gate.stateConfirmation}` == "UNCONFIRMED"}
                                                anchorOrigin={{ vertical: 'top', horizontal: 'right', }}
                                                variant="standard"
                                                badgeContent={confirmationBadgeContent(gate.stateConfirmation)} >
                                                <span className={`badge ${gate.status.toLowerCase()}`}>
                                                    {
                                                        gate.status === "OPENED"
                                                            ? <LockOpenIcon fontSize="small" /> :
                                                            gate.status === "CLOSED" ? <LockIcon fontSize="small" /> :
                                                                <QuestionMarkIcon fontSize="small" />
                                                    } {gate.status}
                                                </span>
                                            </Badge>
                                        </td>
                                        <td data-label="Requested Status">
                                            <span
                                                className={`badge ${gate.requestedStatus ? gate.requestedStatus.toLowerCase() : 'none'}`}>
                                                {renderRequestedStatus(gate.requestedStatus)}
                                            </span>
                                        </td>
                                        <td data-label="Pending Jobs">
                                            <span
                                                className={`badge ${gate.pendingJob ? gate.pendingJob.toLowerCase() : 'none'}`}>
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
                    </Button>
                </DialogActions>
            </Dialog>
            <Dialog open={resetDialogOpen} onClose={() => setResetDialogOpen(false)}>
                <DialogTitle>Reset Downlink Counter</DialogTitle>
                <DialogContent>
                    <p>Are you sure you want to reset the downlink counter?</p>
                </DialogContent>
                <DialogActions>
                    <Button onClick={() => setResetDialogOpen(false)}>Cancel</Button>
                    <Button color="error" onClick={handleResetCounter} variant="contained">
                        Reset
                    </Button>
                </DialogActions>
            </Dialog>
        </div>
    );
}

export default StatusTables;
