import React, { useState } from "react";
import {
    Dialog, DialogTitle, DialogContent, DialogActions,
    Button, MenuItem, TextField, Typography
} from "@mui/material";
import LockIcon from "@mui/icons-material/Lock";
import LockOpenIcon from "@mui/icons-material/LockOpen";
import { requestGateStatusChange } from "../services/api";

function StatusChangeDialog({ open, gate, onClose }) {
    const [requestedStatus, setRequestedStatus] = useState("");

    if (!gate) return null;

    const handleSubmit = async () => {
        const currentStatus = gate.status?.toUpperCase();
        const currentRequested = gate.requestedStatus?.toUpperCase();
        const newRequested = requestedStatus?.toUpperCase();

        if (
            (newRequested === "REQUESTED_OPEN" && currentStatus === "OPENED") ||
            (newRequested === "REQUESTED_CLOSE" && currentStatus === "CLOSED") ||
            (newRequested === currentRequested)
        ) {
            console.warn("Kein Statuswechsel nötig.");
            onClose();
            setRequestedStatus("");
            return;
        }
        try {
            await requestGateStatusChange(gate.id, requestedStatus)
        } catch (err) {
            console.error("Fehler beim Update:", err);
        }
        onClose();
        setRequestedStatus("");
    };

    return (
        <Dialog open={open} onClose={onClose}>
            <DialogTitle>Request Status Change</DialogTitle>
            <DialogContent>
                <Typography><strong>Gate:</strong> {gate.id}</Typography>
                <Typography><strong>Location:</strong> {gate.location}</Typography>
                <Typography>
                    <strong>Current Status:</strong> {
                    gate.status === "OPENED"
                        ? <><LockOpenIcon fontSize="small" /> OPEN</>
                        : <><LockIcon fontSize="small" /> CLOSED</>
                }
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
