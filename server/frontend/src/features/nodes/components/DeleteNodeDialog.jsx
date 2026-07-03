import { useState } from 'react';
import { Dialog, DialogTitle, DialogContent, DialogActions } from '@mui/material';
import { useDeleteNodeMutation } from '../../../app/store/api/api';

const DeleteNodeDialog = ({ open, node, onClose }) => {
  const [deleteNode] = useDeleteNodeMutation();
  const [submitting, setSubmitting] = useState(false);
  const [errorMessage, setErrorMessage] = useState('');

  const handleDelete = async () => {
    if (!node) return;
    setSubmitting(true);
    setErrorMessage('');
    try {
      await deleteNode(node.id).unwrap();
      onClose();
    } catch (err) {
      setErrorMessage('Failed to delete node. Please try again.');
    } finally {
      setSubmitting(false);
    }
  };

  const handleClose = () => {
    if (!submitting) {
      setErrorMessage('');
      onClose();
    }
  };

  return (
    <Dialog open={open} onClose={handleClose}>
      <DialogTitle>Confirm Deletion</DialogTitle>
      <DialogContent>
        Are you sure you want to delete node <strong>{node?.name}</strong>?
        {errorMessage && (
          <div style={{ color: 'var(--red-600)', marginTop: '8px' }}>{errorMessage}</div>
        )}
      </DialogContent>
      <DialogActions>
        <button className="btn btn-ghost" onClick={handleClose} disabled={submitting}>
          Cancel
        </button>
        <button className="btn btn-danger" onClick={handleDelete} disabled={submitting}>
          {submitting ? 'Deleting…' : 'Delete'}
        </button>
      </DialogActions>
    </Dialog>
  );
};

export default DeleteNodeDialog;
