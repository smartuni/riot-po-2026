import React, { useState } from "react";
import {
    Dialog, DialogTitle, DialogContent, DialogActions,
    Button, MenuItem, TextField, Typography
} from "@mui/material";
import LockIcon from "@mui/icons-material/Lock";
import LockOpenIcon from "@mui/icons-material/LockOpen";
import { useRequestGateStatusChangeMutation, useGetUserDetailsQuery } from "../../../app/store/api/api";

function StatusChangeDialog({ open, gate, onClose }) {
    const [requestedStatus, setRequestedStatus] = useState("");
    const [requestGateStatusChange] = useRequestGateStatusChangeMutation();
    const { data: userDetails } = useGetUserDetailsQuery();
    const workerId = userDetails?.workerId;

    if (!gate) return null;

    const handleSubmit = async () => {
        try {
            await requestGateStatusChange({
                gateId: gate.id,
                workerId,
                requestedStatus
            }).unwrap();
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
                    disabled={!requestedStatus || !workerId}
                >
                    Request Change
                </Button>
            </DialogActions>
        </Dialog>
    );
}

export default StatusChangeDialog;
