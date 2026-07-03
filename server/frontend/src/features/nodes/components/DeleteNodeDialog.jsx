import { useState } from 'react';
import { Dialog, DialogTitle, DialogContent, DialogActions } from '@mui/material';
import { useDeleteNodeMutation } from '../../../app/store/api/api';

const DeleteNodeDialog = ({ open, node, onClose }) => {
  const [deleteNode] = useDeleteNodeMutation();
  const [submitting, setSubmitting] = useState(false);

  const handleDelete = async () => {
    if (!node) return;
    setSubmitting(true);
    try {
      await deleteNode(node.id).unwrap();
      onClose();
    } catch (err) {
      console.error('Failed to delete node:', err);
    } finally {
      setSubmitting(false);
    }
  };

  return (
    <Dialog open={open} onClose={onClose}>
      <DialogTitle>Confirm Deletion</DialogTitle>
      <DialogContent>
        Are you sure you want to delete node <strong>{node?.name}</strong>?
      </DialogContent>
      <DialogActions>
        <button className="btn btn-ghost" onClick={onClose} disabled={submitting}>
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
