import * as React from 'react';
import {Dialog, DialogTitle, DialogActions, DialogContent, DialogContentText, Button } from "@mui/material";

export const AlertDialogUplink = ({open, onClose, messageString}) => {
    return (
      <Dialog
        open={open}
        // onClose={onClose}
        aria-labelledby="alert-dialog-title"
        aria-describedby="alert-dialog-description"
      >
        <DialogTitle id="alert-dialog-title">
          {"Uplink Received"}
        </DialogTitle>
        <DialogContent>
          <DialogContentText id="alert-dialog-description">
            Uplink has been received from end-nodes. {messageString}
          </DialogContentText>
        </DialogContent>
        <DialogActions>
          <Button onClick={onClose} autoFocus>
            OK
          </Button>
        </DialogActions>
      </Dialog>
  );
}