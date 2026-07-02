import React, { useState } from "react";
import {
    Dialog, DialogTitle, DialogContent, DialogActions,
    Button, MenuItem, TextField, Typography
} from "@mui/material";
import LockIcon from "@mui/icons-material/Lock";
import LockOpenIcon from "@mui/icons-material/LockOpen";
import { useAppSelector } from "../../../app/store";
import { useRequestGateStatusChangeMutation } from "../../../app/store/api/api";

const selectUser = (state) => state.auth.user;

function StatusChangeDialog({ open, gate, onClose }) {
    const [requestedStatus, setRequestedStatus] = useState("");
    const [error, setError] = useState(null);
    const [requestGateStatusChange] = useRequestGateStatusChangeMutation();
    const userDetails = useAppSelector(selectUser);
    const workerId = userDetails?.workerId;
    const [prevGateId, setPrevGateId] = useState(gate?.id);

    if (gate?.id !== prevGateId) {
        setPrevGateId(gate?.id);
        setError(null);
    }

    if (!gate) return null;

    const handleSubmit = async () => {
        try {
            await requestGateStatusChange({
                gateId: gate.id,
                workerId,
                requestedStatus
            }).unwrap();
            onClose();
            setRequestedStatus("");
            setError(null);
        } catch (err) {
            console.error("Fehler beim Update:", err);
            setError("Failed to request status change. Please try again.");
        }
    };

    const getStatusClass = (status) => {
        switch (status?.toUpperCase()) {
            case "OPEN": return "status-open";
            case "CLOSED": return "status-closed";
            case "OUT_OF_SERVICE": return "status-oos";
            default: return "status-none";
        }
    };

    const getRequestedStatusClass = (status) => {
        switch (status?.toUpperCase()) {
            case "REQUESTED_OPEN": return "status-open";
            case "REQUESTED_CLOSE": return "status-closed";
            case "REQUESTED_NONE": return "status-none";
            default: return "status-none";
        }
    };

    const getStatusDisplay = (status) => {
        switch (status?.toUpperCase()) {
            case "OPEN":
                return <><LockOpenIcon fontSize="small" /> OPEN</>;
            case "CLOSED":
                return <><LockIcon fontSize="small" /> CLOSED</>;
            default:
                return <>NONE</>;
        }
    };

    return (
        <Dialog open={open} onClose={onClose}>
            <DialogTitle>Request Status Change</DialogTitle>
            <DialogContent>
                <Typography><strong>Gate:</strong> {gate.id}</Typography>
                <Typography><strong>Location:</strong> {gate.location}</Typography>
                <Typography>
                    <strong>Current Status:</strong>{" "}
                    <span className={`status-badge ${getStatusClass(gate.status)}`}>
                        {getStatusDisplay(gate.status)}
                    </span>
                </Typography>
                <Typography>
                    <strong>Requested Status:</strong>{" "}
                    <span className={`status-badge ${getRequestedStatusClass(gate.requestedStatus)}`}>
                        {gate.requestedStatus || "none"}
                    </span>
                </Typography>

                <TextField
                    fullWidth
                    select
                    margin="normal"
                    label="Requested Status"
                    value={requestedStatus}
                    onChange={(e) => setRequestedStatus(e.target.value)}
                >
                    <MenuItem value="REQUESTED_OPEN">OPEN</MenuItem>
                    <MenuItem value="REQUESTED_CLOSE">CLOSE</MenuItem>
                    <MenuItem value="REQUESTED_NONE">NONE</MenuItem>
                </TextField>
                {error && (
                    <Typography color="error" sx={{ mt: 1 }}>
                        {error}
                    </Typography>
                )}
            </DialogContent>
            <DialogActions>
                <Button onClick={onClose}>Cancel</Button>
                <Button
                    onClick={handleSubmit}
                    variant="contained"
                    color="warning"
                    disabled={!requestedStatus || !workerId}
                >
                    Request Change
                </Button>
            </DialogActions>
        </Dialog>
    );
}

export default StatusChangeDialog;
