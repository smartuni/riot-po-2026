import * as React from 'react';
import {Dialog, DialogTitle, DialogActions, DialogContent, DialogContentText, Button } from "@mui/material";

export const AlertDialog = ({open, onClose}) => {
    return (
      <Dialog
        open={open}
        // onClose={onClose}
        aria-labelledby="alert-dialog-title"
        aria-describedby="alert-dialog-description"
      >
        <DialogTitle id="alert-dialog-title">
          {"User Details changed"}
        </DialogTitle>
        <DialogContent>
          <DialogContentText id="alert-dialog-description">
            Your user details have been successfully modified.
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