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
        // onRequestChange(requestedStatus);
        try {
            // await updateGate(selectedGate.id, selectedGate);
            await requestGateStatusChange(gate.id, requestedStatus)
            // const updated = await fetchGates(); // Reload data
            // console.log(updated)
            // setGates(updated);
        } catch (err) {
            console.error("Fehler beim Update:", err);
        }
        onClose();
        setRequestedStatus("");
    };

    // const handleRequestChange = async (newStatus) => {
    //     try {
    //         // await updateGate(selectedGate.id, selectedGate);
    //         await requestGateStatusChange(selectedGate.id, )
    //         const updated = await fetchGates(); // Reload data
    //         console.log(updated)
    //         setGates(updated);
    //     } catch (err) {
    //         console.error("Fehler beim Update:", err);
    //     }
    // };

    return (
        <Dialog open={open} onClose={onClose}>
            <DialogTitle>Request Status Change</DialogTitle>
            <DialogContent>
                <Typography><strong>Gate:</strong> {gate.id}</Typography>
                <Typography><strong>Location:</strong> {gate.location}</Typography>
                <Typography>
                    <strong>Current Status:</strong> {
                    gate.status === "OPEN"
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
                    <MenuItem value="OPEN">OPEN</MenuItem>
                    <MenuItem value="CLOSED">CLOSED</MenuItem>
                    <MenuItem value="NONE">NONE</MenuItem>
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
