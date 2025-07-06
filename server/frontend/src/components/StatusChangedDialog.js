import React, { useEffect, useState } from "react";
import {
    Dialog, DialogTitle, DialogContent, DialogActions,
    Button, MenuItem, TextField, Typography
} from "@mui/material";
import LockIcon from "@mui/icons-material/Lock";
import LockOpenIcon from "@mui/icons-material/LockOpen";
import api, { requestGateStatusChange } from "../services/api";

function StatusChangeDialog({ open, gate, onClose }) {
    const [requestedStatus, setRequestedStatus] = useState("");
    const [workerId, setWorkerId] = useState(null);

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

    if (!gate) return null;

    const handleSubmit = async () => {
        try {
            await requestGateStatusChange(gate.id, workerId, requestedStatus);
        } catch (err) {
            console.error("Fehler beim Update:", err);
        }

        onClose();
        setRequestedStatus("");
    };

    const getStatusDisplay = (status) => {
        switch (status?.toUpperCase()) {
            case "OPENED":
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
                    <span className={`badge ${gate.status?.toLowerCase()}`}>
                        {getStatusDisplay(gate.status)}
                    </span>
                </Typography>
                <Typography>
                    <strong>Requested Status:</strong>{" "}
                    <span className={`badge ${gate.requestedStatus?.toLowerCase() || "none"}`}>
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
            </DialogContent>
            <DialogActions>
                <Button onClick={onClose}>Cancel</Button>
                <Button
                    onClick={handleSubmit}
                    variant="contained"
                    color="warning"
                    disabled={!requestedStatus}
                >
                    Request Change
                </Button>
            </DialogActions>
        </Dialog>
    );
}

export default StatusChangeDialog;
