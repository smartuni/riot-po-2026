import React, { useState } from "react";
import {
    Dialog, DialogTitle, DialogContent, DialogActions,
    Button, Typography, Radio, RadioGroup, FormControlLabel, Alert, AlertTitle
} from "@mui/material";
import LockIcon from "@mui/icons-material/Lock";
import LockOpenIcon from "@mui/icons-material/LockOpen";
import { useAppSelector } from "../../../app/store";
import { useSetGateStatusManuallyMutation } from "../../../app/store/api/api";

const selectUser = (state) => state.auth.user;

const getStatusClass = (status) => {
    switch (status?.toUpperCase()) {
        case "OPEN": return "status-open";
        case "CLOSED": return "status-closed";
        case "OUT_OF_SERVICE": return "status-oos";
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

function ManualStatusDialog({ open, gate, onClose }) {
    const [newStatus, setNewStatus] = useState("");
    const [error, setError] = useState(null);
    const [setGateStatusManually] = useSetGateStatusManuallyMutation();
    const userDetails = useAppSelector(selectUser);
    const workerId = userDetails?.workerId;
    const [prevGateId, setPrevGateId] = useState(gate?.id);

    // Reset error when switching gates
    if (gate?.id !== prevGateId) {
        setPrevGateId(gate?.id);
        setError(null);
    }

    if (!gate) return null;

    const handleSubmit = async () => {
        try {
            await setGateStatusManually({
                gateId: gate.id,
                workerId,
                status: newStatus,
            }).unwrap();
            onClose();
            setNewStatus("");
            setError(null);
        } catch (err) {
            console.error("Failed to set gate status manually:", err);
            setError("Failed to set gate status. Please try again.");
        }
    };

    const handleClose = () => {
        setError(null);
        setNewStatus("");
        onClose();
    };

    return (
        <Dialog open={open} onClose={handleClose}>
            <DialogTitle>Set Gate Status Manually</DialogTitle>
            <DialogContent>
                <Typography><strong>Gate:</strong> {gate.id}</Typography>
                <Typography><strong>Location:</strong> {gate.location}</Typography>
                <Typography sx={{ mt: 1 }}>
                    <strong>Current Status:</strong>{" "}
                    <span className={`status-badge ${getStatusClass(gate.status)}`}>
                        {getStatusDisplay(gate.status)}
                    </span>
                </Typography>

                {gate.manualOverride && (
                    <Alert severity="info" sx={{ mt: 2 }}>
                        <AlertTitle>Note</AlertTitle>
                        Status was previously set manually.
                    </Alert>
                )}

                <Alert severity="warning" sx={{ mt: 2 }}>
                    <AlertTitle>Warning</AlertTitle>
                    This will immediately override the gate status. Use with caution.
                </Alert>

                <Typography sx={{ mt: 2, mb: 1, fontWeight: 600 }}>
                    New Status:
                </Typography>
                <RadioGroup
                    value={newStatus}
                    onChange={(e) => setNewStatus(e.target.value)}
                >
                    <FormControlLabel
                        value="OPEN"
                        control={<Radio />}
                        label="Open"
                    />
                    <FormControlLabel
                        value="CLOSED"
                        control={<Radio />}
                        label="Closed"
                    />
                </RadioGroup>

                {error && (
                    <Typography color="error" sx={{ mt: 1 }}>
                        {error}
                    </Typography>
                )}
            </DialogContent>
            <DialogActions>
                <Button onClick={handleClose}>Cancel</Button>
                <Button
                    onClick={handleSubmit}
                    variant="contained"
                    color="warning"
                    disabled={!newStatus || !workerId}
                >
                    Set Status
                </Button>
            </DialogActions>
        </Dialog>
    );
}

export default ManualStatusDialog;
